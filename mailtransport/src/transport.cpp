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

#include "transport.h"
#include "transport_p.h"
#include "legacydecrypt.h"
#include "mailtransport_defs.h"
#include "transportmanager.h"
#include "transporttype_p.h"

#include <QTimer>

#include <KConfigGroup>
#include <QDebug>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStringHandler>
#include <KWallet/KWallet>

#include <agentinstance.h>
#include <agentmanager.h>

using namespace MailTransport;
using namespace KWallet;

Transport::Transport( const QString &cfgGroup ) :
    TransportBase( cfgGroup ), d( new TransportPrivate )
{
  qDebug() << cfgGroup;
  d->passwordLoaded = false;
  d->passwordDirty = false;
  d->storePasswordInFile = false;
  d->needsWalletMigration = false;
  d->passwordNeedsUpdateFromWallet = false;
  load();
}

Transport::~Transport()
{
  delete d;
}

bool Transport::isValid() const
{
  return ( id() > 0 ) && !host().isEmpty() && port() <= 65536;
}

QString Transport::password()
{
  if ( !d->passwordLoaded && requiresAuthentication() && storePassword() &&
       d->password.isEmpty() ) {
    readPassword();
  }
  return d->password;
}

void Transport::setPassword( const QString &passwd )
{
  d->passwordLoaded = true;
  if ( d->password == passwd ) {
    return;
  }
  d->passwordDirty = true;
  d->password = passwd;
}

void Transport::forceUniqueName()
{
  QStringList existingNames;
  foreach ( Transport *t, TransportManager::self()->transports() ) {
    if ( t->id() != id() ) {
      existingNames << t->name();
    }
  }
  int suffix = 1;
  QString origName = name();
  while ( existingNames.contains( name() ) ) {
    setName( i18nc( "%1: name; %2: number appended to it to make "
                    "it unique among a list of names", "%1 #%2", origName, suffix ) );
    ++suffix;
  }

}

void Transport::updatePasswordState()
{
  Transport *original = TransportManager::self()->transportById( id(), false );
  if ( original == this ) {
    qWarning() << "Tried to update password state of non-cloned transport.";
    return;
  }
  if ( original ) {
    d->password = original->d->password;
    d->passwordLoaded = original->d->passwordLoaded;
    d->passwordDirty = original->d->passwordDirty;
  } else {
    qWarning() << "Transport with this ID not managed by transport manager.";
  }
}

bool Transport::isComplete() const
{
  return !requiresAuthentication() || !storePassword() || d->passwordLoaded;
}

QString Transport::authenticationTypeString() const
{
  return Transport::authenticationTypeString( authenticationType() );
}

QString Transport::authenticationTypeString( int type )
{
  switch ( type ) {
    case EnumAuthenticationType::LOGIN:
      return QLatin1String( "LOGIN" );
    case EnumAuthenticationType::PLAIN:
      return QLatin1String( "PLAIN" );
    case EnumAuthenticationType::CRAM_MD5:
      return QLatin1String( "CRAM-MD5" );
    case EnumAuthenticationType::DIGEST_MD5:
      return QLatin1String( "DIGEST-MD5" );
    case EnumAuthenticationType::NTLM:
      return QLatin1String( "NTLM" );
    case EnumAuthenticationType::GSSAPI:
      return QLatin1String( "GSSAPI" );
    case EnumAuthenticationType::CLEAR:
      return i18nc( "Authentication method", "Clear text" );
    case EnumAuthenticationType::APOP:
      return QLatin1String( "APOP" );
    case EnumAuthenticationType::ANONYMOUS:
      return i18nc( "Authentication method", "Anonymous" );
  }
  Q_ASSERT( false );
  return QString();
}

void Transport::usrRead()
{
  TransportBase::usrRead();

  setHost( host().trimmed() );

  if ( d->oldName.isEmpty() ) {
    d->oldName = name();
  }

  // Set TransportType.
  {
    using namespace Akonadi;
    d->transportType = TransportType();
    d->transportType.d->mType = type();
    qDebug() << "type" << type();
    if ( type() == EnumType::Akonadi ) {
      const AgentInstance instance = AgentManager::self()->instance( host() );
      if ( !instance.isValid() ) {
        qWarning() << "Akonadi transport with invalid resource instance.";
      }
      d->transportType.d->mAgentType = instance.type();
      qDebug() << "agent type" << instance.type().name() << "id" << instance.type().identifier();
    }
    // Now we have the type and possibly agentType.  Get the name, description
    // etc. from TransportManager.
    const TransportType::List &types = TransportManager::self()->types();
    int index = types.indexOf( d->transportType );
    if ( index != -1 ) {
      d->transportType = types[ index ];
    } else {
      qWarning() << "Type unknown to manager.";
      d->transportType.d->mName = i18nc( "An unknown transport type", "Unknown" );
    }
  }

  // we have everything we need
  if ( !storePassword() ) {
    return;
  }

  if ( d->passwordLoaded ) {
    if ( d->passwordNeedsUpdateFromWallet ) {
      d->passwordNeedsUpdateFromWallet = false;
      // read password if wallet is open, defer otherwise
      if ( Wallet::isOpen( Wallet::NetworkWallet() ) ) {
        // Don't read the password right away because this can lead
        // to reentrancy problems in KDBusServiceStarter when an application
        // run in Kontact creates the transports (due to a QEventLoop in the
        // synchronous KWallet openWallet call).
        QTimer::singleShot( 0, this, SLOT(readPassword()) );
      } else {
        d->passwordLoaded = false;
      }
    }

    return;
  }

  // try to find a password in the config file otherwise
  KConfigGroup group( config(), currentGroup() );
  if ( group.hasKey( "password" ) ) {
    d->password = KStringHandler::obscure( group.readEntry( "password" ) );
  } else if ( group.hasKey( "password-kmail" ) ) {
    d->password = Legacy::decryptKMail( group.readEntry( "password-kmail" ) );
  } else if ( group.hasKey( "password-knode" ) ) {
    d->password = Legacy::decryptKNode( group.readEntry( "password-knode" ) );
  }

  if ( !d->password.isEmpty() ) {
    d->passwordLoaded = true;
    if ( Wallet::isEnabled() ) {
      d->needsWalletMigration = true;
    } else {
      d->storePasswordInFile = true;
    }
  }
}

