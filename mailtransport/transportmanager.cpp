/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "transportmanager.h"
#include "addtransportdialog.h"
#include "akonadijob.h"
#include "mailtransport_defs.h"
#include "sendmailconfigwidget.h"
#include "sendmailjob.h"
#include "smtpconfigwidget.h"
#include "smtpjob.h"
#include "transport.h"
#include "transportconfigwidget.h"
#include "transportjob.h"
#include "transporttype.h"
#include "transporttype_p.h"
#include "transportconfigdialog.h"

#include <QApplication>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QPointer>
#include <QRegExp>
#include <QStringList>

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <kemailsettings.h>
#include <KLocale>
#include <KMessageBox>
#include <KRandom>
#include <KUrl>
#include <KWallet/Wallet>

#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>

using namespace MailTransport;
using namespace KWallet;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class TransportManager::Private
{
  public:
    Private() {}
    ~Private() {
      delete config;
      qDeleteAll( transports );
    }

    KConfig *config;
    QList<Transport *> transports;
    TransportType::List types;
    bool myOwnChange;
    bool appliedChange;
    KWallet::Wallet *wallet;
    bool walletOpenFailed;
    bool walletAsyncOpen;
    int defaultTransportId;
    bool isMainInstance;
    QList<TransportJob *> walletQueue;
};

class StaticTransportManager : public TransportManager
{
  public:
    StaticTransportManager() : TransportManager() {}
};

StaticTransportManager *sSelf = 0;

static void destroyStaticTransportManager() {
  delete sSelf;
}

TransportManager::TransportManager()
  : QObject(), d( new Private )
{
  KGlobal::locale()->insertCatalog( QLatin1String( "libmailtransport" ) );
  qAddPostRoutine( destroyStaticTransportManager );
  d->myOwnChange = false;
  d->appliedChange = false;
  d->wallet = 0;
  d->walletOpenFailed = false;
  d->walletAsyncOpen = false;
  d->defaultTransportId = -1;
  d->config = new KConfig( QLatin1String( "mailtransports" ) );

  QDBusConnection::sessionBus().registerObject( DBUS_OBJECT_PATH, this,
      QDBusConnection::ExportScriptableSlots |
              QDBusConnection::ExportScriptableSignals );

  QDBusConnection::sessionBus().connect( QString(), QString(),
                              DBUS_INTERFACE_NAME, DBUS_CHANGE_SIGNAL,
                              this, SLOT(slotTransportsChanged()) );

  d->isMainInstance =
          QDBusConnection::sessionBus().registerService( DBUS_SERVICE_NAME );
  connect( QDBusConnection::sessionBus().interface(),
           SIGNAL(serviceOwnerChanged(QString,QString,QString)),
           SLOT(dbusServiceOwnerChanged(QString,QString,QString)) );

  fillTypes();
}

TransportManager::~TransportManager()
{
  qRemovePostRoutine( destroyStaticTransportManager );
  delete d;
}

TransportManager *TransportManager::self()
{
  if ( !sSelf ) {
    sSelf = new StaticTransportManager;
    sSelf->readConfig();
  }
  return sSelf;
}

Transport *TransportManager::transportById( int id, bool def ) const
{
  foreach ( Transport *t, d->transports ) {
    if ( t->id() == id ) {
      return t;
    }
  }

  if ( def || ( id == 0 && d->defaultTransportId != id ) ) {
    return transportById( d->defaultTransportId, false );
  }
  return 0;
}

Transport *TransportManager::transportByName( const QString &name, bool def ) const
{
  foreach ( Transport *t, d->transports ) {
    if ( t->name() == name ) {
      return t;
    }
  }
  if ( def ) {
    return transportById( 0, false );
  }
  return 0;
}

QList< Transport * > TransportManager::transports() const
{
  return d->transports;
}

TransportType::List TransportManager::types() const
{
  return d->types;
}

Transport *TransportManager::createTransport() const
{
  int id = createId();
  Transport *t = new Transport( QString::number( id ) );
  t->setId( id );
  return t;
}

