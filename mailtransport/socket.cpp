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
#include <QRegExp>
#include <QByteArray>
#include <QSslSocket>

// KDE
#include <kdebug.h>
#include <ksocketfactory.h>
#include <klocale.h>

using namespace MailTransport;

Socket::Socket( QObject* parent )
    : QObject( parent ), m_socket( 0 ), m_port( 0 ),  m_secure( false )
{
  kDebug( 5324 ) << k_funcinfo << endl;
}

Socket::~Socket()
{
  kDebug( 5324 ) << objectName() << " " << k_funcinfo << endl;
}

void Socket::reconnect()
{
  kDebug( 5324 ) << objectName() << " " << "Connecting to: " << m_server
          <<  ":" <<  m_port << endl;

#ifdef comm_debug
  // kDebug(5324) << objectName() << m_proto << endl;
#endif

  if ( m_socket )
    return;

  m_socket = static_cast<QSslSocket*>
             ( KSocketFactory::connectToHost( m_proto, m_server, m_port, this
                                            ) );

  m_socket->setProtocol( QSslSocket::AnyProtocol );

  connect( m_socket, SIGNAL( stateChanged( QAbstractSocket::SocketState ) ),
           SLOT( slotStateChanged( QAbstractSocket::SocketState ) ) );
  connect( m_socket, SIGNAL( modeChanged( QSslSocket::SslMode ) ),
           SLOT( slotModeChanged( QSslSocket::SslMode ) ) );
  connect( m_socket, SIGNAL( connected() ), SLOT( slotConnected() ) );
  connect( m_socket, SIGNAL( readyRead() ), SLOT( slotSocketRead() ) );
  connect( m_socket, SIGNAL( encrypted() ), SIGNAL( connected() ) );
  connect( m_socket, SIGNAL( sslErrors( const QList<QSslError> & ) ),
           SLOT( slotSslErrors( const QList<QSslError>& ) ) );
}

void Socket::slotConnected()
{
  kDebug( 5324 ) << k_funcinfo << endl;

  if ( !m_secure ) {
    kDebug( 5324 ) << "normal connect" << endl;
    emit connected();
  } else {
    kDebug( 5324 ) << "encrypted connect" << endl;
    startShake();
  }
}

void Socket::slotStateChanged( QAbstractSocket::SocketState state )
{
#ifdef comm_debug
  kDebug( 5324 ) << objectName() << " "
          << "State is now: " << ( int ) state << endl;
#endif
  if ( state == QAbstractSocket::UnconnectedState )
    emit failed();
}

void Socket::slotModeChanged( QSslSocket::SslMode  state )
{
#ifdef comm_debug
  kDebug( 5324 ) << objectName() << " "  << "Mode is now: " << state << endl;
#endif
}

void Socket::slotSocketRead()
{
  // kDebug(5324) << objectName() << " " << k_funcinfo << endl;

  if ( !m_socket )
    return;

  static QString msg;
  msg += QLatin1String( m_socket->readAll() );

  if ( !msg.endsWith( QLatin1Char( '\n' ) ) )
    return;

#ifdef comm_debug
  kDebug( 5324 ) << objectName() << " " << m_socket->isEncrypted()
          << msg.trimmed() << endl;
#endif

  emit data( msg );
  msg.clear();
}

void Socket::startShake()
{
  kDebug( 5324 ) << objectName() << " " << k_funcinfo << endl;
  m_socket->startClientEncryption();
}

void Socket::slotSslErrors( const QList<QSslError> & )
{
  kDebug( 5324 ) << objectName() << " " << k_funcinfo << endl;
  /* We can safely ignore the errors, we are only interested in the
     capabilities. We're not sending auth info. */
  m_socket->ignoreSslErrors();
  emit connected();
}

void Socket::write( const QString& text )
{
  // kDebug(5324) << objectName() << " " << k_funcinfo << endl;
  // Eat things in the queue when there is no connection. We need
  // to get a connection first don't we...
  if ( !m_socket || !available() )
    return;

  QByteArray cs = ( text + QLatin1String( "\r\n" ) ).toLatin1();

#ifdef comm_debug
  kDebug( 5324 ) << objectName() << " " << "C   : "
  << cs << endl;
#endif

  m_socket->write( cs.data(), cs.size() );
}

bool Socket::available()
{
  // kDebug(5324) << objectName() << " " << k_funcinfo << endl;
  bool ok = m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
  return ok;
}

#include "socket.moc"
