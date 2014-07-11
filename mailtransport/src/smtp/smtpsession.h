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

#ifndef MAILTRANSPORT_SMTPSESSION_H
#define MAILTRANSPORT_SMTPSESSION_H
#include <qobject.h>
#include <ktcpsocket.h>

class QIODevice;
class QUrl;

namespace MailTransport {

class SmtpSessionPrivate;

/** Connection to an SMTP server. */
class SmtpSession : public QObject
{
  Q_OBJECT
  public:
    explicit SmtpSession( QObject *parent = 0 );
    ~SmtpSession();

    /** Open connection to host. */
    void connectToHost( const QUrl &url );

    /** Close the connection to the SMTP server. */
    void disconnectFromHost( bool nice = true );

    /** Sets the SASL method used for authentication. */
    void setSaslMethod( const QString &method );

    /**  Enable TLS encryption. */
    void setUseTLS( bool useTLS );

    /** Send a message. */
    void sendMessage( const QUrl &destination, QIODevice *data );

    /** Returns the error nmeesage, if any.  */
    QString errorMessage() const;

  Q_SIGNALS:
    /** Emitted when an email transfer has been completed. */
    void result( MailTransport::SmtpSession *session );

  private:
    friend class SmtpSessionPrivate;
    SmtpSessionPrivate * const d;
    Q_PRIVATE_SLOT( d, void socketConnected() )
    Q_PRIVATE_SLOT( d, void receivedNewData() )
    Q_PRIVATE_SLOT( d, void socketError(KTcpSocket::Error) )
    Q_PRIVATE_SLOT( d, void socketDisconnected() )
};

}

#endif
