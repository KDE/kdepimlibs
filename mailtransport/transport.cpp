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
#include "transportmanager.h"
#include "mailtransport_defs.h"
#include "legacydecrypt.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstringhandler.h>
#include <kwallet.h>
#include <kconfiggroup.h>

using namespace MailTransport;
using namespace KWallet;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class TransportPrivate {
  public:
    QString password;
    bool passwordLoaded;
    bool passwordDirty;
    bool storePasswordInFile;
    bool needsWalletMigration;
    QString oldName;
};

Transport::Transport( const QString &cfgGroup ) :
    TransportBase( cfgGroup ), d( new TransportPrivate )
{
  kDebug(5324) << cfgGroup;
  d->passwordLoaded = false;
  d->passwordDirty = false;
  d->storePasswordInFile = false;
  d->needsWalletMigration = false;
  readConfig();
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
       d->password.isEmpty() )
    TransportManager::self()->loadPasswords();
  return d->password;
}

void Transport::setPassword(const QString & passwd)
{
  d->passwordLoaded = true;
  if ( d->password == passwd )
    return;
  d->passwordDirty = true;
  d->password = passwd;
}

bool Transport::isComplete() const
{
  return !requiresAuthentication() || !storePassword() || d->passwordLoaded;
}

QString Transport::authenticationTypeString() const
{
  switch ( authenticationType() ) {
    case EnumAuthenticationType::LOGIN: return QLatin1String( "LOGIN" );
    case EnumAuthenticationType::PLAIN: return QLatin1String( "PLAIN" );
    case EnumAuthenticationType::CRAM_MD5: return QLatin1String( "CRAM-MD5" );
    case EnumAuthenticationType::DIGEST_MD5: return QLatin1String( "DIGEST-MD5" );
    case EnumAuthenticationType::NTLM: return QLatin1String( "NTLM" );
    case EnumAuthenticationType::GSSAPI: return QLatin1String( "GSSAPI" );
  }
  Q_ASSERT( false );
  return QString();
}

void Transport::usrReadConfig()
{
  TransportBase::usrReadConfig();
  if ( d->oldName.isEmpty() )
    d->oldName = name();

  // we have everything we need
  if ( !storePassword() || d->passwordLoaded )
    return;

  // try to find a password in the config file otherwise
  KConfigGroup group( config(), currentGroup() );
  if ( group.hasKey( "password" ) )
    d->password = KStringHandler::obscure( group.readEntry( "password" ) );
  else if ( group.hasKey( "password-kmail" ) )
    d->password = Legacy::decryptKMail( group.readEntry( "password-kmail" ) );
  else if ( group.hasKey( "password-knode" ) )
    d->password = Legacy::decryptKNode( group.readEntry( "password-knode" ) );

  if ( !d->password.isEmpty() ) {
    d->passwordLoaded = true;
    if ( Wallet::isEnabled() ) {
      d->needsWalletMigration = true;
    } else {
      d->storePasswordInFile = true;
    }
  } else {
    // read password if wallet is open, defer otherwise
    if ( Wallet::isOpen( Wallet::NetworkWallet() ) )
      readPassword();
  }
}

void Transport::usrWriteConfig()
{
  if ( requiresAuthentication() && storePassword() && d->passwordDirty ) {
    Wallet *wallet = TransportManager::self()->wallet();
    if ( !wallet || wallet->writePassword(QString::number(id()), d->password) != 0 ) {
      // wallet saving failed, ask if we should store in the config file instead
      if ( d->storePasswordInFile || KMessageBox::warningYesNo( 0,
            i18n("KWallet is not available. It is strongly recommended to use "
                "KWallet for managing your passwords.\n"
                "However, the password can be stored in the configuration "
                "file instead. The password is stored in an obfuscated format, "
                "but should not be considered secure from decryption efforts "
                "if access to the configuration file is obtained.\n"
                "Do you want to store the password for server '%1' in the "
                "configuration file?", name() ),
            i18n("KWallet Not Available"),
            KGuiItem( i18n("Store Password") ),
            KGuiItem( i18n("Do Not Store Password") ) )
            == KMessageBox::Yes ) {
        // write to config file
        KConfigGroup group( config(), currentGroup() );
        group.writeEntry( "password", KStringHandler::obscure( d->password ) );
        d->storePasswordInFile = true;
      }
    }
    d->passwordDirty = false;
  }

  TransportBase::usrWriteConfig();
  TransportManager::self()->emitChangesCommitted();
  if ( name() != d->oldName ) {
    emit TransportManager::self()->transportRenamed( id(), d->oldName, name() );
    d->oldName = name();
  }
}

void Transport::readPassword()
{
  // no need to load a password if the account doesn't require auth
  if ( !requiresAuthentication() )
    return;
  d->passwordLoaded = true;

  // check wether there is a chance to find our password at all
  if ( Wallet::folderDoesNotExist(Wallet::NetworkWallet(), WALLET_FOLDER) ||
      Wallet::keyDoesNotExist(Wallet::NetworkWallet(), WALLET_FOLDER,
                              QString::number(id())) )
  {
    // try migrating password from kmail
    if ( Wallet::folderDoesNotExist(Wallet::NetworkWallet(), KMAIL_WALLET_FOLDER ) ||
         Wallet::keyDoesNotExist(Wallet::NetworkWallet(), KMAIL_WALLET_FOLDER,
                                 QString::fromLatin1("transport-%1").arg( id() ) ) )
      return;
    kDebug(5324) << "migrating password from kmail wallet";
    KWallet::Wallet *wallet = TransportManager::self()->wallet();
    if ( wallet ) {
      wallet->setFolder( KMAIL_WALLET_FOLDER );
      wallet->readPassword( QString::fromLatin1("transport-%1").arg( id() ),
                            d->password );
      wallet->removeEntry( QString::fromLatin1("transport-%1").arg( id() ) );
      wallet->setFolder( WALLET_FOLDER );
      d->passwordDirty = true;
      writeConfig();
    }
    return;
  }

  // finally try to open the wallet and read the password
  KWallet::Wallet *wallet = TransportManager::self()->wallet();
  if ( wallet )
    wallet->readPassword( QString::number(id()), d->password );
}

bool Transport::needsWalletMigration() const
{
  return d->needsWalletMigration;
}

void Transport::migrateToWallet()
{
  kDebug(5324) << "migrating" << id() << "to wallet";
  d->needsWalletMigration = false;
  KConfigGroup group( config(), currentGroup() );
  group.deleteEntry( "password" );
  d->passwordDirty = true;
  d->storePasswordInFile = false;
  writeConfig();
}

Transport* Transport::clone() const
{
  QString id = currentGroup().mid( 10 );
  return new Transport( id );
}
