/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>
    Copyright (C) 2007 KovoKs <info@kovoks.nl>

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

// Own
#include "servertest.h"
#include "socket.h"

#include <mailtransport/transportbase.h>
#include <mailtransport/mailtransport_defs.h>

// Qt
#include <QProgressBar>
#include <QTimer>

// KDE
#include <klocale.h>
#include <kdebug.h>

using namespace MailTransport;

namespace MailTransport
{

class ServerTestPrivate
{
  public:
    ServerTestPrivate( ServerTest* test);

    ServerTest* const q;
    QString                   server;
    QString                   testProtocol;

    MailTransport::Socket*                   normalSocket;
    MailTransport::Socket*                   secureSocket;

    QList< int >              connectionResults;
    QHash< int, QList<int> >  authenticationResults;
    QTimer*                   normalSocketTimer;
    QTimer*                   secureSocketTimer;
    QTimer*                   progressTimer;

    QProgressBar*             testProgress;

    bool                      secureSocketFinished;
    bool                      normalSocketFinished;

    void finalResult();
    void read( int type, const QString& text );

    // slots
    void slotNormalPossible();
    void slotNormalNotPossible();
    void slotSslPossible();
    void slotSslNotPossible();
    void slotReadNormal( const QString& text );
    void slotReadSecure( const QString& text );
    void slotUpdateProgress();
};

}

ServerTestPrivate::ServerTestPrivate( ServerTest* test) : q( test )
{
}

void ServerTestPrivate::finalResult()
{
  if ( !secureSocketFinished || !normalSocketFinished )
    return;

  kDebug( 5324 ) << k_funcinfo << connectionResults;

  testProgress->hide();
  progressTimer->stop();

  emit q->finished( connectionResults );
}

void ServerTestPrivate::read( int type, const QString& text )
{
  kDebug( 5324 ) << k_funcinfo << text;

  if ( !text.contains( QLatin1String( "AUTH" ), Qt::CaseInsensitive ) )
    return;

  QStringList protocols;
  protocols << QLatin1String( "LOGIN" ) << QLatin1String( "PLAIN" )
      << QLatin1String( "CRAM-MD5" ) << QLatin1String("DIGEST-MD5")
      << QLatin1String( "NTLM" ) << QLatin1String( "GSSAPI" );

  QStringList results;
  for ( int i = 0 ; i < protocols.count(); ++i ) {
    if ( text.contains( protocols.at( i ), Qt::CaseInsensitive ) )
      results.append( protocols.at( i ) );
  }

  QList<int> result;
  for ( QStringList::ConstIterator it = results.begin() ;
        it != results.end() ; ++it )  {
    if (  *it == QLatin1String("LOGIN") )
      result << Transport::EnumAuthenticationType::LOGIN;
    else if ( *it == QLatin1String("PLAIN") )
      result << Transport::EnumAuthenticationType::PLAIN;
    else if ( *it == QLatin1String("CRAM-MD5") )
      result << Transport::EnumAuthenticationType::CRAM_MD5;
    else if ( *it == QLatin1String("DIGEST-MD5") )
      result << Transport::EnumAuthenticationType::DIGEST_MD5;
    else if ( *it == QLatin1String("NTLM") )
      result << Transport::EnumAuthenticationType::NTLM;
    else if ( *it == QLatin1String("GSSAPI") )
      result << Transport::EnumAuthenticationType::GSSAPI;
  }

  // LOGIN doesn't offer anything over PLAIN, requires more server
  // roundtrips and is not an official SASL mechanism, but a MS-ism,
  // so only enable it if PLAIN isn't available:
  if ( result.contains( Transport::EnumAuthenticationType::PLAIN ) )
    result.removeAll( Transport::EnumAuthenticationType::LOGIN );

  authenticationResults[type] = result;
}

void ServerTestPrivate::slotNormalPossible()
{
  normalSocketTimer->stop();
  connectionResults << Transport::EnumEncryption::None;
}

void ServerTestPrivate::slotReadNormal( const QString& text )
{
  static bool first = true;
  if ( first ) {
    if ( testProtocol == IMAP_PROTOCOL )
      normalSocket->write( QLatin1String( "1 CAPABILITY" ) );
    else if ( testProtocol == SMTP_PROTOCOL )
      normalSocket->write( QLatin1String( "EHLO localhost" ) );
    first = false;
    return;
  }

  if ( text.contains( QLatin1String("STARTTLS" ), Qt::CaseInsensitive) )
    connectionResults << Transport::EnumEncryption::TLS;

  read( Transport::EnumEncryption::None, text );

  normalSocketFinished = true;
  first = true;
  finalResult();
}

