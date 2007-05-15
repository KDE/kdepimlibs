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
#include "mailtransport_defs.h"
#include "transport.h"
#include "smtpjob.h"
#include "sendmailjob.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kemailsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krandom.h>
#include <kurl.h>
#include <kwallet.h>
#include <kconfiggroup.h>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QRegExp>
#include <QStringList>

using namespace MailTransport;
using namespace KWallet;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class TransportManager::Private {
  public:
    ~Private() {
        qRemovePostRoutine(cleanupTransportManager);
        cleanupTransportManager();
    }
    KConfig *config;
    QList<Transport*> transports;
    bool myOwnChange;
    KWallet::Wallet *wallet;
    bool walletOpenFailed;
    bool walletAsyncOpen;
    int defaultTransportId;
    bool isMainInstance;
    QList<TransportJob*> walletQueue;

    static TransportManager *sSelf;
    static void cleanupTransportManager()
    {
        delete sSelf;
        sSelf = 0;
    }
};
TransportManager *TransportManager::Private::sSelf = 0;

TransportManager::TransportManager() :
    QObject(), d( new Private )
{
  d->myOwnChange = false;
  d->wallet = 0;
  d->walletOpenFailed = false;
  d->walletAsyncOpen = false;
  d->defaultTransportId = -1;
  d->config = new KConfig( QLatin1String("mailtransports") );

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
}

TransportManager::~TransportManager()
{
  delete d->config;
  qDeleteAll( d->transports );
}

TransportManager* TransportManager::self()
{
  static TransportManager::Private p;
  if(!p.sSelf) {
    p.sSelf = new TransportManager;
    p.sSelf->readConfig();
    qAddPostRoutine(Private::cleanupTransportManager);
  }
  return p.sSelf;
}

Transport* TransportManager::transportById(int id, bool def) const
{
  foreach ( Transport* t, d->transports )
    if ( t->id() == id )
      return t;

  if ( def || (id == 0 && d->defaultTransportId != id) )
    return transportById( d->defaultTransportId, false );
  return 0;
}

Transport* TransportManager::transportByName(const QString & name, bool def) const
{
  foreach ( Transport* t, d->transports )
    if ( t->name() == name )
      return t;
  if ( def )
    return transportById( 0, false );
  return 0;
}

QList< Transport * > TransportManager::transports() const
{
  return d->transports;
}

Transport* TransportManager::createTransport() const
{
  int id = createId();
  Transport *t = new Transport( QString::number( id ) );
  t->setId( id );
  return t;
}

void TransportManager::addTransport(Transport * transport)
{
  Q_ASSERT( !d->transports.contains( transport ) );
  d->transports.append( transport );
  validateDefault();
  emitChangesCommitted();
}

