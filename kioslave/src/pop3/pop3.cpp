/*
 * Copyright (c) 1999-2001 Alex Zepeda <zipzippy@sonic.net>
 * Copyright (c) 2001-2002 Michael Haeckel <haeckel@kde.org>
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
#include "pop3.h"
#include "../common.h"

extern "C" {
#include <sasl/sasl.h>
}

#include <QCoreApplication>
#include <QByteArray>
#include <QRegExp>

#include <kdebug.h>
#include <kcomponentdata.h>
#include <klocalizedstring.h>
#include <kcodecs.h>
#include <kmd5.h>

#include <kio/slaveinterface.h>

#define GREETING_BUF_LEN 1024
#define MAX_RESPONSE_LEN 512
#define MAX_COMMANDS 10

extern "C" {
  int Q_DECL_EXPORT kdemain(int argc, char **argv);
}

using namespace KIO;

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

int kdemain(int argc, char **argv)
{

  if (argc != 4) {
    kDebug(7105) << "Usage: kio_pop3 protocol domain-socket1 domain-socket2";
    return -1;
  }

  QCoreApplication app( argc, argv ); // needed for QSocketNotifier
  KComponentData componentData("kio_pop3");

  if (!initSASL())
    return -1;

  // Are we looking to use SSL?
  POP3Protocol *slave;
  if (strcasecmp(argv[1], "pop3s") == 0) {
    slave = new POP3Protocol(argv[2], argv[3], true);
  } else {
    slave = new POP3Protocol(argv[2], argv[3], false);
  }

  slave->dispatchLoop();
  delete slave;

  sasl_done();

  return 0;
}

POP3Protocol::POP3Protocol(const QByteArray & pool, const QByteArray & app,
                           bool isSSL)
:  TCPSlaveBase((isSSL ? "pop3s" : "pop3"), pool, app, isSSL)
{
  kDebug(7105);
  //m_cmd = CMD_NONE;
  m_iOldPort = 0;
  m_tTimeout.tv_sec = 10;
  m_tTimeout.tv_usec = 0;
  supports_apop = false;
  m_try_apop = true;
  m_try_sasl = true;
  opened = false;
  readBufferLen = 0;
}

POP3Protocol::~POP3Protocol()
{
  kDebug(7105);
  closeConnection();
}

void POP3Protocol::setHost(const QString & _host, quint16 _port,
                           const QString & _user, const QString & _pass)
{
  m_sServer = _host;
  m_iPort = _port;
  m_sUser = _user;
  m_sPass = _pass;
}

ssize_t POP3Protocol::myRead(void *data, ssize_t len)
{
  if (readBufferLen) {
    ssize_t copyLen = (len < readBufferLen) ? len : readBufferLen;
    memcpy(data, readBuffer, copyLen);
    readBufferLen -= copyLen;
    if (readBufferLen)
      memmove(readBuffer, &readBuffer[copyLen], readBufferLen);
    return copyLen;
  }
  waitForResponse(600);
  return read((char*)data, len);
}

ssize_t POP3Protocol::myReadLine(char *data, ssize_t len)
{
  ssize_t copyLen = 0, readLen = 0;
  while (true) {
    while (copyLen < readBufferLen && readBuffer[copyLen] != '\n')
      copyLen++;
    if (copyLen < readBufferLen || copyLen == len) {
      copyLen++;
      memcpy(data, readBuffer, copyLen);
      data[copyLen] = '\0';
      readBufferLen -= copyLen;
      if (readBufferLen)
        memmove(readBuffer, &readBuffer[copyLen], readBufferLen);
      return copyLen;
    }
    waitForResponse(600);
    readLen = read(&readBuffer[readBufferLen], len - readBufferLen);
    readBufferLen += readLen;
    if (readLen <= 0) {
      data[0] = '\0';
      return 0;
    }
  }
}

POP3Protocol::Resp POP3Protocol::getResponse(char *r_buf, unsigned int r_len)
{
  char *buf = 0;
  unsigned int recv_len = 0;
  // fd_set FDs;

  // Give the buffer the appropriate size
  r_len = r_len ? r_len : MAX_RESPONSE_LEN;

  buf = new char[r_len];

  // Clear out the buffer
  memset(buf, 0, r_len);
  myReadLine(buf, r_len - 1);
  //kDebug(7105) << "S:" << buf;

  // This is really a funky crash waiting to happen if something isn't
  // null terminated.
  recv_len = strlen(buf);

  /*
   *   From rfc1939:
   *
   *   Responses in the POP3 consist of a status indicator and a keyword
   *   possibly followed by additional information.  All responses are
   *   terminated by a CRLF pair.  Responses may be up to 512 characters
   *   long, including the terminating CRLF.  There are currently two status
   *   indicators: positive ("+OK") and negative ("-ERR").  Servers MUST
   *   send the "+OK" and "-ERR" in upper case.
   */

  if (strncmp(buf, "+OK", 3) == 0) {
    if (r_buf && r_len) {
      memcpy(r_buf, (buf[3] == ' ' ? buf + 4 : buf + 3),
             qMin(r_len, (buf[3] == ' ' ? recv_len - 4 : recv_len - 3)));
    }

    delete[]buf;

    return Ok;
  } else if (strncmp(buf, "-ERR", 4) == 0) {
    if (r_buf && r_len) {
      memcpy(r_buf, (buf[4] == ' ' ? buf + 5 : buf + 4),
             qMin(r_len, (buf[4] == ' ' ? recv_len - 5 : recv_len - 4)));
    }

    QString serverMsg = QString::fromLatin1(buf).mid(5).trimmed();

    m_sError = i18n("The server said: \"%1\"", serverMsg);

    delete[]buf;

    return Err;
  } else if (strncmp(buf, "+ ", 2) == 0) {
    if (r_buf && r_len) {
      memcpy(r_buf, buf + 2, qMin(r_len, recv_len - 4));
      r_buf[qMin(r_len - 1, recv_len - 4)] = '\0';
    }

    delete[]buf;

    return Cont;
  } else {
    kDebug(7105) << "Invalid POP3 response received!";

    if (r_buf && r_len) {
      memcpy(r_buf, buf, qMin(r_len, recv_len));
    }

    if (!*buf) {
      m_sError = i18n("The server terminated the connection.");
    } else {
      m_sError = i18n("Invalid response from server:\n\"%1\"", buf);
    }

    delete[]buf;

    return Invalid;
  }
}