void TransportManager::addTransport( Transport *transport )
{
  if ( d->transports.contains( transport ) ) {
    kDebug() << "Already have this transport.";
    return;
  }

  kDebug() << "Added transport" << transport;
  d->transports.append( transport );
  validateDefault();
  emitChangesCommitted();
}

void TransportManager::schedule( TransportJob *job )
{
  connect( job, SIGNAL(result(KJob*)), SLOT(jobResult(KJob*)) );

  // check if the job is waiting for the wallet
  if ( !job->transport()->isComplete() ) {
    kDebug() << "job waits for wallet:" << job;
    d->walletQueue << job;
    loadPasswordsAsync();
    return;
  }

  job->start();
}

void TransportManager::createDefaultTransport()
{
  KEMailSettings kes;
  Transport *t = createTransport();
  t->setName( i18n( "Default Transport" ) );
  t->setHost( kes.getSetting( KEMailSettings::OutServer ) );
  if ( t->isValid() ) {
    t->writeConfig();
    addTransport( t );
  } else {
    kWarning() << "KEMailSettings does not contain a valid transport.";
  }
}

bool TransportManager::showNewTransportDialog( QWidget *parent )
{
  QPointer<AddTransportDialog> dialog = new AddTransportDialog( parent );
  bool accepted = ( dialog->exec() == QDialog::Accepted );
  delete dialog;
  return accepted;
}

bool TransportManager::promptCreateTransportIfNoneExists( QWidget *parent )
{
  if ( !isEmpty() ) {
    return true;
  }

  const int response = KMessageBox::messageBox( parent,
                   KMessageBox::WarningContinueCancel,
                   i18n( "You must create an outgoing account before sending." ),
                   i18n( "Create Account Now?" ),
                   KGuiItem( i18n( "Create Account Now" ) ) );
  if ( response == KMessageBox::Continue ) {
    return showNewTransportDialog( parent );
  }
  return false;
}

bool TransportManager::configureTransport( Transport *transport, QWidget *parent )
{
  if( transport->type() == Transport::EnumType::Akonadi ) {
    using namespace Akonadi;
    AgentInstance instance = AgentManager::self()->instance( transport->host() );
    if( !instance.isValid() ) {
      kWarning() << "Invalid resource instance" << transport->host();
    }
    instance.configure( parent ); // Async...
    transport->writeConfig();
    return true; // No way to know here if the user cancelled or not.
  }

  QPointer<KDialog> dialog = new KDialog( parent );
  TransportConfigWidget *configWidget = 0;
  switch( transport->type() ) {
    case Transport::EnumType::SMTP:
      {
        configWidget = new SMTPConfigWidget( transport, dialog );
        break;
      }
    case Transport::EnumType::Sendmail:
      {
        configWidget =  new SendmailConfigWidget( transport, dialog );
        break;
      }
    default:
      {
        Q_ASSERT( false );
        delete dialog;
        return false;
      }
  }
  dialog->setMainWidget( configWidget );
  dialog->setCaption( i18n( "Configure account" ) );
  dialog->setButtons( KDialog::Ok | KDialog::Cancel );
  bool okClicked = ( dialog->exec() == QDialog::Accepted );
  if( okClicked ) {
    configWidget->apply(); // calls transport->writeConfig()
  }
  delete dialog;
  return okClicked;
}

TransportJob *TransportManager::createTransportJob( int transportId )
{
  Transport *t = transportById( transportId, false );
  if ( !t ) {
    return 0;
  }
  t = t->clone(); // Jobs delete their transports.
  switch ( t->type() ) {
    case Transport::EnumType::SMTP:
      return new SmtpJob( t, this );
    case Transport::EnumType::Sendmail:
      return new SendmailJob( t, this );
    case Transport::EnumType::Akonadi:
      return new AkonadiJob( t, this );
  }
  Q_ASSERT( false );
  return 0;
}

TransportJob *TransportManager::createTransportJob( const QString &transport )
{
  bool ok = false;
  Transport *t = 0;

  int transportId = transport.toInt( &ok );
  if ( ok ) {
    t = transportById( transportId );
  }

  if ( !t ) {
    t = transportByName( transport, false );
  }

  if ( t ) {
    return createTransportJob( t->id() );
  }

  return 0;
}

