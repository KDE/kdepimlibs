/*
 * Copyright (c) 2000, 2001 Alex Zepeda <zipzippy@sonic.net>
 * Copyright (c) 2001 Michael H�kel <Michael@Haeckel.Net>
 * Copyright (c) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
 * Copyright (c) 2003 Marc Mutz <mutz@kde.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "smtp.h"
#include "smtp_debug.h"

extern "C" {
#include <sasl/sasl.h>
}

#include "../common.h"
#include "request.h"
#include "response.h"
#include "transactionstate.h"
#include "command.h"
#include "kioslavesession.h"
using KioSMTP::Capabilities;
using KioSMTP::Command;
using KioSMTP::EHLOCommand;
using KioSMTP::AuthCommand;
using KioSMTP::MailFromCommand;
using KioSMTP::RcptToCommand;
using KioSMTP::DataCommand;
using KioSMTP::TransferCommand;
using KioSMTP::Request;
using KioSMTP::Response;
using KioSMTP::TransactionState;
using KioSMTP::SMTPSessionInterface;

#include <kemailsettings.h>

#include <qdebug.h>
#include <kio/slaveinterface.h>
#include <klocalizedstring.h>
#include <QUrl>

#include <QHostInfo>

#include <memory>
using std::auto_ptr;

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <netdb.h>

extern "C" {
  Q_DECL_EXPORT int kdemain(int argc, char **argv);
}

int kdemain(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  app.setApplicationName(QLatin1String("kio_smtp"));

  if (argc != 4) {
    fprintf(stderr,
            "Usage: kio_smtp protocol domain-socket1 domain-socket2\n");
    exit(-1);
  }

  if (!initSASL())
    exit(-1);
  SMTPProtocol slave( argv[2], argv[3], qstricmp( argv[1], "smtps" ) == 0 );
  slave.dispatchLoop();
  sasl_done();
  return 0;
}

SMTPProtocol::SMTPProtocol(const QByteArray & pool, const QByteArray & app,
                           bool useSSL)
:  TCPSlaveBase(useSSL ? "smtps" : "smtp", pool, app, useSSL),
   m_sOldPort( 0 ),
   m_opened(false),
   m_sessionIface( new KioSMTP::KioSlaveSession( this ) )
{
  //qCDebug(SMTP_LOG) << "SMTPProtocol::SMTPProtocol";
}

SMTPProtocol::~SMTPProtocol() {
  //qCDebug(SMTP_LOG) << "SMTPProtocol::~SMTPProtocol";
  smtp_close();
  delete m_sessionIface;
}

void SMTPProtocol::openConnection() {

  // Don't actually call smtp_open() yet. Just pretend that we are connected.
  // We can't call smtp_open() here, as that does EHLO, and the EHLO command
  // needs the fake hostname. However, we only get the fake hostname in put(), so
  // we call smtp_open() there.
  connected();
}

void SMTPProtocol::closeConnection() {
  smtp_close();
}

void SMTPProtocol::special( const QByteArray & aData ) {
  QDataStream s( aData );
  int what;
  s >> what;
  if ( what == 'c' ) {
    const QString response = m_sessionIface->capabilities().createSpecialResponse(
      ( isUsingSsl() && !isAutoSsl() )
      || m_sessionIface->haveCapability( "STARTTLS" )
    );
    infoMessage( response );
  } else if ( what == 'N' ) {
    if ( !execute( Command::NOOP ) )
      return;
  } else {
    error( KIO::ERR_INTERNAL, i18n("The application sent an invalid request.") );
    return;
  }
  finished();
}


// Usage: smtp://smtphost:port/send?to=user@host.com&subject=blah
// If smtphost is the name of a profile, it'll use the information
// provided by that profile.  If it's not a profile name, it'll use it as
// nature intended.
// One can also specify in the query:
// headers=0 (turns off header generation)
// to=emailaddress
// cc=emailaddress
// bcc=emailaddress
// subject=text
// profile=text (this will override the "host" setting)
// hostname=text (used in the HELO)
// body={7bit,8bit} (default: 7bit; 8bit activates the use of the 8BITMIME SMTP extension)
void SMTPProtocol::put(const QUrl & url, int /*permissions */ ,
                       KIO::JobFlags)
{
  Request request = Request::fromURL( url ); // parse settings from URL's query

  KEMailSettings mset;
  QUrl open_url = url;
  if ( !request.hasProfile() ) {
    //qCDebug(SMTP_LOG) << "kio_smtp: Profile is null";
    bool hasProfile = mset.profiles().contains( open_url.host() );
    if ( hasProfile ) {
      mset.setProfile(open_url.host());
      open_url.setHost(mset.getSetting(KEMailSettings::OutServer));
      m_sUser = mset.getSetting(KEMailSettings::OutServerLogin);
      m_sPass = mset.getSetting(KEMailSettings::OutServerPass);

      if (m_sUser.isEmpty())
        m_sUser.clear();
      if (m_sPass.isEmpty())
        m_sPass.clear();
      open_url.setUserName(m_sUser);
      open_url.setPassword(m_sPass);
      m_sServer = open_url.host();
      m_port = open_url.port();
    }
    else {
      mset.setProfile(mset.defaultProfileName());
    }
  }
  else {
    mset.setProfile( request.profileName() );
  }

  // Check KEMailSettings to see if we've specified an E-Mail address
  // if that worked, check to see if we've specified a real name
  // and then format accordingly (either: emailaddress@host.com or
  // Real Name <emailaddress@host.com>)
  if ( !request.hasFromAddress() ) {
    const QString from = mset.getSetting( KEMailSettings::EmailAddress );
    if ( !from.isNull() )
      request.setFromAddress( from );
    else if ( request.emitHeaders() ) {
      error(KIO::ERR_NO_CONTENT, i18n("The sender address is missing."));
      return;
    }
  }

  if ( !smtp_open( request.heloHostname() ) )
  {
    error(KIO::ERR_SERVICE_NOT_AVAILABLE,
          i18n("SMTPProtocol::smtp_open failed (%1)", // ### better error message?
               open_url.path()));
    return;
  }

  if ( request.is8BitBody()
       && !m_sessionIface->haveCapability("8BITMIME") && !m_sessionIface->eightBitMimeRequested() ) {
    error( KIO::ERR_SERVICE_NOT_AVAILABLE,
           i18n("Your server (%1) does not support sending of 8-bit messages.\n"
                "Please use base64 or quoted-printable encoding.", m_sServer) );
    return;
  }

  queueCommand( new MailFromCommand( m_sessionIface, request.fromAddress().toLatin1(),
                                     request.is8BitBody(), request.size() ) );

  // Loop through our To and CC recipients, and send the proper
  // SMTP commands, for the benefit of the server.
  const QStringList recipients = request.recipients();
  for ( QStringList::const_iterator it = recipients.begin() ; it != recipients.end() ; ++it )
    queueCommand( new RcptToCommand( m_sessionIface, (*it).toLatin1() ) );

  queueCommand( Command::DATA );
  queueCommand( new TransferCommand( m_sessionIface, request.headerFields( mset.getSetting( KEMailSettings::RealName ) ) ) );

  TransactionState ts;
  if ( !executeQueuedCommands( &ts ) ) {
    if ( ts.errorCode() )
      error( ts.errorCode(), ts.errorMessage() );
  } else
    finished();
}


