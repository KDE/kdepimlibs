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

#ifndef MAILTRANSPORT_TRANSPORT_H
#define MAILTRANSPORT_TRANSPORT_H

#include <mailtransport_export.h>
#include <transportbase.h>
#include <transporttype.h>

class TransportPrivate;

namespace MailTransport {

class TransportType;

/**
  Represents the settings of a specific mail transport.

  To create a new empty Transport object, use TransportManager::createTransport().

  Initialize an empty Transport object by calling the set...() methods defined in
  kcfg-generated TransportBase, and in this class. Note that some transports use
  the "host" setting to store the following values:
   - Sendmail transport: path to the sendmail executable
   - Akonadi transports: resource ID.
*/
// TODO KDE5: Do something about the kcfg-generated TransportBase.
// Currently it has the config stuff as private members, which means it is
// utterly inextensible.  Also the sendmail and akonadi-type transports use
// the "host" setting for keeping the location of the sendmail executable and
// the resource id, respectively.  This is a hack; they should have separate
// config options... (cberzan)
class MAILTRANSPORT_EXPORT Transport : public TransportBase
{
  Q_OBJECT
  friend class TransportManager;
  friend class TransportManagerPrivate;

  public:
    /**
      Destructor
    */
    virtual ~Transport();

    typedef QList<Transport*> List;

    /**
      Returns true if this transport is valid, ie. has all necessary data set.
    */
    bool isValid() const;

    /**
      Returns the password of this transport.
    */
    QString password();

    /**
      Sets the password of this transport.
      @param passwd The new password.
    */
    void setPassword( const QString &passwd );

    /**
      Makes sure the transport has a unique name.  Adds #1, #2, #3 etc. if
      necessary.
      @since 4.4
    */
    void forceUniqueName();

    /**
      This function synchronizes the password of this transport with the
      password of the transport with the same ID that is managed by the
      transport manager. This is only useful for cloned transports, since
      their passwords don't automatically get updated when calling
      TransportManager::loadPasswordsAsync() or TransportManager::loadPasswords().

      @sa clone()
      @since 4.2
    */
    void updatePasswordState();

    /**
      Returns true if all settings have been loaded.
      This is the way to find out if the password has already been loaded
      from the wallet.
    */
    bool isComplete() const;

    /**
      Returns a string representation of the authentication type.
    */
    QString authenticationTypeString() const;

    /**
      Returns a string representation of the authentication type.
      Convienence function when there isn't a Transport object
      instantiated.

      @since 4.5
    */
    static QString authenticationTypeString( int type );

    /**
      Returns a deep copy of this Transport object which will no longer be
      automatically updated. Use this if you need to store a Transport object
      over a longer time. However it is recommended to store transport identifiers
      instead if possible.

      @sa updatePasswordState()
    */
    Transport *clone() const;

    /**
      Returns the type of this transport.
      @see TransportType.
      @since 4.4
    */
    TransportType transportType() const;

    /**
      Sets the type of this transport.
      @see TransportType.
      @since 4.4
    */
    void setTransportType( const TransportType &type );

  protected:
    /**
      Creates a Transport object. Should only be used by TransportManager.
      @param cfgGroup The KConfig group to read its data from.
    */
    Transport( const QString &cfgGroup );

    virtual void usrRead();
    virtual bool usrSave();

    /**
      Returns true if the password was not stored in the wallet.
    */
    bool needsWalletMigration() const;

    /**
      Try to migrate the password from the config file to the wallet.
    */
    void migrateToWallet();

  private Q_SLOTS:

    // Used by our friend, TransportManager
    void readPassword();

  private:
    TransportPrivate *const d;
};

} // namespace MailTransport

#endif // MAILTRANSPORT_TRANSPORT_H
