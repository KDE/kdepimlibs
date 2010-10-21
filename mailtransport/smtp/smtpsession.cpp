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
#include "smtp/response.h"
#include "smtp/command.h"
#include "smtp/transactionstate.h"

#include <ktcpsocket.h>
#include <KMessageBox>
#include <KIO/PasswordDialog>
#include <kio/authinfo.h>
#include <kio/global.h>
#include <KLocalizedString>
#include <KDebug>

using namespace MailTransport;
using namespace KioSMTP;

class MailTransport::SmtpSessionPrivate : public KioSMTP::SMTPSessionInterface
{
  public:
    explicit SmtpSessionPrivate( SmtpSession *session ) :
      useTLS( true ),
      socket( 0 ),
      currentCommand( 0 ),
      currentTransactionState( 0 ),
      state( Initial ),
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
      Q_ASSERT( socket );
      socket->setAdvertisedSslVersion( KTcpSocket::TlsV1 );
      socket->ignoreSslErrors();
      socket->startClientEncryption();
      const bool encrypted = socket->waitForEncrypted( -1 );

      const KSslCipher cipher = socket->sessionCipher();
      if ( !encrypted || socket->sslErrors().count() > 0 || socket->encryptionMode() != KTcpSocket::SslClientMode
           || cipher.isNull() || cipher.usedBits() == 0 )
      {
        kDebug() << "Initial SSL handshake failed. cipher.isNull() is" << cipher.isNull()
                 << ", cipher.usedBits() is" << cipher.usedBits()
                 << ", the socket says:" <<  socket->errorString()
                 << "and the list of SSL errors contains"
                 << socket->sslErrors().count() << "items.";
        return false;
      } else {
        kDebug() << "TLS negotiation done.";
        return true;
      }
    }

    bool lf2crlfAndDotStuffingRequested() const { return true; }
    QString requestedSaslMethod() const { return saslMethod; }
    TLSRequestState tlsRequested() const { return useTLS ? ForceTLS : ForceNoTLS; }

    void socketConnected()
    {
      kDebug();
    }

    bool sendCommandLine( const QByteArray &cmdline )
    {
      if ( cmdline.length() < 4096 )
        kDebug(7112) << "C: >>" << cmdline.trimmed().data() << "<<";
      else
        kDebug(7112) << "C: <" << cmdline.length() << " bytes>";
      ssize_t numWritten, cmdline_len = cmdline.length();
      if ( (numWritten = socket->write( cmdline ) ) != cmdline_len ) {
        kDebug(7112) << "Tried to write " << cmdline_len << " bytes, but only "
                    << numWritten << " were written!" << endl;
        error( KIO::ERR_SLAVE_DEFINED, i18n ("Writing to socket failed.") );
        return false;
      }
      return true;
    }

    bool run( int type, TransactionState * ts = 0 )
    {
      return run( Command::createSimpleCommand( type, this ), ts );
    }

    bool run( Command * cmd, TransactionState * ts = 0 )
    {
      Q_ASSERT( cmd );
      Q_ASSERT( !currentCommand );
      Q_ASSERT( !currentTransactionState );

      // ### WTF?
      if ( cmd->doNotExecute( ts ) )
        return true;

      currentCommand = cmd;
      currentTransactionState = ts;

      while ( !cmd->isComplete() && !cmd->needsResponse() ) {
        const QByteArray cmdLine = cmd->nextCommandLine( ts );
        if ( ts && ts->failedFatally() ) {
          // TODO error
          return false;
        }
        if ( cmdLine.isEmpty() )
          continue;
        if ( !sendCommandLine( cmdLine ) ) {
          // TODO error
          return false;
        }
      }
      return true;
    }

    void receivedNewData()
    {
      kDebug();
      while ( socket->canReadLine() ) {
        const QByteArray buffer = socket->readLine();
        kDebug() << "S: >>" << buffer << "<<";
        currentResponse.parseLine( buffer, buffer.size() );
        // ...until the response is complete or the parser is so confused
        // that it doesn't think a RSET would help anymore:
        if ( currentResponse.isComplete() ) {
          handleResponse( currentResponse );
          currentResponse = Response();
        } else if ( !currentResponse.isWellFormed() ) {
          // TODO error we can't recover from
        }
      }
    }

    void handleResponse( const KioSMTP::Response &response )
    {
      if ( currentCommand ) {
        if ( !currentCommand->processResponse( response, currentTransactionState ) ) {
          // TODO: error
        }
        while ( !currentCommand->isComplete() && !currentCommand->needsResponse() ) {
          const QByteArray cmdLine = currentCommand->nextCommandLine( currentTransactionState );
          if ( currentTransactionState && currentTransactionState->failedFatally() ) {
          // TODO error
          }
          if ( cmdLine.isEmpty() )
            continue;
          if ( !sendCommandLine( cmdLine ) ) {
          // TODO error
          }
        }
        if ( currentCommand->isComplete() ) {
          Command *cmd = currentCommand;
          currentCommand = 0;
          currentTransactionState = 0;
          handleCommand( cmd );
        }
        return;
      }

      // command-less responses
      switch ( state ) {
        case Initial: // server greeting
        {
          if ( !response.isOk() ) {
            error( KIO::ERR_COULD_NOT_LOGIN,
                  i18n("The server (%1) did not accept the connection.\n"
                        "%2", destination.host(), response.errorMessage() ) );
            // TODO error
          }
          state = EHLOPreTls;
          // TODO fake hostname handling
          EHLOCommand *ehloCmdPreTLS = new EHLOCommand( this, destination.host() );
          run( ehloCmdPreTLS );
          break;
        }
        default: Q_ASSERT( !"Unhandled command-less response." );
      }
    }

    void handleCommand( Command *cmd )
    {
      switch ( state ) {
        case EHLOPreTls:
        {
          if ( ( haveCapability("STARTTLS") && tlsRequested() != SMTPSessionInterface::ForceNoTLS )
              || tlsRequested() == SMTPSessionInterface::ForceTLS )
          {
            state = StartTLS;
            run( Command::STARTTLS );
            break;
          }
        }
        // fall through
        case EHLOPostTls:
        {
          authenticate();
          break;
        }
        case StartTLS:
        {
          // re-issue EHLO to refresh the capability list (could be have
          // been faked before TLS was enabled):
          state = EHLOPostTls;
          EHLOCommand *ehloCmdPostTLS = new EHLOCommand( this, destination.host() );
          run( ehloCmdPostTLS );
          break;
        }
        default: Q_ASSERT( !"Unhandled command" );
      }

      delete cmd;
    }

    void authenticate()
    {
      kDebug();
    }

  public:
    QString saslMethod;
    bool useTLS;

    KUrl destination;
    KTcpSocket *socket;
    QIODevice *data;
    KioSMTP::Response currentResponse;
    KioSMTP::Command * currentCommand;
    KioSMTP::TransactionState *currentTransactionState;

    enum State {
      Initial,
      EHLOPreTls,
      StartTLS,
      EHLOPostTls
    };
    State state;

  private:
    SmtpSession *q;
};

SmtpSession::SmtpSession(QObject* parent) :
  QObject(parent),
  d( new SmtpSessionPrivate( this ) )
{
  kDebug();
  d->socket = new KTcpSocket( this );
  connect( d->socket, SIGNAL(connected()), SLOT(socketConnected()) );
  connect( d->socket, SIGNAL(readyRead()), SLOT(receivedNewData()) );
}

SmtpSession::~SmtpSession()
{
  kDebug();
  delete d;
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
  kDebug() << url;
  if ( url.protocol() == QLatin1String( "smtps" ) )
    d->socket->connectToHostEncrypted( url.host(), url.port() );
  else if ( url.protocol() == QLatin1String( "smtp" ) )
    d->socket->connectToHost( url.host(), url.port() );
  else
    Q_ASSERT( !"Unsupported protocol!" );
}

void SmtpSession::sendMessage(const KUrl& destination, QIODevice* data)
{
  d->destination = destination;
  if ( d->socket->state() != KTcpSocket::ConnectedState && d->socket->state() != KTcpSocket::ConnectingState ) {
    connectToHost( destination );
  }

  d->data = data;
}

#include "smtpsession.moc"
