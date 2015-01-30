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
    /** TLS request state. */
    enum TLSRequestState {
        UseTLSIfAvailable,
        ForceTLS,
        ForceNoTLS
    };

    virtual ~SMTPSessionInterface();
    virtual bool startSsl() = 0;

    /** Parse capability response from the server. */
    void parseFeatures(const KioSMTP::Response &ehloResponse);

    /** Returns the server reported capabilities. */
    const Capabilities &capabilities() const;

    /** Clear the capabilities reported by the server (e.g. when reconnecting the session) */
    void clearCapabilities();

    /** This is a pure convenience wrapper around
     *  @ref KioSMTP::Capabilities::have()
     */
    virtual bool haveCapability(const char *cap) const;

    /** @return true is pipelining is available and allowed by metadata */
    bool canPipelineCommands() const;

    virtual void error(int id, const QString &msg) = 0;
    /** Show information message box with message @p msg and caption @p caption. */
    virtual void informationMessageBox(const QString &msg, const QString &caption) = 0;
    virtual bool openPasswordDialog(KIO::AuthInfo &authInfo) = 0;
    virtual void dataReq() = 0;
    virtual int readData(QByteArray &ba) = 0;

    /** SASL method requested for authentication. */
    virtual QString requestedSaslMethod() const = 0;
    /** TLS requested for encryption. */
    virtual TLSRequestState tlsRequested() const = 0;
    /** LF2CRLF and dot stuffing requested. */
    virtual bool lf2crlfAndDotStuffingRequested() const = 0;
    /** 8bit MIME support requested. */
    virtual bool eightBitMimeRequested() const;
    /** Pipelining has been requested. */
    virtual bool pipeliningRequested() const;

private :
    KioSMTP::Capabilities m_capabilities;
};

}

#endif
