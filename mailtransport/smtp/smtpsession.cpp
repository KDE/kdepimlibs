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

#include "smtpsession.h"

#include "smtp/smtpsessioninterface.h"
#include <ktcpsocket.h>
#include <KMessageBox>

using namespace MailTransport;

class MailTransport::SmtpSessionPrivate : public KioSMTP::SMTPSessionInterface
{
  public:
    explicit SmtpSessionPrivate( SmtpSession *session ) :
      useTLS( true ),
      socket( 0 ),
      q( session )
     {}

    // TODO implement these
    void dataReq() {};
    int readData(QByteArray& ba) { return 0; }
    void error(int id, const QString& msg) { }

    void informationMessageBox(const QString& msg, const QString& caption)
    {
      KMessageBox::information( 0, msg, caption );
    }

    // TODO
    bool openPasswordDialog(KIO::AuthInfo& authInfo) { return true; }

    bool startSsl()
    {
      // TODO looking at TCPSlaveBase this is far from complete...
      Q_ASSERT( socket );
      socket->startClientEncryption();
      const bool encrypted = socket->waitForEncrypted( -1 );
      return encrypted;
    }

    bool lf2crlfAndDotStuffingRequested() const { return true; }
    QString requestedSaslMethod() const { return saslMethod; }
    TLSRequestState tlsRequested() const { return useTLS ? ForceTLS : ForceNoTLS; }

  public:
    QString saslMethod;
    bool useTLS;

    KTcpSocket *socket;

  private:
    SmtpSession *q;
};

SmtpSession::SmtpSession(QObject* parent) :
  QObject(parent),
  d( new SmtpSessionPrivate( this ) )
{
}

void SmtpSession::setSaslMethod(const QString& method)
{
  d->saslMethod = method;
}


#include "smtpsession.h"