void TransportManager::schedule(TransportJob * job)
{
  connect( job, SIGNAL(result(KJob*)), SLOT(jobResult(KJob*)) );

  // check if the job is waiting for the wallet
  if ( !job->transport()->isComplete() ) {
    kDebug(5324) << k_funcinfo << " job waits for wallet: " << job << endl;
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
  t->setName( i18n("Default Transport") );
  t->setHost( kes.getSetting( KEMailSettings::OutServer ) );
  if ( t->isValid() ) {
    t->writeConfig();
    addTransport( t );
  } else
    kWarning() << k_funcinfo
            << "KEMailSettings does not contain a valid transport." << endl;
}

TransportJob* TransportManager::createTransportJob( int transportId )
{
  Transport *t = transportById( transportId, false );
  if ( !t )
    return 0;
  switch ( t->type() ) {
    case Transport::EnumType::SMTP:
      return new SmtpJob( t->clone(), this );
    case Transport::EnumType::Sendmail:
      return new SendmailJob( t->clone(), this );
  }
  Q_ASSERT( false );
  return 0;
}

TransportJob* TransportManager::createTransportJob(const QString & transport)
{
  bool ok = false;
  Transport *t = 0;

  int transportId = transport.toInt( &ok );
  if ( ok )
    t = transportById( transportId );

  if ( !t )
    t = transportByName( transport, false );

  if ( t )
    return createTransportJob( t->id() );

  KUrl url( transport );
  if ( !url.isValid() )
    return 0;

  t = new Transport( QLatin1String("adhoc") );
  t->setDefaults();
  t->setName( transport );
  t->setAdHoc( true );

  if ( url.protocol() == SMTP_PROTOCOL || url.protocol() == SMTPS_PROTOCOL ) {
    t->setType( Transport::EnumType::SMTP );
    t->setHost( url.host() );
    if ( url.protocol() == SMTPS_PROTOCOL ) {
      t->setEncryption( Transport::EnumEncryption::SSL );
      t->setPort( SMTPS_PORT );
    }
    if ( url.port() != -1 )
      t->setPort( url.port() );
    if ( url.hasUser() ) {
      t->setRequiresAuthentication( true );
      t->setUserName( url.user() );
    }
  }

  else if ( url.protocol() == QLatin1String("file") ) {
    t->setType( Transport::EnumType::Sendmail );
    t->setHost( url.path( KUrl::RemoveTrailingSlash ) );
  }

  else {
    delete t;
    return 0;
  }

  switch ( t->type() ) {
    case Transport::EnumType::SMTP:
      return new SmtpJob( t, this );
    case Transport::EnumType::Sendmail:
      return new SendmailJob( t, this );
  }

  delete t;
  Q_ASSERT( false );
  return 0;
}

bool TransportManager::isEmpty() const
{
  return d->transports.isEmpty();
}

QList<int> TransportManager::transportIds() const
{
  QList<int> rv;
  foreach ( Transport *t, d->transports )
    rv << t->id();
  return rv;
}

QStringList TransportManager::transportNames() const
{
  QStringList rv;
  foreach ( Transport *t, d->transports )
    rv << t->name();
  return rv;
}

QString TransportManager::defaultTransportName() const
{
  Transport* t = transportById( d->defaultTransportId, false );
  if ( t )
    return t->name();
  return QString();
}

int TransportManager::defaultTransportId() const
{
  return d->defaultTransportId;
}

void TransportManager::setDefaultTransport(int id)
{
  if ( id == d->defaultTransportId || !transportById( id, false ) )
    return;
  d->defaultTransportId = id;
  writeConfig();
}

void TransportManager::removeTransport(int id)
{
  Transport *t = transportById( id, false );
  if ( !t )
    return;
  d->transports.removeAll( t );
  validateDefault();
  QString group = t->currentGroup();
  delete t;
  d->config->deleteGroup( group );
  writeConfig();
}

void TransportManager::readConfig()
{
  QList<Transport*> oldTransports = d->transports;
  d->transports.clear();

  QRegExp re( QLatin1String("^Transport (.+)$") );
  QStringList groups = d->config->groupList().filter( re );
  foreach ( QString s, groups ) {
    re.indexIn( s );
    Transport *t = 0;

    // see if we happen to have that one already
    foreach ( Transport *old, oldTransports ) {
      if ( old->currentGroup() == QLatin1String("Transport ") + re.cap( 1 ) ) {
        kDebug(5324) << k_funcinfo
                << " reloading existing transport: " << s << endl;
        t = old;
        t->readConfig();
        oldTransports.removeAll( old );
        break;
      }
    }

    if ( !t )
      t = new Transport( re.cap( 1 ) );
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

void TransportManager::emitChangesCommitted()
{
  d->myOwnChange = true; // prevent us from reading our changes again
  emit transportsChanged();
  emit changesCommitted();
}

void TransportManager::slotTransportsChanged()
{
  if ( d->myOwnChange ) {
    d->myOwnChange = false;
    return;
  }

  kDebug(5324) << k_funcinfo << endl;
  d->config->reparseConfiguration();
  // FIXME: this deletes existing transport objects!
  readConfig();
  emit transportsChanged();
}

int TransportManager::createId() const
{
  QList<int> usedIds;
  foreach ( Transport *t, d->transports )
    usedIds << t->id();
  usedIds << 0; // 0 is default for unknown
  int newId;
  do {
      newId = KRandom::random();
  } while ( usedIds.contains( newId )  );
  return newId;
}

KWallet::Wallet * TransportManager::wallet()
{
  if ( d->wallet && d->wallet->isOpen() )
    return d->wallet;

  if ( !Wallet::isEnabled() || d->walletOpenFailed )
    return 0;

  WId window = 0;
  if ( qApp->activeWindow() )
    window = qApp->activeWindow()->winId();
  else if ( qApp->mainWidget() )
    window = qApp->mainWidget()->topLevelWidget()->winId();

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
  if ( !d->wallet )
    return;
  if ( !d->wallet->hasFolder( WALLET_FOLDER ) )
    d->wallet->createFolder( WALLET_FOLDER );
  d->wallet->setFolder( WALLET_FOLDER );
}

void TransportManager::loadPasswords()
{
  foreach ( Transport *t, d->transports )
    t->readPassword();

  // flush the wallet queue
  foreach ( TransportJob *job, d->walletQueue ) {
    job->start();
  }
  d->walletQueue.clear();

  emit passwordsChanged();
}

void TransportManager::loadPasswordsAsync()
{
  kDebug(5324) << k_funcinfo << endl;

  // check if there is anything to do at all
  bool found = false;
  foreach ( Transport *t, d->transports ) {
    if ( !t->isComplete() ) {
      found = true;
      break;
    }
  }
  if ( !found )
    return;

  // async wallet opening
  if ( !d->wallet && !d->walletOpenFailed ) {
    WId window = 0;
    if ( qApp->activeWindow() )
      window = qApp->activeWindow()->winId();
    else if ( qApp->mainWidget() )
      window = qApp->mainWidget()->topLevelWidget()->winId();
    d->wallet = Wallet::openWallet( Wallet::NetworkWallet(), window,
                                  Wallet::Asynchronous );
    if ( d->wallet ) {
      connect( d->wallet, SIGNAL(walletOpened(bool)), SLOT(slotWalletOpened(bool)) );
      d->walletAsyncOpen = true;
    }
    else {
      d->walletOpenFailed = true;
      loadPasswords();
    }
    return;
  }
  if ( d->wallet && !d->walletAsyncOpen )
    loadPasswords();
}

void TransportManager::slotWalletOpened( bool success )
{
  kDebug(5324) << k_funcinfo << endl;
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
  if ( !firstRun )
    return;
  firstRun = false;

  // check if we are the main instance
  if ( !d->isMainInstance )
    return;

  // check if migration is needed
  QStringList names;
  foreach ( Transport *t, d->transports )
    if ( t->needsWalletMigration() )
      names << t->name();
  if ( names.isEmpty() )
    return;

  // ask user if he wants to migrate
  int result = KMessageBox::questionYesNoList( 0,
    i18n("The following mail transports store passwords in the configuration "
         "file instead in KWallet.\nIt is recommended to use KWallet for "
         "password storage for security reasons.\n"
         "Do you want to migrate your passwords to KWallet?"),
    names );
  if ( result != KMessageBox::Yes )
    return;

  // perform migration
  foreach ( Transport *t, d->transports )
    if ( t->needsWalletMigration() )
      t->migrateToWallet();
}

void TransportManager::dbusServiceOwnerChanged(const QString & service, const QString & oldOwner, const QString & newOwner)
{
  Q_UNUSED( oldOwner );
  if ( service == DBUS_SERVICE_NAME && newOwner.isEmpty() )
    QDBusConnection::sessionBus().registerService( DBUS_SERVICE_NAME );
}

void TransportManager::jobResult(KJob * job)
{
  d->walletQueue.removeAll( static_cast<TransportJob*>( job ) );
}

#include "transportmanager.moc"