bool TransportManager::isEmpty() const
{
  return d->transports.isEmpty();
}

QList<int> TransportManager::transportIds() const
{
  QList<int> rv;
  foreach ( Transport *t, d->transports ) {
    rv << t->id();
  }
  return rv;
}

QStringList TransportManager::transportNames() const
{
  QStringList rv;
  foreach ( Transport *t, d->transports ) {
    rv << t->name();
  }
  return rv;
}

QString TransportManager::defaultTransportName() const
{
  Transport *t = transportById( d->defaultTransportId, false );
  if ( t ) {
    return t->name();
  }
  return QString();
}

int TransportManager::defaultTransportId() const
{
  return d->defaultTransportId;
}

void TransportManager::setDefaultTransport( int id )
{
  if ( id == d->defaultTransportId || !transportById( id, false ) ) {
    return;
  }
  d->defaultTransportId = id;
  writeConfig();
}

void TransportManager::removeTransport( int id )
{
  Transport *t = transportById( id, false );
  if ( !t ) {
    return;
  }
  emit transportRemoved( t->id(), t->name() );

  // Kill the resource, if Akonadi-type transport.
  if( t->type() == Transport::EnumType::Akonadi ) {
    using namespace Akonadi;
    const AgentInstance instance = AgentManager::self()->instance( t->host() );
    if( !instance.isValid() ) {
      kWarning() << "Could not find resource instance.";
    }
    AgentManager::self()->removeInstance( instance );
  }

  d->transports.removeAll( t );
  validateDefault();
  QString group = t->currentGroup();
  delete t;
  d->config->deleteGroup( group );
  writeConfig();

}

void TransportManager::readConfig()
{
  QList<Transport *> oldTransports = d->transports;
  d->transports.clear();

  QRegExp re( QLatin1String( "^Transport (.+)$" ) );
  QStringList groups = d->config->groupList().filter( re );
  foreach ( const QString &s, groups ) {
    re.indexIn( s );
    Transport *t = 0;

    // see if we happen to have that one already
    foreach ( Transport *old, oldTransports ) {
      if ( old->currentGroup() == QLatin1String( "Transport " ) + re.cap( 1 ) ) {
        kDebug() << "reloading existing transport:" << s;
        t = old;
        t->readConfig();
        oldTransports.removeAll( old );
        break;
      }
    }

    if ( !t ) {
      t = new Transport( re.cap( 1 ) );
    }
    if ( t->id() <= 0 ) {
      t->setId( createId() );
      t->writeConfig();
    }
    d->transports.append( t );
  }

  qDeleteAll( oldTransports );
  oldTransports.clear();

  // read default transport
  KConfigGroup group( d->config, "General" );
  d->defaultTransportId = group.readEntry( "default-transport", 0 );
  if ( d->defaultTransportId == 0 ) {
    // migrated default transport contains the name instead
    QString name = group.readEntry( "default-transport", QString() );
    if ( !name.isEmpty() ) {
      Transport *t = transportByName( name, false );
      if ( t ) {
        d->defaultTransportId = t->id();
        writeConfig();
      }
    }
  }
  validateDefault();
  migrateToWallet();
}

void TransportManager::writeConfig()
{
  KConfigGroup group( d->config, "General" );
  group.writeEntry( "default-transport", d->defaultTransportId );
  d->config->sync();
  emitChangesCommitted();
}