void SMTPProtocol::setHost(const QString & host, quint16 port,
                           const QString & user, const QString & pass)
{
  m_sServer = host;
  m_port = port;
  m_sUser = user;
  m_sPass = pass;
}

bool SMTPProtocol::sendCommandLine( const QByteArray & cmdline ) {
  //kDebug( cmdline.length() < 4096, 7112) << "C: " << cmdline.data();
  //kDebug( cmdline.length() >= 4096, 7112) << "C: <" << cmdline.length() << " bytes>";
  if ( cmdline.length() < 4096 )
    qCDebug(SMTP_LOG) << "C: >>" << cmdline.trimmed().data() << "<<";
  else
    qCDebug(SMTP_LOG) << "C: <" << cmdline.length() << " bytes>";
  ssize_t numWritten, cmdline_len = cmdline.length();
  if ( (numWritten = write( cmdline.data(), cmdline_len ) )!= cmdline_len ) {
    qCDebug(SMTP_LOG) << "Tried to write " << cmdline_len << " bytes, but only "
                 << numWritten << " were written!" << endl;
    error( KIO::ERR_SLAVE_DEFINED, i18n ("Writing to socket failed.") );
    return false;
  }
  return true;
}

Response SMTPProtocol::getResponse( bool * ok ) {

  if ( ok )
    *ok = false;

  Response response;
  char buf[2048];

  int recv_len = 0;
  do {
    // wait for data...
    if ( !waitForResponse( 600 ) ) {
      error( KIO::ERR_SERVER_TIMEOUT, m_sServer );
      return response;
    }

    // ...read data...
    recv_len = readLine( buf, sizeof(buf) - 1 );
    if ( recv_len < 1 && !isConnected() ) {
      error( KIO::ERR_CONNECTION_BROKEN, m_sServer );
      return response;
    }

    qCDebug(SMTP_LOG) << "S: >>" << QByteArray( buf, recv_len ).trimmed().data() << "<<";
    // ...and parse lines...
    response.parseLine( buf, recv_len );

    // ...until the response is complete or the parser is so confused
    // that it doesn't think a RSET would help anymore:
  } while ( !response.isComplete() && response.isWellFormed() );

  if ( !response.isValid() ) {
    error( KIO::ERR_NO_CONTENT, i18n("Invalid SMTP response (%1) received.", response.code()) );
    return response;
  }

  if ( ok )
    *ok = true;

  return response;
}

