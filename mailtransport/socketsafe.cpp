/* This file is part of the KDE project
   Copyright (C) 2006-2007 KovoKs <info@kovoks.nl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

// Uncomment the next line for full comm debug
#define comm_debug

// Own
#include "socketsafe.h"

// Qt
#include <QRegExp>
#include <QByteArray>
#include <QSslSocket>

// KDE
#include <kdebug.h>
#include <ksocketfactory.h>
#include <klocale.h>

namespace MailTransport {

SocketSafe::SocketSafe(QObject* parent)
    :QObject(parent), m_socket(0), m_port(0), m_aboutToClose(false),
             m_secure(false), m_tls(false)
{
    kDebug(50002) << k_funcinfo << endl;
}

SocketSafe::~SocketSafe()
{
    kDebug(50002) << objectName() << " " << k_funcinfo << endl;
    m_aboutToClose=true;
}

void SocketSafe::reconnect()
{
    kDebug(50002) << k_funcinfo << endl;
    if (m_aboutToClose)
        return;

    kDebug(50002) << objectName() << " " << "Connecting to: " << m_server << ":"
            <<  m_port << endl;

#ifdef comm_debug
    // kDebug(50002) << objectName() << m_proto << endl;
#endif

    if (m_socket)
        return;

    m_socket = static_cast<QSslSocket*>
             (KSocketFactory::connectToHost(m_proto, m_server, m_port, this));

    m_socket->setProtocol(QSslSocket::AnyProtocol);

    connect(m_socket, SIGNAL(stateChanged( QAbstractSocket::SocketState )),
            SLOT(slotStateChanged( QAbstractSocket::SocketState )));
    connect(m_socket, SIGNAL(modeChanged(  QSslSocket::SslMode  )),
            SLOT(slotModeChanged(  QSslSocket::SslMode  )));
    connect(m_socket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_socket, SIGNAL(readyRead()), SLOT(slotSocketRead()) );
    connect(m_socket, SIGNAL(encrypted()), SIGNAL(connected()) );
    connect(m_socket, SIGNAL(sslErrors( const QList<QSslError> &)),
                SLOT( slotSslErrors ( const QList<QSslError>& )));
}

void SocketSafe::slotConnected()
{
    kDebug(50002) << k_funcinfo << endl;

    if ( !m_secure || m_tls)
    {
        kDebug(50002) << "normal connect" << endl;
        emit connected();
    }
    else
    {
        kDebug(50002) << "encrypted connect" << endl;
        startShake();
    }
}

void SocketSafe::slotStateChanged( QAbstractSocket::SocketState state)
{
#ifdef comm_debug
    kDebug(50002) << objectName() << " "
            << "State is now: " << (int)state << endl;
#endif
    if (state == QAbstractSocket::UnconnectedState)
	emit failed();
}

void SocketSafe::slotModeChanged(  QSslSocket::SslMode  state)
{
#ifdef comm_debug
    kDebug(50002) << objectName() << " "
            << "Mode is now: " << state << endl;
#endif

}
void SocketSafe::slotSocketRead()
{
    // kDebug(50002) << objectName() << " " << k_funcinfo << endl;

    if (!m_socket)
        return;

    static QString msg;
    msg += QLatin1String(m_socket->readAll());

    if (!msg.endsWith(QLatin1Char('\n')))
        return;
    
#ifdef comm_debug
    kDebug(50002) << objectName() << " " << m_socket->isEncrypted()
                << msg.trimmed() << endl;
#endif

    emit data(msg);
    msg.clear();
}

void SocketSafe::startShake()
{
    kDebug(50002) << objectName() << " " << k_funcinfo << endl;
    m_socket->startClientEncryption();
}

void SocketSafe::slotSslErrors(const QList<QSslError> & errors)
{
    kDebug(50002) << objectName() << " " << k_funcinfo << endl;
    emit connected();
}

void SocketSafe::write(const QString& text)
{
    // kDebug(50002) << objectName() << " " << k_funcinfo << endl;
    // Eat things in the queue when there is no connection. We need
    // to get a connection first don't we...
    if (!m_socket || !available())
        return;

    QByteArray cs = (text+QLatin1String("\r\n")).toLatin1();

#ifdef comm_debug
    kDebug(50002) << objectName() << " " << "C   : "
          << cs << endl;
#endif

    m_socket->write(cs.data(), cs.size());
}

bool SocketSafe::available()
{
    // kDebug(50002) << objectName() << " " << k_funcinfo << endl;
    bool ok = m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
    return ok;
}

}

#include "socketsafe.moc"
