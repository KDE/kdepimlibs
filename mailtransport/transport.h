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

#include <mailtransport/transportbase.h>
#include <mailtransport/mailtransport_export.h>

class TransportPrivate;

namespace MailTransport {

/**
  Represents the settings of a specific mail transport.

  To create a new empty Transport object, use TransportManager::createTransport().
*/
class MAILTRANSPORT_EXPORT Transport : public TransportBase
{
  friend class TransportManager;

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
      This function syncronizes the password of this transport with the password of the
      transport with the same ID that is managed by the transport manager.
      This is only useful for cloned transports, since their passwords don't automatically
      get updated when calling TransportManager::loadPasswordsAsync() or
      TransportManager::loadPasswords().

      @sa clone()
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
      Returns a deep copy of this Transport object which will no longer be
      automatically updated. Use this if you need to store a Transport object
      over a longer time. However it is recommended to store transport identifiers
      instead if possible.

      @sa updatePasswordState()
    */
    Transport *clone() const;

  protected:
    /**
      Creates a Transport object. Should only be used by TransportManager.
      @param cfgGroup The KConfig group to read its data from.
    */
    Transport( const QString &cfgGroup );

    virtual void usrReadConfig();
    virtual void usrWriteConfig();

    /**
      Returns true if the password was not stored in the wallet.
    */
    bool needsWalletMigration() const;

    /**
      Try to migrate the password from the config file to the wallet.
    */
    void migrateToWallet();

  private:
    void readPassword();

  private:
    TransportPrivate *const d;
};

}

#endif
