/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#ifndef KIOSMTP_SMTPSESSIONINTERFACE_H
#define KIOSMTP_SMTPSESSIONINTERFACE_H

#include <kio/slavebase.h>

#include "capabilities.h"

class QByteArray;
class QString;

namespace KIO {
class AuthInfo;
}

namespace KioSMTP {

class Response;

/** Interface to the SMTP session for command classes.
 * There are sub-classes for the in-process mode, the KIO slave mode and for unit testing.
 * @since 4.6
 */
class SMTPSessionInterface
{
  public:
    virtual ~SMTPSessionInterface();
    virtual bool startSsl() = 0;
    virtual bool isUsingSsl() const = 0;
    virtual bool isAutoSsl() const = 0;

    /** Parse capability response from the server. */
    void parseFeatures( const KioSMTP::Response & );

    /** Returns the server reported capabilities. */
    const Capabilities& capabilities() const;

    /** Clear the capabilities reported by the server (e.g. when reconnecting the session) */
    void clearCapabilities();

    /** This is a pure convenience wrapper around
     *  @ref KioSMTP::Capabilities::have()
     */
    virtual bool haveCapability( const char * cap ) const;

    /** @return true is pipelining is available and allowed by metadata */
    bool canPipelineCommands() const;

    /** This is a pure convenience wrapper around
     *  @ref KioSMTP::Capabilities::createSpecialResponse
     */
    QString createSpecialResponse() const;

    virtual void error( int id, const QString & msg ) = 0;
    virtual void messageBox( KIO::SlaveBase::MessageBoxType id, const QString & msg, const QString &caption ) = 0;
    virtual bool openPasswordDialog( KIO::AuthInfo &authInfo ) = 0;
    virtual void dataReq() = 0;
    virtual int readData( QByteArray & ba ) = 0;
    virtual QString metaData( const QString & key ) const = 0;

  private :
    KioSMTP::Capabilities m_capabilities;
};

}


#endif
