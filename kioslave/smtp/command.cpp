/*  -*- c++ -*-
    command.cc

    This file is part of kio_smtp, the KDE SMTP kioslave.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "command.h"

#include "smtpsessioninterface.h"
#include "response.h"
#include "transactionstate.h"

#include <klocale.h>
#include <kdebug.h>
#include <kcodecs.h>
#include <kio/slavebase.h> // for test_commands, where SMTPProtocol is not derived from TCPSlaveBase

#include <QtCore/QUrl>

#include <assert.h>

namespace KioSMTP {

static sasl_callback_t callbacks[] = {
    { SASL_CB_ECHOPROMPT, NULL, NULL },
    { SASL_CB_NOECHOPROMPT, NULL, NULL },
    { SASL_CB_GETREALM, NULL, NULL },
    { SASL_CB_USER, NULL, NULL },
    { SASL_CB_AUTHNAME, NULL, NULL },
    { SASL_CB_PASS, NULL, NULL },
    { SASL_CB_CANON_USER, NULL, NULL },
    { SASL_CB_LIST_END, NULL, NULL }
};

  //
  // Command (base class)
  //

  Command::Command( SMTPSessionInterface * smtp, int flags )
    : mSMTP( smtp ),
      mComplete( false ), mNeedResponse( false ), mFlags( flags )
  {
    assert( smtp );
  }

  Command::~Command() {}

  bool Command::processResponse( const Response & r, TransactionState * ) {
    mComplete = true;
    mNeedResponse = false;
    return r.isOk();
  }

  void Command::ungetCommandLine( const QByteArray &, TransactionState * ) {
    mComplete = false;
  }

  Command * Command::createSimpleCommand( int which, SMTPSessionInterface * smtp ) {
    switch ( which ) {
    case STARTTLS: return new StartTLSCommand( smtp );
    case DATA:     return new DataCommand( smtp );
    case NOOP:     return new NoopCommand( smtp );
    case RSET:     return new RsetCommand( smtp );
    case QUIT:     return new QuitCommand( smtp );
    default:       return 0;
    }
  }

  //
  // relay methods:
  //

  void Command::parseFeatures( const Response & r ) {
    mSMTP->parseFeatures( r );
  }

  int Command::startSsl() {
    return mSMTP->startSsl();
  }

  bool Command::haveCapability( const char * cap ) const {
    return mSMTP->haveCapability( cap );
  }

  //
  // EHLO / HELO
  //

  QByteArray EHLOCommand::nextCommandLine( TransactionState * ) {
    mNeedResponse = true;
    mComplete = mEHLONotSupported;
    const char * cmd = mEHLONotSupported ? "HELO " : "EHLO " ;
    return cmd + QUrl::toAce( mHostname ) + "\r\n"; //krazy:exclude=qclasses
  }

  bool EHLOCommand::processResponse( const Response & r, TransactionState * ) {
    mNeedResponse = false;
    // "command not {recognized,implemented}" response:
    if ( r.code() == 500 || r.code() == 502 ) {
      if ( mEHLONotSupported ) { // HELO failed...
        mSMTP->error( KIO::ERR_INTERNAL_SERVER,
                      i18n("The server rejected both EHLO and HELO commands "
                           "as unknown or unimplemented.\n"
                           "Please contact the server's system administrator.") );
        return false;
      }
      mEHLONotSupported = true; // EHLO failed, but that's ok.
      return true;
    }
    mComplete = true;
    if ( r.code() / 10 ==  25 ) { // 25x: success
      parseFeatures( r );
      return true;
    }
    mSMTP->error( KIO::ERR_UNKNOWN,
                  i18n( "Unexpected server response to %1 command.\n%2",
                        QString::fromLatin1( mEHLONotSupported ? "HELO" : "EHLO" ),
                        r.errorMessage() ) );
    return false;
  }

  //
  // STARTTLS - rfc 3207
  //

  QByteArray StartTLSCommand::nextCommandLine( TransactionState * ) {
    mComplete = true;
    mNeedResponse = true;
    return "STARTTLS\r\n";
  }

  bool StartTLSCommand::processResponse( const Response & r, TransactionState * ) {
    mNeedResponse = false;
    if ( r.code() != 220 ) {
      mSMTP->error( r.errorCode(),
                    i18n("Your SMTP server does not support TLS. "
                        "Disable TLS, if you want to connect "
                        "without encryption.") );
      return false;
    }

    if (startSsl()) {
      return true;
    } else {
      //kDebug(7112) << "TLS negotiation failed!";
      mSMTP->informationMessageBox(
                         i18n("Your SMTP server claims to "
                             "support TLS, but negotiation "
                             "was unsuccessful.\nYou can "
                             "disable TLS in the SMTP account settings dialog."),
                         i18n("Connection Failed") );
      return false;
    }
  }


#define SASLERROR mSMTP->error(KIO::ERR_COULD_NOT_AUTHENTICATE, \
  i18n("An error occurred during authentication: %1",  \
   QString::fromUtf8( sasl_errdetail( conn ) )));

  //
  // AUTH - rfc 2554
  //
  AuthCommand::AuthCommand( SMTPSessionInterface * smtp,
                            const char *mechanisms,
                            const QString &aFQDN,
                            KIO::AuthInfo &ai )
    : Command( smtp, CloseConnectionOnError|OnlyLastInPipeline ),
      mAi( &ai ),
      mFirstTime( true )
  {
    mMechusing = 0;
    int result;
    conn = 0;
    client_interact = 0;
    mOut = 0; mOutlen = 0;
    mOneStep = false;

    result = sasl_client_new( "smtp", aFQDN.toLatin1(),
      0, 0, callbacks, 0, &conn );
    if ( result != SASL_OK ) {
      SASLERROR
      return;
    }
    do {
      result = sasl_client_start(conn, mechanisms,
        &client_interact, &mOut, &mOutlen, &mMechusing);

      if (result == SASL_INTERACT)
        if ( !saslInteract( client_interact ) ) {
          return;
        };
    } while ( result == SASL_INTERACT );
    if ( result != SASL_CONTINUE && result != SASL_OK ) {
      SASLERROR
      return;
    }
    if ( result == SASL_OK ) mOneStep = true;
    kDebug(7112) << "Mechanism: " << mMechusing << " one step: " << mOneStep;
  }

  AuthCommand::~AuthCommand()
  {
    if ( conn ) {
      kDebug(7112) << "dispose sasl connection";
      sasl_dispose( &conn );
      conn = 0;
    }
  }

  bool AuthCommand::saslInteract( void *in )
  {
    kDebug(7112) << "saslInteract: ";
    sasl_interact_t *interact = ( sasl_interact_t * ) in;

    //some mechanisms do not require username && pass, so don't need a popup
    //window for getting this info
    for ( ; interact->id != SASL_CB_LIST_END; interact++ ) {
      if ( interact->id == SASL_CB_AUTHNAME ||
           interact->id == SASL_CB_PASS ) {

        if ( mAi->username.isEmpty() || mAi->password.isEmpty()) {
          if (!mSMTP->openPasswordDialog(*mAi)) {
            mSMTP->error(KIO::ERR_ABORTED, i18n("No authentication details supplied."));
            return false;
          }
        }
        break;
      }
    }

    interact = ( sasl_interact_t * ) in;
    while( interact->id != SASL_CB_LIST_END ) {
      switch( interact->id ) {
        case SASL_CB_USER:
        case SASL_CB_AUTHNAME:
          kDebug(7112) << "SASL_CB_[USER|AUTHNAME]: " << mAi->username;
          interact->result = strdup( mAi->username.toUtf8() );
          interact->len = strlen( (const char *) interact->result );
          break;
        case SASL_CB_PASS:
          kDebug(7112) << "SASL_CB_PASS: [HIDDEN]";
          interact->result = strdup( mAi->password.toUtf8() );
          interact->len = strlen( (const char *) interact->result );
          break;
        default:
          interact->result = NULL; interact->len = 0;
          break;
      }
      interact++;
    }
    return true;
  }

  bool AuthCommand::doNotExecute( const TransactionState * ) const {
    return !mMechusing;
  }

  void AuthCommand::ungetCommandLine( const QByteArray & s, TransactionState * ) {
    mUngetSASLResponse = s;
    mComplete = false;
  }

  QByteArray AuthCommand::nextCommandLine( TransactionState * ) {
    mNeedResponse = true;
    QByteArray cmd;

    QByteArray challenge;
    if ( !mUngetSASLResponse.isNull() ) {
      // implement un-ungetCommandLine
      cmd = mUngetSASLResponse;
      mUngetSASLResponse = 0;
    } else if ( mFirstTime ) {
      QString firstCommand = QLatin1String("AUTH ") + QString::fromLatin1( mMechusing );

      challenge = QByteArray::fromRawData( mOut, mOutlen ).toBase64();
      if ( !challenge.isEmpty() ) {
        firstCommand += QLatin1Char(' ');
        firstCommand += QString::fromLatin1( challenge.data(), challenge.size() );
      }
      cmd = firstCommand.toLatin1();

      if ( mOneStep ) mComplete = true;
    } else {
//      kDebug(7112) << "SS: '" << mLastChallenge << "'";
      challenge = QByteArray::fromBase64( mLastChallenge );
      int result;
      do {
        result = sasl_client_step(conn, challenge.isEmpty() ? 0 : challenge.data(),
                                  challenge.size(),
                                  &client_interact,
                                  &mOut, &mOutlen);
        if (result == SASL_INTERACT)
          if ( !saslInteract( client_interact ) ) {
            return "";
          };
      } while ( result == SASL_INTERACT );
      if ( result != SASL_CONTINUE && result != SASL_OK ) {
        kDebug(7112) << "sasl_client_step failed with: " << result;
        SASLERROR
        return "";
      }
      cmd = QByteArray::fromRawData( mOut, mOutlen ).toBase64();

//      kDebug(7112) << "CC: '" << cmd << "'";
      mComplete = ( result == SASL_OK );
    }
    cmd += "\r\n";
    return cmd;
  }

  bool AuthCommand::processResponse( const Response & r, TransactionState * ) {
    if ( !r.isOk() ) {
      if ( mFirstTime )
        if ( haveCapability( "AUTH" ) ) {
          QString chooseADifferentMsg( i18n("Choose a different authentication method.") );
          mSMTP->error( KIO::ERR_COULD_NOT_LOGIN,
              ( mMechusing ? i18n("Your SMTP server does not support %1.", QString::fromLatin1( mMechusing ) )
              : i18n("Your SMTP server does not support (unspecified method).") )
                        + QLatin1Char('\n') + chooseADifferentMsg + QLatin1Char('\n') + r.errorMessage() );
        }
        else
          mSMTP->error( KIO::ERR_COULD_NOT_LOGIN,
                        i18n( "Your SMTP server does not support authentication.\n"
                              "%1", r.errorMessage() ) );
      else
        mSMTP->error( KIO::ERR_COULD_NOT_LOGIN,
                      i18n( "Authentication failed.\n"
                            "Most likely the password is wrong.\n"
                            "%1", r.errorMessage() ) );
      return false;
    }
    mFirstTime = false;
    mLastChallenge = r.lines().front(); // ### better join all lines with \n?
    mNeedResponse = false;
    return true;
  }

  //
  // MAIL FROM:
  //

  QByteArray MailFromCommand::nextCommandLine( TransactionState * ) {
    mComplete = true;
    mNeedResponse = true;
    QByteArray cmdLine = "MAIL FROM:<" + mAddr + '>';
    if ( m8Bit && haveCapability("8BITMIME") )
      cmdLine += " BODY=8BITMIME";
    if ( mSize && haveCapability("SIZE") )
      cmdLine += " SIZE=" + QByteArray().setNum( mSize );
    return cmdLine + "\r\n";
  }

  bool MailFromCommand::processResponse( const Response & r, TransactionState * ts ) {
    assert( ts );
    mNeedResponse = false;

    if ( r.code() == 250 )
      return true;

    ts->setMailFromFailed( QString::fromLatin1(mAddr), r );
    return false;
  }

  //
  // RCPT TO:
  //

  QByteArray RcptToCommand::nextCommandLine( TransactionState * ) {
    mComplete = true;
    mNeedResponse = true;
    return "RCPT TO:<" + mAddr + ">\r\n";
  }

  bool RcptToCommand::processResponse( const Response & r, TransactionState * ts ) {
    assert( ts );
    mNeedResponse = false;

    if ( r.code() == 250 ) {
      ts->setRecipientAccepted();
      return true;
    }

    ts->addRejectedRecipient( QString::fromLatin1(mAddr), r.errorMessage() );
    return false;
  }

  //
  // DATA (only initial processing!)
  //

  QByteArray DataCommand::nextCommandLine( TransactionState * ts ) {
    assert( ts );
    mComplete = true;
    mNeedResponse = true;
    ts->setDataCommandIssued( true );
    return "DATA\r\n";
  }

  void DataCommand::ungetCommandLine( const QByteArray &, TransactionState * ts ) {
    assert( ts );
    mComplete = false;
    ts->setDataCommandIssued( false );
  }

  bool DataCommand::processResponse( const Response & r, TransactionState * ts ) {
    assert( ts );
    mNeedResponse = false;

    if ( r.code() == 354 ) {
      ts->setDataCommandSucceeded( true, r );
      return true;
    }

    ts->setDataCommandSucceeded( false, r );
    return false;
  }

  //
  // DATA (data transfer)
  //
  void TransferCommand::ungetCommandLine( const QByteArray & cmd, TransactionState * ) {
    if ( cmd.isEmpty() )
      return; // don't change state when we can't detect the unget in
              // the next nextCommandLine !!
    mWasComplete = mComplete;
    mComplete = false;
    mNeedResponse = false;
    mUngetBuffer = cmd;
  }

  bool TransferCommand::doNotExecute( const TransactionState * ts ) const {
    assert( ts );
    return ts->failed();
  }

  QByteArray TransferCommand::nextCommandLine( TransactionState * ts ) {
    assert( ts ); // let's rely on it ( at least for the moment )
    assert( !isComplete() );
    assert( !ts->failed() );

    static const QByteArray dotCRLF = ".\r\n";
    static const QByteArray CRLFdotCRLF = "\r\n.\r\n";

    if ( !mUngetBuffer.isEmpty() ) {
      const QByteArray ret = mUngetBuffer;
      mUngetBuffer = 0;
      if ( mWasComplete ) {
        mComplete = true;
        mNeedResponse = true;
      }
      return ret; // don't prepare(), it's slave-generated or already prepare()d
    }

    // normal processing:

    kDebug(7112) << "requesting data";
    mSMTP->dataReq();
    QByteArray ba;
    int result = mSMTP->readData( ba );
    kDebug(7112) << "got " << result << " bytes";
    if ( result > 0 )
      return prepare( ba );
    else if ( result < 0 ) {
      ts->setFailedFatally( KIO::ERR_INTERNAL,
                            i18n("Could not read data from application.") );
      mComplete = true;
      mNeedResponse = true;
      return 0;
    }
    mComplete = true;
    mNeedResponse = true;
    return mLastChar == '\n' ? dotCRLF : CRLFdotCRLF ;
  }

  bool TransferCommand::processResponse( const Response & r, TransactionState * ts ) {
    mNeedResponse = false;
    assert( ts );
    ts->setComplete();
    if ( !r.isOk() ) {
      ts->setFailed();
      mSMTP->error( r.errorCode(),
                    i18n( "The message content was not accepted.\n"
                          "%1", r.errorMessage() ) );
      return false;
    }
    return true;
  }

  static QByteArray dotstuff_lf2crlf( const QByteArray & ba, char & last ) {
    QByteArray result( ba.size() * 2 + 1, 0 ); // worst case: repeated "[.]\n"
    const char * s = ba.data();
    const char * const send = ba.data() + ba.size();
    char * d = result.data();

    while ( s < send ) {
      const char ch = *s++;
      if ( ch == '\n' && last != '\r' )
        *d++ = '\r'; // lf2crlf
      else if ( ch == '.' && last == '\n' )
        *d++ = '.'; // dotstuff
      last = *d++ = ch;
    }

    result.truncate( d - result.data() );
    return result;
  }

  QByteArray TransferCommand::prepare( const QByteArray & ba ) {
    if ( ba.isEmpty() )
      return 0;
    if ( mSMTP->lf2crlfAndDotStuffingRequested() ) {
      kDebug(7112) << "performing dotstuffing and LF->CRLF transformation";
      return dotstuff_lf2crlf( ba, mLastChar );
    } else {
      mLastChar = ba[ ba.size() - 1 ];
      return ba;
    }
  }

  //
  // NOOP
  //

  QByteArray NoopCommand::nextCommandLine( TransactionState * ) {
    mComplete = true;
    mNeedResponse = true;
    return "NOOP\r\n";
  }

  //
  // RSET
  //

  QByteArray RsetCommand::nextCommandLine( TransactionState * ) {
    mComplete = true;
    mNeedResponse = true;
    return "RSET\r\n";
  }

  //
  // QUIT
  //

  QByteArray QuitCommand::nextCommandLine( TransactionState * ) {
    mComplete = true;
    mNeedResponse = true;
    return "QUIT\r\n";
  }

} // namespace KioSMTP