bool Transport::usrSave()
{
  if ( requiresAuthentication() && storePassword() && d->passwordDirty ) {
    Wallet *wallet = TransportManager::self()->wallet();
    if ( !wallet || wallet->writePassword( QString::number( id() ), d->password ) != 0 ) {
      // wallet saving failed, ask if we should store in the config file instead
      if ( d->storePasswordInFile || KMessageBox::warningYesNo(
             0,
             i18n( "KWallet is not available. It is strongly recommended to use "
                   "KWallet for managing your passwords.\n"
                   "However, the password can be stored in the configuration "
                   "file instead. The password is stored in an obfuscated format, "
                   "but should not be considered secure from decryption efforts "
                   "if access to the configuration file is obtained.\n"
                   "Do you want to store the password for server '%1' in the "
                   "configuration file?", name() ),
             i18n( "KWallet Not Available" ),
             KGuiItem( i18n( "Store Password" ) ),
             KGuiItem( i18n( "Do Not Store Password" ) ) ) == KMessageBox::Yes ) {
        // write to config file
        KConfigGroup group( config(), currentGroup() );
        group.writeEntry( "password", KStringHandler::obscure( d->password ) );
        d->storePasswordInFile = true;
      }
    }
    d->passwordDirty = false;
  }

  if (!TransportBase::usrSave()) {
    return false;
  }
  TransportManager::self()->emitChangesCommitted();
  if ( name() != d->oldName ) {
    emit TransportManager::self()->transportRenamed( id(), d->oldName, name() );
    d->oldName = name();
  }

  return true;
}

void Transport::readPassword()
{
  // no need to load a password if the account doesn't require auth
  if ( !requiresAuthentication() ) {
    return;
  }
  d->passwordLoaded = true;

  // check whether there is a chance to find our password at all
  if ( Wallet::folderDoesNotExist( Wallet::NetworkWallet(), WALLET_FOLDER ) ||
       Wallet::keyDoesNotExist( Wallet::NetworkWallet(), WALLET_FOLDER,
                                QString::number( id() ) ) ) {
    // try migrating password from kmail
    if ( Wallet::folderDoesNotExist( Wallet::NetworkWallet(), KMAIL_WALLET_FOLDER ) ||
         Wallet::keyDoesNotExist( Wallet::NetworkWallet(), KMAIL_WALLET_FOLDER,
                                  QString::fromLatin1( "transport-%1" ).arg( id() ) ) ) {
      return;
    }
    qDebug() << "migrating password from kmail wallet";
    KWallet::Wallet *wallet = TransportManager::self()->wallet();
    if ( wallet ) {
      QString pwd;
      wallet->setFolder( KMAIL_WALLET_FOLDER );
      if ( wallet->readPassword( QString::fromLatin1( "transport-%1" ).arg( id() ), pwd ) == 0 ) {
        setPassword( pwd );
        save();
      } else {
        d->password.clear();
        d->passwordLoaded = false;
      }
      wallet->removeEntry( QString::fromLatin1( "transport-%1" ).arg( id() ) );
      wallet->setFolder( WALLET_FOLDER );
    }
    return;
  }

  // finally try to open the wallet and read the password
  KWallet::Wallet *wallet = TransportManager::self()->wallet();
  if ( wallet ) {
    QString pwd;
    if ( wallet->readPassword( QString::number( id() ), pwd ) == 0 ) {
      setPassword( pwd );
    } else {
      d->password.clear();
      d->passwordLoaded = false;
    }
  }
}

bool Transport::needsWalletMigration() const
{
  return d->needsWalletMigration;
}

void Transport::migrateToWallet()
{
  qDebug() << "migrating" << id() << "to wallet";
  d->needsWalletMigration = false;
  KConfigGroup group( config(), currentGroup() );
  group.deleteEntry( "password" );
  group.deleteEntry( "password-kmail" );
  group.deleteEntry( "password-knode" );
  d->passwordDirty = true;
  d->storePasswordInFile = false;
  save();
}

Transport *Transport::clone() const
{
  QString id = currentGroup().mid( 10 );
  return new Transport( id );
}

TransportType Transport::transportType() const
{
  if ( !d->transportType.isValid() ) {
    qWarning() << "Invalid transport type.";
  }
  return d->transportType;
}

void Transport::setTransportType( const TransportType &type )
{
  Q_ASSERT( type.isValid() );
  d->transportType = type;
  setType( type.type() );
}