bool SMTPProtocol::executeQueuedCommands( TransactionState * ts ) {
  assert( ts );

  if (m_sessionIface->canPipelineCommands())
     qDebug() << "using pipelining";

  while( !mPendingCommandQueue.isEmpty() ) {
    QByteArray cmdline = collectPipelineCommands( ts );
    if ( ts->failedFatally() ) {
      smtp_close( false ); // _hard_ shutdown
      return false;
    }
    if ( ts->failed() )
      break;
    if ( cmdline.isEmpty() )
      continue;
    if ( !sendCommandLine( cmdline ) ||
         !batchProcessResponses( ts ) ||
         ts->failedFatally() ) {
      smtp_close( false ); // _hard_ shutdown
      return false;
    }
  }

  if ( ts->failed() ) {
    if ( !execute( Command::RSET ) )
      smtp_close( false );
    return false;
  }
  return true;
}

QByteArray SMTPProtocol::collectPipelineCommands( TransactionState * ts ) {
  assert( ts );

  QByteArray cmdLine;
  unsigned int cmdLine_len = 0;

  while ( !mPendingCommandQueue.isEmpty() ) {

    Command * cmd = mPendingCommandQueue.head();

    if ( cmd->doNotExecute( ts ) ) {
      delete mPendingCommandQueue.dequeue();
      if ( cmdLine_len )
        break;
      else
        continue;
    }

    if ( cmdLine_len && cmd->mustBeFirstInPipeline() )
      break;

    if ( cmdLine_len && !m_sessionIface->canPipelineCommands() )
      break;

    while ( !cmd->isComplete() && !cmd->needsResponse() ) {
      const QByteArray currentCmdLine = cmd->nextCommandLine( ts );
      if ( ts->failedFatally() )
        return cmdLine;
      const unsigned int currentCmdLine_len = currentCmdLine.length();

      cmdLine_len += currentCmdLine_len;
      cmdLine += currentCmdLine;

      // If we are executing the transfer command, don't collect the whole
      // command line (which may be several MBs) before sending it, but instead
      // send the data each time we have collected 32 KB of the command line.
      //
      // This way, the progress information in clients like KMail works correctly,
      // because otherwise, the TransferCommand would read the whole data from the
      // job at once, then sending it. The progress update on the client however
      // happens when sending data to the job, not when this slave writes the data
      // to the socket. Therefore that progress update is incorrect.
      //
      // 32 KB seems to be a sensible limit. Additionally, a job can only transfer
      // 32 KB at once anyway.
      if ( dynamic_cast<TransferCommand *>( cmd ) != 0 &&
           cmdLine_len >= 32 * 1024 ) {
        return cmdLine;
      }
    }

    mSentCommandQueue.enqueue( mPendingCommandQueue.dequeue() );

    if ( cmd->mustBeLastInPipeline() )
      break;
  }

  return cmdLine;
}