void TransportManager::fillTypes()
{
  Q_ASSERT( d->types.isEmpty() );

  // SMTP.
  {
    TransportType type;
    type.d->mType = Transport::EnumType::SMTP;
    type.d->mName = i18nc( "@option SMTP transport", "SMTP" );
    type.d->mDescription = i18n( "An SMTP server on the internet" );
    d->types << type;
  }

  // Sendmail.
  {
    TransportType type;
    type.d->mType = Transport::EnumType::Sendmail;
    type.d->mName = i18nc( "@option sendmail transport", "Sendmail" );
    type.d->mDescription = i18n( "A local sendmail installation" );
    d->types << type;
  }

  // All Akonadi resources with MailTransport capability.
  {
    using namespace Akonadi;
    foreach ( const AgentType &atype, AgentManager::self()->types() ) {
      // TODO probably the string "MailTransport" should be #defined somewhere
      // and used like that in the resources (?)
      if( atype.capabilities().contains( QLatin1String( "MailTransport" ) ) ) {
        TransportType type;
        type.d->mType = Transport::EnumType::Akonadi;
        type.d->mAgentType = atype;
        type.d->mName = atype.name();
        type.d->mDescription = atype.description();
        d->types << type;
        kDebug() << "Found Akonadi type" << atype.name();
      }
    }

    // Watch for appearing and disappearing types.
    connect( AgentManager::self(), SIGNAL(typeAdded(Akonadi::AgentType)),
        this, SLOT(agentTypeAdded(Akonadi::AgentType)) );
    connect( AgentManager::self(), SIGNAL(typeRemoved(Akonadi::AgentType)),
        this, SLOT(agentTypeRemoved(Akonadi::AgentType)) );
  }

  kDebug() << "Have SMTP, Sendmail, and" << d->types.count() - 2 << "Akonadi types.";
}

void TransportManager::emitChangesCommitted()
{
  d->myOwnChange = true; // prevent us from reading our changes again
  d->appliedChange = false; // but we have to read them at least once
  emit transportsChanged();
  emit changesCommitted();
}

void TransportManager::slotTransportsChanged()
{
  if ( d->myOwnChange && d->appliedChange ) {
    d->myOwnChange = false;
    d->appliedChange = false;
    return;
  }

  kDebug();
  d->config->reparseConfiguration();
  // FIXME: this deletes existing transport objects!
  readConfig();
  d->appliedChange = true; // to prevent recursion
  emit transportsChanged();
}

int TransportManager::createId() const
{
  QList<int> usedIds;
  foreach ( Transport *t, d->transports ) {
    usedIds << t->id();
  }
  usedIds << 0; // 0 is default for unknown
  int newId;
  do {
      newId = KRandom::random();
  } while ( usedIds.contains( newId ) );
  return newId;
}

KWallet::Wallet * TransportManager::wallet()
{
  if ( d->wallet && d->wallet->isOpen() ) {
    return d->wallet;
  }

  if ( !Wallet::isEnabled() || d->walletOpenFailed ) {
    return 0;
  }

  WId window = 0;
  if ( qApp->activeWindow() ) {
    window = qApp->activeWindow()->winId();
  } else if ( !QApplication::topLevelWidgets().isEmpty() ) {
    window = qApp->topLevelWidgets().first()->winId();
  }

  delete d->wallet;
  d->wallet = Wallet::openWallet( Wallet::NetworkWallet(), window );

  if ( !d->wallet ) {
    d->walletOpenFailed = true;
    return 0;
  }

  prepareWallet();
  return d->wallet;
}

void TransportManager::prepareWallet()
{
  if ( !d->wallet ) {
    return;
  }
  if ( !d->wallet->hasFolder( WALLET_FOLDER ) ) {
    d->wallet->createFolder( WALLET_FOLDER );
  }
  d->wallet->setFolder( WALLET_FOLDER );
}

void TransportManager::loadPasswords()
{
  foreach ( Transport *t, d->transports ) {
    t->readPassword();
  }

  // flush the wallet queue
  foreach ( TransportJob *job, d->walletQueue ) {
    job->start();
  }
  d->walletQueue.clear();

  emit passwordsChanged();
}