bool POP3Protocol::sendCommand(const QByteArray &cmd)
{
  /*
   *   From rfc1939:
   *
   *   Commands in the POP3 consist of a case-insensitive keyword, possibly
   *   followed by one or more arguments.  All commands are terminated by a
   *   CRLF pair.  Keywords and arguments consist of printable ASCII
   *   characters.  Keywords and arguments are each separated by a single
   *   SPACE character.  Keywords are three or four characters long. Each
   *   argument may be up to 40 characters long.
   */

  if (!isConnected())
    return false;

  QByteArray cmdrn = cmd + "\r\n";

  // Show the command line the client sends, but make sure the password
  // doesn't show up in the debug output
  QByteArray debugCommand = cmd;
  if (!m_sPass.isEmpty())
    debugCommand.replace(m_sPass.toLatin1(),"<password>");
  //kDebug(7105) << "C:" << debugCommand;

  // Now actually write the command to the socket
  if (write(cmdrn.data(), cmdrn.size()) != static_cast < ssize_t >
      (cmdrn.size())) {
    m_sError = i18n("Could not send to server.\n");
    return false;
  }

  return true;
}

POP3Protocol::Resp POP3Protocol::command(const QByteArray &cmd, char *recv_buf,
                           unsigned int len)
{
  sendCommand(cmd);
  return getResponse(recv_buf, len);
}

void POP3Protocol::openConnection()
{
  m_try_apop = !hasMetaData("auth") || metaData("auth") == "APOP";
  m_try_sasl = !hasMetaData("auth") || metaData("auth") == "SASL";

  if (!pop3_open()) {
    kDebug(7105) << "pop3_open failed";
  } else {
    connected();
  }
}

void POP3Protocol::closeConnection()
{
  // If the file pointer exists, we can assume the socket is valid,
  // and to make sure that the server doesn't magically undo any of
  // our deletions and so-on, we should send a QUIT and wait for a
  // response.  We don't care if it's positive or negative.  Also
  // flush out any semblance of a persistant connection, i.e.: the
  // old username and password are now invalid.
  if (!opened) {
    return;
  }

  command("QUIT");
  disconnectFromHost();
  readBufferLen = 0;
  m_sOldUser = m_sOldPass = m_sOldServer = "";
  opened = false;
}