bool SMTPProtocol::batchProcessResponses( TransactionState * ts ) {
  assert( ts );

  while ( !mSentCommandQueue.isEmpty() ) {

    Command * cmd = mSentCommandQueue.head();
    assert( cmd->isComplete() );

    bool ok = false;
    Response r = getResponse( &ok );
    if ( !ok )
      return false;
    cmd->processResponse( r, ts );
    if ( ts->failedFatally() )
      return false;

    delete mSentCommandQueue.dequeue();
  }

  return true;
}

void SMTPProtocol::queueCommand( int type ) {
  queueCommand( Command::createSimpleCommand( type, m_sessionIface ) );
}

bool SMTPProtocol::execute( int type, TransactionState * ts ) {
  auto_ptr<Command> cmd( Command::createSimpleCommand( type, m_sessionIface ) );
  if (!cmd.get())
    qCritical() << "Command::createSimpleCommand( " << type << " ) returned null!" ;
  return execute( cmd.get(), ts );
}

// ### fold into pipelining engine? How? (execute() is often called
// ### when command queues are _not_ empty!)
bool SMTPProtocol::execute( Command * cmd, TransactionState * ts ) {

  if( !cmd )
    qCritical( ) << "SMTPProtocol::execute() called with no command to run!" ;

  if ( cmd->doNotExecute( ts ) )
    return true;

  do {
    while ( !cmd->isComplete() && !cmd->needsResponse() ) {
      const QByteArray cmdLine = cmd->nextCommandLine( ts );
      if ( ts && ts->failedFatally() ) {
        smtp_close( false );
        return false;
      }
      if ( cmdLine.isEmpty() )
        continue;
      if ( !sendCommandLine( cmdLine ) ) {
        smtp_close( false );
        return false;
      }
    }

    bool ok = false;
    Response r = getResponse( &ok );
    if ( !ok ) {
      // Only close without sending QUIT if the responce was incomplete
      // rfc5321 forbidds a client from closing a connection without sending
      // QUIT (section 4.1.1.10)
      if ( r.isComplete() ) {
        smtp_close();
      } else {
        smtp_close( false );
      }
      return false;
    }
    if ( !cmd->processResponse( r, ts ) ) {
      if ( (ts && ts->failedFatally()) ||
           cmd->closeConnectionOnError() ||
           !execute( Command::RSET ) )
        smtp_close( false );
      return false;
    }
  } while ( !cmd->isComplete() );

  return true;
}

