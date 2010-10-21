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
#include <KIO/PasswordDialog>
#include <kio/authinfo.h>
#include <kio/global.h>
#include <KLocalizedString>

using namespace MailTransport;

class MailTransport::SmtpSessionPrivate : public KioSMTP::SMTPSessionInterface
{
  public:
    explicit SmtpSessionPrivate( SmtpSession *session ) :
      useTLS( true ),
      socket( 0 ),
      q( session )
     {}

    void dataReq() { /* noop */ };
    int readData(QByteArray& ba)
    {
      if ( data->atEnd() ) {
        ba.clear();
        return 0;
      } else {
        Q_ASSERT( data->isOpen() );
        ba = data->read( 32 * 1024 );
        return ba.size();
      }
    }

    void error(int id, const QString& msg)
    {
      KMessageBox::error( 0, KIO::buildErrorString( id, msg ), i18n( "Mail Sending Failed" ) );
    }

    void informationMessageBox(const QString& msg, const QString& caption)
    {
      KMessageBox::information( 0, msg, caption );
    }

    bool openPasswordDialog(KIO::AuthInfo& authInfo) {
      return KIO::PasswordDialog::getNameAndPassword(
        authInfo.username,
        authInfo.password,
        &(authInfo.keepPassword),
        authInfo.prompt,
        authInfo.readOnly,
        authInfo.caption,
        authInfo.comment,
        authInfo.commentLabel
      ) == KIO::PasswordDialog::Accepted;
    }

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
    QIODevice *data;

  private:
    SmtpSession *q;
};

SmtpSession::SmtpSession(QObject* parent) :
  QObject(parent),
  d( new SmtpSessionPrivate( this ) )
{
  d->socket = new KTcpSocket( this );
}

void SmtpSession::setSaslMethod(const QString& method)
{
  d->saslMethod = method;
}

void SmtpSession::setUseTLS(bool useTLS)
{
  d->useTLS = useTLS;
}

void SmtpSession::connectToHost(const KUrl& url)
{
  if ( url.protocol() == QLatin1String( "smtps" ) )
    d->socket->connectToHostEncrypted( url.host(), url.port() );
  else if ( url.protocol() == QLatin1String( "smtp" ) )
    d->socket->connectToHost( url.host(), url.port() );
  else
    Q_ASSERT( !"Unsupported protocol!" );
}

void SmtpSession::sendMessage(const KUrl& destination, QIODevice* data)
{
  if ( d->socket->state() != KTcpSocket::ConnectedState && d->socket->state() != KTcpSocket::ConnectingState ) {
    connectToHost( destination );
  }

  d->data = data;
}




#include "smtpsession.h"