void TransportManager::loadPasswordsAsync()
{
  kDebug();

  // check if there is anything to do at all
  bool found = false;
  foreach ( Transport *t, d->transports ) {
    if ( !t->isComplete() ) {
      found = true;
      break;
    }
  }
  if ( !found ) {
    return;
  }

  // async wallet opening
  if ( !d->wallet && !d->walletOpenFailed ) {
    WId window = 0;
    if ( qApp->activeWindow() ) {
      window = qApp->activeWindow()->winId();
    } else if ( !QApplication::topLevelWidgets().isEmpty() ) {
      window = qApp->topLevelWidgets().first()->winId();
    }

    d->wallet = Wallet::openWallet( Wallet::NetworkWallet(), window,
                                    Wallet::Asynchronous );
    if ( d->wallet ) {
      connect( d->wallet, SIGNAL(walletOpened(bool)), SLOT(slotWalletOpened(bool)) );
      d->walletAsyncOpen = true;
    } else {
      d->walletOpenFailed = true;
      loadPasswords();
    }
    return;
  }
  if ( d->wallet && !d->walletAsyncOpen ) {
    loadPasswords();
  }
}

void TransportManager::slotWalletOpened( bool success )
{
  kDebug();
  d->walletAsyncOpen = false;
  if ( !success ) {
    d->walletOpenFailed = true;
    delete d->wallet;
    d->wallet = 0;
  } else {
    prepareWallet();
  }
  loadPasswords();
}

void TransportManager::validateDefault()
{
  if ( !transportById( d->defaultTransportId, false ) ) {
    if ( isEmpty() ) {
      d->defaultTransportId = -1;
    } else {
      d->defaultTransportId = d->transports.first()->id();
      writeConfig();
    }
  }
}

void TransportManager::migrateToWallet()
{
  // check if we tried this already
  static bool firstRun = true;
  if ( !firstRun ) {
    return;
  }
  firstRun = false;

  // check if we are the main instance
  if ( !d->isMainInstance ) {
    return;
  }

  // check if migration is needed
  QStringList names;
  foreach ( Transport *t, d->transports ) {
    if ( t->needsWalletMigration() ) {
      names << t->name();
    }
  }
  if ( names.isEmpty() ) {
    return;
  }

  // ask user if he wants to migrate
  int result = KMessageBox::questionYesNoList(
    0,
    i18n( "The following mail transports store their passwords in an "
          "unencrypted configuration file.\nFor security reasons, "
          "please consider migrating these passwords to KWallet, the "
          "KDE Wallet management tool,\nwhich stores sensitive data "
          "for you in a strongly encrypted file.\n"
          "Do you want to migrate your passwords to KWallet?" ),
    names, i18n( "Question" ),
    KGuiItem( i18n( "Migrate" ) ), KGuiItem( i18n( "Keep" ) ),
    QString::fromAscii( "WalletMigrate" ) );
  if ( result != KMessageBox::Yes ) {
    return;
  }

  // perform migration
  foreach ( Transport *t, d->transports ) {
    if ( t->needsWalletMigration() ) {
      t->migrateToWallet();
    }
  }
}

void TransportManager::dbusServiceOwnerChanged( const QString &service,
                                                const QString &oldOwner,
                                                const QString &newOwner )
{
  Q_UNUSED( oldOwner );
  if ( service == DBUS_SERVICE_NAME && newOwner.isEmpty() ) {
    QDBusConnection::sessionBus().registerService( DBUS_SERVICE_NAME );
  }
}

void TransportManager::agentTypeAdded( const Akonadi::AgentType &atype )
{
  using namespace Akonadi;
  if( atype.capabilities().contains( QLatin1String( "MailTransport" ) ) ) {
    TransportType type;
    type.d->mType = Transport::EnumType::Akonadi;
    type.d->mAgentType = atype;
    type.d->mName = atype.name();
    type.d->mDescription = atype.description();
    d->types << type;
    kDebug() << "Added new Akonadi type" << atype.name();
  }
}

void TransportManager::agentTypeRemoved( const Akonadi::AgentType &atype )
{
  using namespace Akonadi;
  foreach ( const TransportType &type, d->types ) {
    if( type.type() == Transport::EnumType::Akonadi &&
        type.agentType() == atype ) {
      d->types.removeAll( type );
      kDebug() << "Removed Akonadi type" << atype.name();
    }
  }
}

void TransportManager::jobResult( KJob *job )
{
  d->walletQueue.removeAll( static_cast<TransportJob*>( job ) );
}

#include "transportmanager.moc"