int POP3Protocol::loginAPOP( char *challenge, KIO::AuthInfo &ai )
{
  char buf[512];

  QString apop_string = QString::fromLatin1("APOP ");
  if (m_sUser.isEmpty() || m_sPass.isEmpty()) {
    // Prompt for usernames
    if (!openPasswordDialog(ai)) {
      error(ERR_ABORTED, i18n("No authentication details supplied."));
      closeConnection();
      return -1;
    } else {
      m_sUser = ai.username;
      m_sPass = ai.password;
    }
  }
  m_sOldUser = m_sUser;
  m_sOldPass = m_sPass;

  apop_string.append(m_sUser);

  memset(buf, 0, sizeof(buf));

  KMD5 ctx;

  kDebug(7105) << "APOP challenge: " << challenge;

  // Generate digest
  ctx.update(challenge, strlen(challenge));
  ctx.update(m_sPass.toLatin1() );

  // Genenerate APOP command
  apop_string.append(" ");
  apop_string.append(ctx.hexDigest());

  if (command(apop_string.toLocal8Bit(), buf, sizeof(buf)) == Ok) {
    return 0;
  }

  kDebug(7105) << "Could not login via APOP. Falling back to USER/PASS";
  closeConnection();
  if (metaData("auth") == "APOP") {
    error(ERR_COULD_NOT_LOGIN,
          i18n
          ("Login via APOP failed. The server %1 may not support APOP, although it claims to support it, or the password may be wrong.\n\n%2",
          m_sServer,
          m_sError));
    return -1;
  }
  return 1;
}