void ServerTestPrivate::slotReadSecure( const QString& text )
{
  static bool first = true;
  if ( first ) {
    if ( testProtocol == IMAPS_PROTOCOL )
      secureSocket->write( QLatin1String( "1 CAPABILITY" ) );
    else if ( testProtocol == SMTPS_PROTOCOL )
      secureSocket->write( QLatin1String( "EHLO localhost" ) );
    first = false;
    return;
  }

  read( Transport::EnumEncryption::SSL, text );

  secureSocketFinished = true;
  first = true;
  finalResult();
}

void ServerTestPrivate::slotNormalNotPossible()
{
  normalSocketFinished = true;
  finalResult();
}

void ServerTestPrivate::slotSslPossible()
{
  secureSocketTimer->stop();
  connectionResults << Transport::EnumEncryption::SSL;
  secureSocketFinished = true;
  finalResult();
}

void ServerTestPrivate::slotSslNotPossible()
{
  secureSocketFinished = true;
  finalResult();
}

void ServerTestPrivate::slotUpdateProgress()
{
  testProgress->setValue( testProgress->value() + 1 );
}

//---------------------- end private class -----------------------//

ServerTest::ServerTest( QWidget* parent )
  : QWidget( parent ), d( new ServerTestPrivate( this ) )
{
  d->normalSocketTimer = new QTimer( this );
  d->normalSocketTimer->setSingleShot( true );
  connect( d->normalSocketTimer, SIGNAL( timeout() ), SLOT( slotNormalNotPossible() ) );

  d->secureSocketTimer = new QTimer( this );
  d->secureSocketTimer->setSingleShot( true );
  connect( d->secureSocketTimer, SIGNAL( timeout() ), SLOT( slotSslNotPossible() ) );

  d->progressTimer = new QTimer( this );
  connect( d->progressTimer, SIGNAL( timeout() ), SLOT( slotUpdateProgress() ) );
}

ServerTest::~ServerTest()
{
  delete d;
}

void ServerTest::start()
{
  kDebug( 5324 ) << k_funcinfo << d;

  d->connectionResults.clear();
  d->authenticationResults.clear();

  d->testProgress->setMaximum( 20 );
  d->testProgress->setValue( 0 );
  d->testProgress->setTextVisible( true );
  d->testProgress->show();
  d->progressTimer->start( 1000 );

  d->normalSocket = new MailTransport::Socket( this );
  d->normalSocket->setObjectName( QLatin1String( "normal" ) );
  d->normalSocket->setServer( d->server );
  d->normalSocket->setProtocol( d->testProtocol );
  if ( d->testProtocol == IMAP_PROTOCOL )
    d->normalSocket->setPort( IMAP_PORT );
  else
    d->normalSocket->setPort( SMTP_PORT );
  connect( d->normalSocket, SIGNAL( connected() ), SLOT( slotNormalPossible() ) );
  connect( d->normalSocket, SIGNAL( failed() ), SLOT( slotNormalNotPossible() ) );
  connect( d->normalSocket, SIGNAL( data( const QString& ) ),
           SLOT( slotReadNormal( const QString& ) ) );
  d->normalSocket->reconnect();
  d->normalSocketTimer->start( 10000 );

  d->secureSocket = new MailTransport::Socket( this );
  d->secureSocket->setObjectName( QLatin1String( "secure" ) );
  d->secureSocket->setServer( d->server );
  d->secureSocket->setProtocol( d->testProtocol + QLatin1Char( 's' ) );
  if ( d->testProtocol == IMAP_PROTOCOL)
    d->secureSocket->setPort( IMAPS_PORT );
  else
    d->secureSocket->setPort( SMTPS_PORT );
  d->secureSocket->setSecure( true );
  connect( d->secureSocket, SIGNAL( connected() ), SLOT( slotSslPossible() ) );
  connect( d->secureSocket, SIGNAL( failed() ), SLOT( slotSslNotPossible() ) );
  connect( d->secureSocket, SIGNAL( data( const QString& ) ),
           SLOT( slotReadSecure( const QString& ) ) );
  d->secureSocket->reconnect();
  d->secureSocketTimer->start( 10000 );
}

void ServerTest::setServer( const QString& server )
{
  d->server = server;
}

void ServerTest::setProgressBar( QProgressBar* pb )
{
  d->testProgress = pb;
}

void ServerTest::setProtocol( const QString& protocol )
{
  d->testProtocol = protocol;
}

QString ServerTest::protocol()
{
  return d->testProtocol;
}

QString ServerTest::server()
{
  return d->server;
}

QProgressBar* ServerTest::progressBar()
{
  return d->testProgress;
}

QList< int > ServerTest::normalProtocols()
{
  return d->authenticationResults[TransportBase::EnumEncryption::None];
}

QList< int > ServerTest::secureProtocols()
{
  return d->authenticationResults[Transport::EnumEncryption::SSL];
}


#include "servertest.moc"