bool SMTPProtocol::smtp_open(const QString& fakeHostname)
{
  if (m_opened &&
      m_sOldPort == m_port &&
      m_sOldServer == m_sServer &&
      m_sOldUser == m_sUser &&
      (fakeHostname.isNull() || m_hostname == fakeHostname))
    return true;

  smtp_close();
  if (!connectToHost(isAutoSsl() ? QLatin1String("smtps") : QLatin1String("smtp"), m_sServer, m_port))
    return false;             // connectToHost has already send an error message.
  m_opened = true;

  bool ok = false;
  Response greeting = getResponse( &ok );
  if ( !ok || !greeting.isOk() )
  {
    if ( ok )
      error( KIO::ERR_COULD_NOT_LOGIN,
             i18n("The server (%1) did not accept the connection.\n"
                  "%2", m_sServer,  greeting.errorMessage() ) );
    smtp_close();
    return false;
  }

  if ( !fakeHostname.isNull() ) {
    m_hostname = fakeHostname;
  }
  else {
   // FIXME: We need a way to find the FQDN again. Also change in servertest then.
    m_hostname = QHostInfo::localHostName();
    if( m_hostname.isEmpty() ) {
      m_hostname = QLatin1String("localhost.invalid");
    }
    else if ( !m_hostname.contains( QLatin1Char('.') ) ) {
      m_hostname += QLatin1String(".localnet");
    }
  }

  EHLOCommand ehloCmdPreTLS( m_sessionIface, m_hostname );
  if ( !execute( &ehloCmdPreTLS ) ) {
    smtp_close();
    return false;
  }

  if ( ( m_sessionIface->haveCapability("STARTTLS") /*### && canUseTLS()*/ && m_sessionIface->tlsRequested() != SMTPSessionInterface::ForceNoTLS )
       || m_sessionIface->tlsRequested() == SMTPSessionInterface::ForceTLS ) {
    // For now we're gonna force it on.

    if ( execute( Command::STARTTLS ) ) {

      // re-issue EHLO to refresh the capability list (could be have
      // been faked before TLS was enabled):
      EHLOCommand ehloCmdPostTLS( m_sessionIface, m_hostname );
      if ( !execute( &ehloCmdPostTLS ) ) {
        smtp_close();
        return false;
      }
    }
  }
  // Now we try and login
  if (!authenticate()) {
    smtp_close();
    return false;
  }

  m_sOldPort = m_port;
  m_sOldServer = m_sServer;
  m_sOldUser = m_sUser;
  m_sOldPass = m_sPass;

  return true;
}

bool SMTPProtocol::authenticate()
{
  // return with success if the server doesn't support SMTP-AUTH or an user
  // name is not specified and metadata doesn't tell us to force it.
  if ( (m_sUser.isEmpty() || !m_sessionIface->haveCapability( "AUTH" )) &&
    m_sessionIface->requestedSaslMethod().isEmpty() ) return true;

  KIO::AuthInfo authInfo;
  authInfo.username = m_sUser;
  authInfo.password = m_sPass;
  authInfo.prompt = i18n("Username and password for your SMTP account:");

  QStringList strList;

  if (!m_sessionIface->requestedSaslMethod().isEmpty())
    strList.append( m_sessionIface->requestedSaslMethod() );
  else
    strList = m_sessionIface->capabilities().saslMethodsQSL();

  const QByteArray ba = strList.join( QLatin1String(" ") ).toLatin1();
  AuthCommand authCmd( m_sessionIface, ba.constData(), m_sServer, authInfo );
  bool ret = execute( &authCmd );
  m_sUser = authInfo.username;
  m_sPass = authInfo.password;
  return ret;
}

void SMTPProtocol::smtp_close( bool nice ) {
  if (!m_opened)                  // We're already closed
    return;

  if ( nice )
    execute( Command::QUIT );
  qCDebug(SMTP_LOG) << "closing connection";
  disconnectFromHost();
  m_sOldServer.clear();
  m_sOldUser.clear();
  m_sOldPass.clear();

  m_sessionIface->clearCapabilities();
  qDeleteAll( mPendingCommandQueue );
  mPendingCommandQueue.clear();
  qDeleteAll( mSentCommandQueue );
  mSentCommandQueue.clear();

  m_opened = false;
}

void SMTPProtocol::stat(const QUrl & url)
{
  QString path = url.path();
  error(KIO::ERR_DOES_NOT_EXIST, url.path());
}