bool POP3Protocol::saslInteract( void *in, AuthInfo &ai )
{
  kDebug(7105);
  sasl_interact_t *interact = ( sasl_interact_t * ) in;

  //some mechanisms do not require username && pass, so don't need a popup
  //window for getting this info
  for ( ; interact->id != SASL_CB_LIST_END; interact++ ) {
    if ( interact->id == SASL_CB_AUTHNAME ||
         interact->id == SASL_CB_PASS ) {

      if (m_sUser.isEmpty() || m_sPass.isEmpty()) {
        if (!openPasswordDialog(ai)) {
          error(ERR_ABORTED, i18n("No authentication details supplied."));
          return false;
        }
        m_sUser = ai.username;
        m_sPass = ai.password;
      }
      break;
    }
  }

  interact = ( sasl_interact_t * ) in;
  while( interact->id != SASL_CB_LIST_END ) {
    kDebug(7105) << "SASL_INTERACT id: " << interact->id;
    switch( interact->id ) {
      case SASL_CB_USER:
      case SASL_CB_AUTHNAME:
        kDebug(7105) << "SASL_CB_[USER|AUTHNAME]: " << m_sUser;
        interact->result = strdup( m_sUser.toUtf8() );
        interact->len = strlen( (const char *) interact->result );
        break;
      case SASL_CB_PASS:
        kDebug(7105) << "SASL_CB_PASS: [hidden] ";
        interact->result = strdup( m_sPass.toUtf8() );
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

#define SASLERROR  closeConnection(); \
error(ERR_COULD_NOT_AUTHENTICATE, i18n("An error occurred during authentication: %1",  \
 QString::fromUtf8( sasl_errdetail( conn ) ))); \

int POP3Protocol::loginSASL( KIO::AuthInfo &ai )
{
  char buf[512];
  QString sasl_buffer = QString::fromLatin1("AUTH");

  int result;
  sasl_conn_t *conn = NULL;
  sasl_interact_t *client_interact = NULL;
  const char *out = NULL;
  uint outlen;
  const char *mechusing = NULL;
  Resp resp;

  result = sasl_client_new( "pop",
                       m_sServer.toLatin1(),
                       0, 0, callbacks, 0, &conn );

  if ( result != SASL_OK ) {
    kDebug(7105) << "sasl_client_new failed with: " << result;
    SASLERROR
    return false;
  }

  // We need to check what methods the server supports...
  // This is based on RFC 1734's wisdom
  if ( hasMetaData("sasl") || command(sasl_buffer.toLocal8Bit()) == Ok  ) {

    QStringList sasl_list;
    if (hasMetaData("sasl")) {
      sasl_list.append(metaData("sasl").toLatin1());
    } else
      while (true /* !AtEOF() */ ) {
        memset(buf, 0, sizeof(buf));
        myReadLine(buf, sizeof(buf) - 1);

        // HACK: This assumes fread stops at the first \n and not \r
        if ( (buf[0] == 0) || (strcmp(buf, ".\r\n") == 0) ) {
          break;              // End of data
        }
        // sanders, changed -2 to -1 below
        buf[strlen(buf) - 2] = '\0';

        sasl_list.append(buf);
      }

    do {
      result = sasl_client_start(conn, sasl_list.join(" ").toLatin1(),
        &client_interact, &out, &outlen, &mechusing);

      if (result == SASL_INTERACT)
        if ( !saslInteract( client_interact, ai ) ) {
          closeConnection();
          sasl_dispose( &conn );
          return -1;
        };
    } while ( result == SASL_INTERACT );
    if ( result != SASL_CONTINUE && result != SASL_OK ) {
      kDebug(7105) << "sasl_client_start failed with: " << result;
      SASLERROR
      sasl_dispose( &conn );
      return -1;
    }

    kDebug(7105) << "Preferred authentication method is " << mechusing << ".";

    QByteArray msg,tmp;

    QString firstCommand = "AUTH " + QString::fromLatin1( mechusing );
    msg = QByteArray::fromRawData( out, outlen ).toBase64();
    if ( !msg.isEmpty() ) {
      firstCommand += ' ';
      firstCommand += QString::fromLatin1( msg.data(), msg.size() );
    }

    tmp.resize( 2049 );
    resp = command( firstCommand.toLatin1(), tmp.data(), 2049 );
    while( resp == Cont ) {
      tmp.resize(tmp.indexOf((char)0));
      msg = QByteArray::fromBase64( tmp );
      do {
        result = sasl_client_step(conn, msg.isEmpty() ? 0 : msg.data(),
                                  msg.size(),
                                  &client_interact,
                                  &out, &outlen);

        if (result == SASL_INTERACT)
          if ( !saslInteract( client_interact, ai ) ) {
            closeConnection();
            sasl_dispose( &conn );
            return -1;
          };
      } while ( result == SASL_INTERACT );
      if ( result != SASL_CONTINUE && result != SASL_OK ) {
        kDebug(7105) << "sasl_client_step failed with: " << result;
        SASLERROR
        sasl_dispose( &conn );
        return -1;
      }

      msg = QByteArray::fromRawData( out, outlen ).toBase64();
      tmp.resize(2049);
      resp = command( msg, tmp.data(), 2049 );
    }

    sasl_dispose( &conn );
    if ( resp == Ok ) {
      kDebug(7105) << "SASL authenticated";
      m_sOldUser = m_sUser;
      m_sOldPass = m_sPass;
      return 0;
    }

    if (metaData("auth") == "SASL") {
      closeConnection();
      error(ERR_COULD_NOT_LOGIN,
            i18n
            ("Login via SASL (%1) failed. The server may not support %2, or the password may be wrong.\n\n%3",
            mechusing, mechusing, m_sError));
      return -1;
    }
  }

  if (metaData("auth") == "SASL") {
    closeConnection();
    error(ERR_COULD_NOT_LOGIN,
          i18n("Your POP3 server (%1) does not support SASL.\n"
               "Choose a different authentication method.", m_sServer));
    return -1;
  }
  return 1;
}

bool POP3Protocol::loginPASS( KIO::AuthInfo &ai )
{
  char buf[512];

  if (m_sUser.isEmpty() || m_sPass.isEmpty()) {
    // Prompt for usernames
    if (!openPasswordDialog(ai)) {
      error(ERR_ABORTED, i18n("No authentication details supplied."));
      closeConnection();
      return false;
    } else {
      m_sUser = ai.username;
      m_sPass = ai.password;
    }
  }
  m_sOldUser = m_sUser;
  m_sOldPass = m_sPass;

  QString one_string = QString::fromLatin1("USER ");
  one_string.append( m_sUser );

  if ( command(one_string.toLocal8Bit(), buf, sizeof(buf)) != Ok ) {
    kDebug(7105) << "Could not login. Bad username Sorry";

    m_sError =
        i18n("Could not login to %1.\n\n", m_sServer) + m_sError;
    error(ERR_COULD_NOT_LOGIN, m_sError);
    closeConnection();

    return false;
  }

  one_string = QString::fromLatin1("PASS ");
  one_string.append(m_sPass);

  if ( command(one_string.toLocal8Bit(), buf, sizeof(buf)) != Ok ) {
    kDebug(7105) << "Could not login. Bad password Sorry.";
    m_sError =
        i18n
        ("Could not login to %1. The password may be wrong.\n\n%2",
        m_sServer, m_sError);
    error(ERR_COULD_NOT_LOGIN, m_sError);
    closeConnection();
    return false;
  }
  kDebug(7105) << "USER/PASS login succeeded";
  return true;
}

bool POP3Protocol::pop3_open()
{
  kDebug(7105);
  char  *greeting_buf;
  if ((m_iOldPort == m_iPort) && (m_sOldServer == m_sServer) &&
      (m_sOldUser == m_sUser) && (m_sOldPass == m_sPass)) {
    kDebug(7105) << "Reusing old connection";
    return true;
  }
  do {
    closeConnection();

    if (!connectToHost((isAutoSsl() ? "pop3s" : "pop3"), m_sServer.toLatin1(), m_iPort)) {
      // error(ERR_COULD_NOT_CONNECT, m_sServer);
      // ConnectToHost has already send an error message.
      return false;
    }
    opened = true;

    greeting_buf = new char[GREETING_BUF_LEN];
    memset(greeting_buf, 0, GREETING_BUF_LEN);

    // If the server doesn't respond with a greeting
    if (getResponse(greeting_buf, GREETING_BUF_LEN) != Ok) {
      m_sError =
          i18n("Could not login to %1.\n\n", m_sServer) +
          ((!greeting_buf
            || !*greeting_buf) ?
           i18n("The server terminated the connection immediately.") :
           i18n("Server does not respond properly:\n%1\n",
           greeting_buf));
      error(ERR_COULD_NOT_LOGIN, m_sError);
      delete[]greeting_buf;
      closeConnection();
      return false;             // we've got major problems, and possibly the
      // wrong port
    }
    QString greeting(greeting_buf);
    delete[]greeting_buf;

    if (greeting.length() > 0) {
      greeting.truncate(greeting.length() - 2);
    }

    // Does the server support APOP?
    QString apop_cmd;
    QRegExp re("<[A-Za-z0-9\\.\\-_]+@[A-Za-z0-9\\.\\-_]+>$", Qt::CaseInsensitive);

    kDebug(7105) << "greeting: " << greeting;
    int apop_pos = greeting.indexOf(re);
    supports_apop = (bool) (apop_pos != -1);

    if (metaData("nologin") == "on")
      return true;

    if (metaData("auth") == "APOP" && !supports_apop) {
      error(ERR_COULD_NOT_LOGIN,
          i18n("Your POP3 server (%1) does not support APOP.\n"
               "Choose a different authentication method.", m_sServer));
      closeConnection();
      return false;
    }

    m_iOldPort = m_iPort;
    m_sOldServer = m_sServer;

    // Try to go into TLS mode
    if ((metaData("tls") == "on" /*### || (canUseTLS() &&
                                     metaData("tls") != "off")*/)
        && command("STLS") == Ok ) {
      if (startSsl()) {
        kDebug(7105) << "TLS mode has been enabled.";
      } else {
        kDebug(7105) << "TLS mode setup has failed. Aborting." << endl;
        error(ERR_SLAVE_DEFINED,
              i18n("Your POP3 server claims to "
                   "support TLS but negotiation "
                   "was unsuccessful.\nYou can "
                   "disable TLS in the POP account settings dialog."));
        closeConnection();
        return false;
      }
    } else if (metaData("tls") == "on") {
      error(ERR_SLAVE_DEFINED,
            i18n("Your POP3 server (%1) does not support TLS. Disable "
                 "TLS, if you want to connect without encryption.", m_sServer));
      closeConnection();
      return false;
    }

    KIO::AuthInfo authInfo;
    authInfo.username = m_sUser;
    authInfo.password = m_sPass;
    authInfo.prompt = i18n("Username and password for your POP3 account:");

    if ( supports_apop && m_try_apop ) {
      kDebug(7105) << "Trying APOP";
      int retval = loginAPOP( greeting.toLatin1().data() + apop_pos, authInfo );
      switch ( retval ) {
        case 0: return true;
        case -1: return false;
        default:
          m_try_apop = false;
      }
    } else if ( m_try_sasl ) {
      kDebug(7105) << "Trying SASL";
      int retval = loginSASL( authInfo );
      switch ( retval ) {
        case 0: return true;
        case -1: return false;
        default:
          m_try_sasl = false;
      }
    } else {
      // Fall back to conventional USER/PASS scheme
      kDebug(7105) << "Trying USER/PASS";
      return loginPASS( authInfo );
    }
  } while ( true );
}

size_t POP3Protocol::realGetSize(unsigned int msg_num)
{
  char *buf;
  QByteArray cmd;
  size_t ret = 0;

  buf = new char[MAX_RESPONSE_LEN];
  memset(buf, 0, MAX_RESPONSE_LEN);
  cmd = "LIST " + QByteArray::number( msg_num );
  if ( command(cmd, buf, MAX_RESPONSE_LEN) != Ok ) {
    delete[]buf;
    return 0;
  } else {
    cmd = buf;
    cmd.remove(0, cmd.indexOf(" "));
    ret = cmd.toLong();
  }
  delete[]buf;
  return ret;
}

void POP3Protocol::get(const KUrl & url)
{
// List of supported commands
//
// URI                                 Command   Result
// pop3://user:pass@domain/index       LIST      List message sizes
// pop3://user:pass@domain/uidl        UIDL      List message UIDs
// pop3://user:pass@domain/remove/#1   DELE #1   Mark a message for deletion
// pop3://user:pass@domain/download/#1 RETR #1   Get message header and body
// pop3://user:pass@domain/list/#1     LIST #1   Get size of a message
// pop3://user:pass@domain/uid/#1      UIDL #1   Get UID of a message
// pop3://user:pass@domain/commit      QUIT      Delete marked messages
// pop3://user:pass@domain/headers/#1  TOP #1    Get header of message
//
// Notes:
// Sizes are in bytes.
// No support for the STAT command has been implemented.
// commit closes the connection to the server after issuing the QUIT command.

  bool ok = true;
  char buf[MAX_PACKET_LEN];
  char destbuf[MAX_PACKET_LEN];
  QString cmd, path = url.path();
  int maxCommands = (metaData("pipelining") == "on") ? MAX_COMMANDS : 1;

  if (path.at(0) == '/')
    path.remove(0, 1);
  if (path.isEmpty()) {
    kDebug(7105) << "We should be a dir!!";
    error(ERR_IS_DIRECTORY, url.url());
    //m_cmd = CMD_NONE;
    return;
  }

  if (((path.indexOf('/') == -1) && (path != "index") && (path != "uidl")
       && (path != "commit"))) {
    error(ERR_MALFORMED_URL, url.url());
    //m_cmd = CMD_NONE;
    return;
  }

  cmd = path.left(path.indexOf('/'));
  path.remove(0, path.indexOf('/') + 1);

  if (!pop3_open()) {
    kDebug(7105) << "pop3_open failed";
    error(ERR_COULD_NOT_CONNECT, m_sServer);
    return;
  }

  if ((cmd == "index") || (cmd == "uidl")) {
    unsigned long size = 0;
    bool result;

    if (cmd == "index") {
      result = ( command("LIST") == Ok );
    } else {
      result = ( command("UIDL") == Ok );
    }

    /*
       LIST
       +OK Mailbox scan listing follows
       1 2979
       2 1348
       .
     */
    if (result) {
      while (true /* !AtEOF() */ ) {
        memset(buf, 0, sizeof(buf));
        myReadLine(buf, sizeof(buf) - 1);

        // HACK: This assumes fread stops at the first \n and not \r
        if ( (buf[0] == 0) || (strcmp(buf, ".\r\n") == 0) ) {
          break;                // End of data
        }
        // sanders, changed -2 to -1 below
        int bufStrLen = strlen(buf);
        buf[bufStrLen - 2] = '\0';
        size += bufStrLen;
        data(QByteArray::fromRawData(buf, bufStrLen));
        totalSize(size);
      }
    }
    kDebug(7105) << "Finishing up list";
    data(QByteArray());
    finished();
  } else if (cmd == "remove") {
    const QStringList waitingCommands = path.split(',');
    int activeCommands = 0;
    QStringList::ConstIterator it = waitingCommands.begin();
    while (it != waitingCommands.end() || activeCommands > 0) {
      while (activeCommands < maxCommands && it != waitingCommands.end()) {
        sendCommand(("DELE " + *it).toLatin1());
        activeCommands++;
        it++;
      }
      getResponse(buf, sizeof(buf) - 1);
      activeCommands--;
    }
    finished();
    //m_cmd = CMD_NONE;
  } else if (cmd == "download" || cmd == "headers") {
    const QStringList waitingCommands = path.split(',', QString::SkipEmptyParts);
    bool noProgress = (metaData("progress") == "off"
                       || waitingCommands.count() > 1);
    int p_size = 0;
    unsigned int msg_len = 0;
    QString list_cmd("LIST ");
    list_cmd += path;
    memset(buf, 0, sizeof(buf));
    if ( !noProgress ) {
      if ( command(list_cmd.toLatin1(), buf, sizeof(buf) - 1) == Ok ) {
        list_cmd = buf;
        // We need a space, otherwise we got an invalid reply
        if (!list_cmd.indexOf(" ")) {
          kDebug(7105) << "List command needs a space? " << list_cmd;
          closeConnection();
          error(ERR_INTERNAL, i18n("Unexpected response from POP3 server."));
          return;
        }
        list_cmd.remove(0, list_cmd.indexOf(" ") + 1);
        msg_len = list_cmd.toUInt(&ok);
        if (!ok) {
          kDebug(7105) << "LIST command needs to return a number? :" <<
              list_cmd << ":";
          closeConnection();
          error(ERR_INTERNAL, i18n("Unexpected response from POP3 server."));
          return;
        }
      } else {
        closeConnection();
        error( ERR_SLAVE_DEFINED, i18n( "Error during communication with the POP3 server while "
                                        "trying to list mail: %1", m_sError ) );
        return;
      }
    }

    int activeCommands = 0;
    QStringList::ConstIterator it = waitingCommands.begin();
    bool firstCommand = true;
    while (it != waitingCommands.end() || activeCommands > 0) {
      while (activeCommands < maxCommands && it != waitingCommands.end()) {
        sendCommand(QString((cmd ==
                      "headers") ? QString("TOP " + *it + " 0") : QString("RETR " +
                     *it)).toLatin1());
        activeCommands++;
        it++;
      }
      if ( getResponse(buf, sizeof(buf) - 1) == Ok ) {
        activeCommands--;
        if ( firstCommand ) {
          firstCommand = false;
          mimeType("message/rfc822");
        }
        totalSize(msg_len);
        memset(buf, 0, sizeof(buf));
        char ending = '\n';
        bool endOfMail = false;
        bool eat = false;
        while (true /* !AtEOF() */ ) {
          ssize_t readlen = myRead(buf, sizeof(buf) - 1);
          if (readlen <= 0) {
            if (isConnected())
              error(ERR_SERVER_TIMEOUT, m_sServer);
            else
              error(ERR_CONNECTION_BROKEN, m_sServer);
            closeConnection();
            return;
          }
          if (ending == '.' && readlen > 1 && buf[0] == '\r'
              && buf[1] == '\n') {
            readBufferLen = readlen - 2;
            memcpy(readBuffer, &buf[2], readBufferLen);
            break;
          }
          bool newline = (ending == '\n');

          if (buf[readlen - 1] == '\n')
            ending = '\n';
          else if (buf[readlen - 1] == '.'
                   && ((readlen > 1) ? buf[readlen - 2] == '\n' : ending ==
                       '\n'))
            ending = '.';
          else
            ending = ' ';

          char *buf1 = buf, *buf2 = destbuf;
          // ".." at start of a line means only "."
          // "." means end of data
          for (ssize_t i = 0; i < readlen; i++) {
            if (*buf1 == '\r' && eat) {
              endOfMail = true;
              if (i == readlen - 1 /* && !AtEOF() */ )
                myRead(buf, 1);
              else if (i < readlen - 2) {
                readBufferLen = readlen - i - 2;
                memcpy(readBuffer, &buf[i + 2], readBufferLen);
              }
              break;
            } else if (*buf1 == '\n') {
              newline = true;
              eat = false;
            } else if (*buf1 == '.' && newline) {
              newline = false;
              eat = true;
            } else {
              newline = false;
              eat = false;
            }
            if (!eat) {
              *buf2 = *buf1;
              buf2++;
            }
            buf1++;
          }

          if (buf2 > destbuf) {
            data(QByteArray::fromRawData(destbuf, buf2-destbuf));
          }

          if (endOfMail)
            break;

          if (!noProgress) {
            p_size += readlen;
            processedSize(p_size);
          }
        }
        infoMessage("message complete");
      } else {
        kDebug(7105) << "Could not login. Bad RETR Sorry";
        closeConnection();
        error( ERR_SLAVE_DEFINED, i18n( "Error during communication with the POP3 server while "
                                        "trying to download mail: %1", m_sError ) );
        return;
      }
    }
    kDebug(7105) << "Finishing up";
    data(QByteArray());
    finished();
  } else if ((cmd == "uid") || (cmd == "list")) {
    QString qbuf;
    (void) path.toInt(&ok);

    if (!ok) {
      return;                   //  We fscking need a number!
    }

    if (cmd == "uid") {
      path.prepend("UIDL ");
    } else {
      path.prepend("LIST ");
    }

    memset(buf, 0, sizeof(buf));
    if ( command(path.toLatin1(), buf, sizeof(buf) - 1) == Ok ) {
      const int len = strlen(buf);
      mimeType("text/plain");
      totalSize(len);
      data(QByteArray::fromRawData(buf, len));
      processedSize(len);
      kDebug(7105) << buf;
      kDebug(7105) << "Finishing up uid";
      data(QByteArray());
      finished();
    } else {
      closeConnection();
      error(ERR_INTERNAL, i18n("Unexpected response from POP3 server."));
      return;
    }
  } else if (cmd == "commit") {
    kDebug(7105) << "Issued QUIT";
    closeConnection();
    finished();
    //m_cmd = CMD_NONE;
    return;
  }
}

void POP3Protocol::listDir(const KUrl &)
{
  bool isINT;
  int num_messages = 0;
  QByteArray q_buf(MAX_RESPONSE_LEN, 0);

  // Try and open a connection
  if (!pop3_open()) {
    kDebug(7105) << "pop3_open failed";
    error(ERR_COULD_NOT_CONNECT, m_sServer);
    return;
  }
  // Check how many messages we have. STAT is by law required to
  // at least return +OK num_messages total_size
  if ( command("STAT", q_buf.data(), MAX_RESPONSE_LEN) != Ok ) {
    error(ERR_INTERNAL, i18n("The POP3 command 'STAT' failed"));
    return;
  }
  kDebug(7105) << "The stat buf is :" << q_buf << ":";
  if (q_buf.indexOf(" ") == -1) {
    error(ERR_INTERNAL,
          i18n("Invalid POP3 response, should have at least one space."));
    closeConnection();
    return;
  }
  q_buf.remove(q_buf.indexOf(" "), q_buf.length());

  num_messages = q_buf.toUInt(&isINT);
  if (!isINT) {
    error(ERR_INTERNAL, i18n("Invalid POP3 STAT response."));
    closeConnection();
    return;
  }
  UDSEntry entry;
  QString fname;
  for (int i = 0; i < num_messages; i++) {
    fname = "Message %1";

    entry.insert(KIO::UDSEntry::UDS_NAME, fname.arg(i + 1));
    entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("text/plain"));

    KUrl uds_url;
    if (isAutoSsl()) {
      uds_url.setProtocol("pop3s");
    } else {
      uds_url.setProtocol("pop3");
    }

    uds_url.setUser(m_sUser);
    uds_url.setPass(m_sPass);
    uds_url.setHost(m_sServer);
    uds_url.setPath(QString::fromLatin1("/download/%1").arg(i + 1));
    entry.insert(KIO::UDSEntry::UDS_URL, uds_url.url());

    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
    entry.insert(KIO::UDSEntry::UDS_SIZE, realGetSize(i + 1));
    entry.insert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IXUSR | S_IWUSR);

    listEntry(entry, false);
    entry.clear();
  }
  listEntry(entry, true);       // ready

  finished();
}

void POP3Protocol::stat(const KUrl & url)
{
  QString _path = url.path();

  if (_path.at(0) == '/')
    _path.remove(0, 1);

  UDSEntry entry;
  entry.insert(KIO::UDSEntry::UDS_NAME, _path);
  entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
  entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("message/rfc822"));

  // TODO: maybe get the size of the message?
  statEntry(entry);

  finished();
}

void POP3Protocol::del(const KUrl & url, bool /*isfile */ )
{
  QString invalidURI;
  bool isInt;

  if (!pop3_open()) {
    kDebug(7105) << "pop3_open failed";
    error(ERR_COULD_NOT_CONNECT, m_sServer);
    return;
  }

  QString _path = url.path();
  if (_path.at(0) == '/') {
    _path.remove(0, 1);
  }

  _path.toUInt(&isInt);
  if (!isInt) {
    invalidURI = _path;
  } else {
    _path.prepend("DELE ");
    if ( command(_path.toLatin1()) != Ok ) {
      invalidURI = _path;
    }
  }

  kDebug(7105) << "Path:" << _path;
  finished();
}
