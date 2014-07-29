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

// Uncomment the next line for full comm debug
// #define comm_debug

// Own
#include "socket.h"

// Qt
#include <QByteArray>
#include <QSslSocket>

// KDE
#include <QDebug>
#include <ksocketfactory.h>

using namespace MailTransport;

namespace MailTransport
{

class SocketPrivate
{
public:
    SocketPrivate(Socket *s);
    Socket             *const q;
    QSslSocket         *socket;
    QString             server;
    QString             protocol;
    int                 port;
    bool                secure;

    // slots
    void slotConnected();
    void slotStateChanged(QAbstractSocket::SocketState state);
    void slotModeChanged(QSslSocket::SslMode  state);
    void slotSocketRead();
    void slotSslErrors(const QList<QSslError> &errors);

private:
    QString m_msg;
};

}

SocketPrivate::SocketPrivate(Socket *s) : q(s)
{
}

void SocketPrivate::slotConnected()
{
    qDebug();

    if (!secure) {
        qDebug() << "normal connect";
        emit q->connected();
    } else {
        qDebug() << "encrypted connect";
        socket->startClientEncryption();
    }
}

void SocketPrivate::slotStateChanged(QAbstractSocket::SocketState state)
{
#ifdef comm_debug
    qDebug() << "State is now:" << (int) state;
#endif
    if (state == QAbstractSocket::UnconnectedState) {
        emit q->failed();
    }
}

void SocketPrivate::slotModeChanged(QSslSocket::SslMode  state)
{
#ifdef comm_debug
    qDebug() << "Mode is now:" << state;
#endif
    if (state == QSslSocket::SslClientMode) {
        emit q->tlsDone();
    }
}

void SocketPrivate::slotSocketRead()
{
    qDebug();

    if (!socket) {
        return;
    }

    m_msg += QLatin1String(socket->readAll());

    if (!m_msg.endsWith(QLatin1Char('\n'))) {
        return;
    }

#ifdef comm_debug
    qDebug() << socket->isEncrypted() << m_msg.trimmed();
#endif

    emit q->data(m_msg);
    m_msg.clear();
}

void SocketPrivate::slotSslErrors(const QList<QSslError> &)
{
    qDebug();
    /* We can safely ignore the errors, we are only interested in the
    capabilities. We're not sending auth info. */
    socket->ignoreSslErrors();
    emit q->connected();
}

// ------------------ end private ---------------------------//

Socket::Socket(QObject *parent)
    : QObject(parent), d(new SocketPrivate(this))
{
    d->socket = 0;
    d->port = 0;
    d->secure = false;
    qDebug();
}

Socket::~Socket()
{
    qDebug();
    delete d;
}

void Socket::reconnect()
{
    qDebug() << "Connecting to:" << d->server <<  ":" <<  d->port;

#ifdef comm_debug
    // qDebug() << d->protocol;
#endif

    if (d->socket) {
        return;
    }

    d->socket =
        static_cast<QSslSocket *>(KSocketFactory::connectToHost(d->protocol, d->server,
                                  d->port, this));

    d->socket->setProtocol(QSsl::AnyProtocol);

    connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            SLOT(slotStateChanged(QAbstractSocket::SocketState)));
    connect(d->socket, SIGNAL(modeChanged(QSslSocket::SslMode)),
            SLOT(slotModeChanged(QSslSocket::SslMode)));
    connect(d->socket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(d->socket, SIGNAL(readyRead()), SLOT(slotSocketRead()));
    connect(d->socket, SIGNAL(encrypted()), SIGNAL(connected()));
    connect(d->socket, SIGNAL(sslErrors(QList<QSslError>)),
            SLOT(slotSslErrors(QList<QSslError>)));
}

void Socket::write(const QString &text)
{
    // qDebug();
    // Eat things in the queue when there is no connection. We need
    // to get a connection first don't we...
    if (!d->socket || !available()) {
        return;
    }

    QByteArray cs = (text + QLatin1String("\r\n")).toLatin1();

#ifdef comm_debug
    qDebug() << "C   :" << cs;
#endif

    d->socket->write(cs.data(), cs.size());
}

bool Socket::available()
{
    // qDebug();
    bool ok = d->socket && d->socket->state() == QAbstractSocket::ConnectedState;
    return ok;
}

void Socket::startTLS()
{
    qDebug() << objectName();
    d->socket->setProtocol(QSsl::TlsV1);
    d->socket->startClientEncryption();
}

void Socket::setProtocol(const QString &proto)
{
    d->protocol = proto;
}

void Socket::setServer(const QString &server)
{
    d->server = server;
}

void Socket::setPort(int port)
{
    d->port = port;
}

void Socket::setSecure(bool what)
{
    d->secure = what;
}

#include "moc_socket.cpp"
