/*
  Copyright (C) 2006-2007 KovoKs <info@kovoks.nl>

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

#ifndef MAILTRANSPORT_SOCKET_H
#define MAILTRANSPORT_SOCKET_H

#include <mailtransport/mailtransport_export.h>

#include <QtNetwork/QSslSocket>

namespace MailTransport {

class SocketPrivate;

/**
 * @class Socket
 * Responsible for communicating with the server, it's designed to work
 * with the ServerTest class.
 * @author Tom Albers <tomalbers@kde.nl>
 */
class MAILTRANSPORT_EXPORT Socket : public QObject
{
  Q_OBJECT

  public:

    /**
     * Contructor, it will not auto connect. Call reconnect() to connect to
     * the parameters given.
     * @param parent the parent
     */
    explicit Socket( QObject *parent );

    /**
     * Destructor
     */
    ~Socket();

    /**
     * Existing connection will be closed and a new connection will be
     * made
     */
    virtual void reconnect();

    /**
     * Write @p text to the socket
     */
    virtual void write( const QString &text );

    /**
     * @return true when the connection is live and kicking
     */
    virtual bool available();

    /**
     * set the protocol to use
     */
    void setProtocol( const QString &proto );

    /**
     * set the server to use
     */
    void setServer( const QString &server );

    /**
     * set the port to use. If not specified, it will use the default
     * belonging to the protocol.
     */
    void setPort( int port );

    int port();

    /**
     * this will be a secure connection
     */
    void setSecure( bool what );

    /**
     * If you want to start TLS encryption, call this. For example after the starttls command.
     */
    void startTLS();

  private:
    Q_DECLARE_PRIVATE( Socket )
    SocketPrivate *const d;

    Q_PRIVATE_SLOT( d, void slotConnected() )
    Q_PRIVATE_SLOT( d, void slotStateChanged( QAbstractSocket::SocketState state ) )
    Q_PRIVATE_SLOT( d, void slotModeChanged( QSslSocket::SslMode  state ) )
    Q_PRIVATE_SLOT( d, void slotSocketRead() )
    Q_PRIVATE_SLOT( d, void slotSslErrors( const QList<QSslError> &errors ) )

  Q_SIGNALS:
    /**
     * emits the incoming data
     */
    void data( const QString & );

    /**
     * emitted when there is a connection (ready to send something).
     */
    void connected();

    /**
     * emitted when not connected.
     */
    void failed();

    /**
     * emitted when startShake() is completed.
     */
    void tlsDone();
};

} // namespace MailTransport

#endif // MAILTRANSPORT_SOCKET_H

