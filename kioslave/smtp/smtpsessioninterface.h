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
    virtual void parseFeatures( const KioSMTP::Response & ) = 0;
    virtual bool startSsl() = 0;
    virtual bool isUsingSsl() const = 0;
    virtual bool isAutoSsl() const = 0;
    virtual bool haveCapability( const char * cap ) const = 0;
    virtual void error( int id, const QString & msg ) = 0;
    virtual void messageBox( KIO::SlaveBase::MessageBoxType id, const QString & msg, const QString &caption ) = 0;
    virtual bool openPasswordDialog( KIO::AuthInfo &authInfo ) = 0;
    virtual void dataReq() = 0;
    virtual int readData( QByteArray & ba ) = 0;
    virtual QString metaData( const QString & key ) const = 0;
};

}


#endif
