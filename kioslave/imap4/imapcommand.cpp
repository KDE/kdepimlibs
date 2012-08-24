/**********************************************************************
 *
 *   imapcommand.cc  - IMAP4rev1 command handler
 *   Copyright (C) 2000 Sven Carstens <s.carstens@gmx.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *   Send comments and bug fixes to s.carstens@gmx.de
 *
 *********************************************************************/

#include "imapcommand.h"
#include <kimap/rfccodecs.h>

/*#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>

#include <QRegExp>
#include <QBuffer>

#include <kprotocolmanager.h>
#include <ksock.h>
#include <kdebug.h>
#include <kcomponentdata.h>
#include <kio/connection.h>
#include <kio/slaveinterface.h>
#include <kio/passdlg.h>
#include <klocale.h> */

using namespace KIMAP;

imapCommand::imapCommand ()
{
  mComplete = false;
  mId.clear();
}

imapCommand::imapCommand (const QString & command, const QString & parameter)
//  aCommand(NULL),
//  mResult(NULL),
//  mParameter(NULL)
{
  mComplete = false;
  aCommand = command;
  aParameter = parameter;
  mId.clear();
}

bool
imapCommand::isComplete ()
{
  return mComplete;
}

const QString &
imapCommand::result ()
{
  return mResult;
}

const QString &
imapCommand::resultInfo ()
{
  return mResultInfo;
}

const QString &
imapCommand::id ()
{
  return mId;
}

const QString &
imapCommand::parameter ()
{
  return aParameter;
}

const QString &
imapCommand::command ()
{
  return aCommand;
}

void
imapCommand::setId (const QString & id)
{
  if ( mId.isEmpty() ) {
    mId = id;
  }
}

void
imapCommand::setComplete ()
{
  mComplete = true;
}

void
imapCommand::setResult (const QString & result)
{
  mResult = result;
}

void
imapCommand::setResultInfo (const QString & result)
{
  mResultInfo = result;
}

void
imapCommand::setCommand (const QString & command)
{
  aCommand = command;
}

void
imapCommand::setParameter (const QString & parameter)
{
  aParameter = parameter;
}

const QString
imapCommand::getStr ()
{
  if ( parameter().isEmpty() ) {
    return id() + ' ' + command() + "\r\n";
  } else {
    return id() + ' ' + command() + ' ' + parameter() + "\r\n";
  }
}

CommandPtr
imapCommand::clientNoop ()
{
  return CommandPtr( new imapCommand( "NOOP", "" ) );
}

CommandPtr
imapCommand::clientFetch (ulong uid, const QString & fields, bool nouid)
{
  return CommandPtr( clientFetch( uid, uid, fields, nouid ) );
}

CommandPtr
imapCommand::clientFetch (ulong fromUid, ulong toUid, const QString & fields,
                          bool nouid)
{
  QString uid = QString::number( fromUid );

  if ( fromUid != toUid ) {
    uid += ':';
    if ( toUid < fromUid ) {
      uid += '*';
    } else {
      uid += QString::number( toUid );
    }
  }
  return clientFetch( uid, fields, nouid );
}

CommandPtr
imapCommand::clientFetch (const QString & sequence, const QString & fields,
                          bool nouid)
{
  return CommandPtr( new imapCommand( nouid ? "FETCH" : "UID FETCH",
                                      sequence + " (" + fields + ')' ) );
}

CommandPtr
imapCommand::clientList (const QString & reference, const QString & path,
                         bool lsub)
{
  return CommandPtr( new imapCommand( lsub ? "LSUB" : "LIST",
                          QString( "\"" ) + KIMAP::encodeImapFolderName( reference ) +
                          "\" \"" + KIMAP::encodeImapFolderName( path ) + "\"" ) );
}

CommandPtr
imapCommand::clientSelect (const QString & path, bool examine)
{
  Q_UNUSED( examine );
  /** @note We use always SELECT, because UW-IMAP doesn't check for new mail, when
     used with the "mbox driver" and the folder is opened with EXAMINE
     and Courier can't append to a mailbox that is in EXAMINE state */
  return CommandPtr( new imapCommand( "SELECT",
                          QString( "\"" ) + KIMAP::encodeImapFolderName( path ) + "\"" ) );
}

CommandPtr
imapCommand::clientClose()
{
  return CommandPtr( new imapCommand( "CLOSE", "" ) );
}

CommandPtr
imapCommand::clientCopy (const QString & box, const QString & sequence,
                         bool nouid)
{
  return CommandPtr( new imapCommand( nouid ? "COPY" : "UID COPY",
                          sequence + " \"" + KIMAP::encodeImapFolderName( box ) + "\"" ) );
}

CommandPtr
imapCommand::clientAppend (const QString & box, const QString & flags,
                           ulong size)
{
  QString tmp;
  if ( !flags.isEmpty() ) {
    tmp = '(' + flags + ") ";
  }
  tmp += '{' + QString::number( size ) + '}';

  return CommandPtr( new imapCommand( "APPEND",
                          "\"" + KIMAP::encodeImapFolderName( box ) + "\" " + tmp ) );
}

CommandPtr
imapCommand::clientStatus (const QString & path, const QString & parameters)
{
  return CommandPtr( new imapCommand( "STATUS",
                          QString( "\"" ) + KIMAP::encodeImapFolderName( path ) +
                          "\" (" + parameters + ")" ) );
}

CommandPtr
imapCommand::clientCreate (const QString & path)
{
  return CommandPtr( new imapCommand( "CREATE",
                          QString( "\"" ) + KIMAP::encodeImapFolderName( path ) + "\"" ) );
}

CommandPtr
imapCommand::clientDelete (const QString & path)
{
  return CommandPtr( new imapCommand( "DELETE",
                          QString( "\"" ) + KIMAP::encodeImapFolderName( path ) + "\"" ) );
}

CommandPtr
imapCommand::clientSubscribe (const QString & path)
{
  return CommandPtr( new imapCommand( "SUBSCRIBE",
                          QString( "\"" ) + KIMAP::encodeImapFolderName( path ) + "\"" ) );
}

CommandPtr
imapCommand::clientUnsubscribe (const QString & path)
{
  return CommandPtr(  new imapCommand( "UNSUBSCRIBE",
                          QString( "\"" ) + KIMAP::encodeImapFolderName( path ) + "\"" ) );
}

CommandPtr
imapCommand::clientExpunge ()
{
  return CommandPtr( new imapCommand( "EXPUNGE", QString( "" ) ) );
}

CommandPtr
imapCommand::clientRename (const QString & src, const QString & dest)
{
  return CommandPtr( new imapCommand( "RENAME",
                          QString( "\"" ) + KIMAP::encodeImapFolderName( src ) +
                          "\" \"" + KIMAP::encodeImapFolderName( dest ) + "\"" ) );
}

CommandPtr
imapCommand::clientSearch (const QString & search, bool nouid)
{
  return CommandPtr( new imapCommand( nouid ? "SEARCH" : "UID SEARCH", search ) );
}

CommandPtr
imapCommand::clientStore (const QString & set, const QString & item,
                          const QString & data, bool nouid)
{
  return CommandPtr( new imapCommand( nouid ? "STORE" : "UID STORE",
                          set + ' ' + item + " (" + data + ')' ) );
}

CommandPtr
imapCommand::clientLogout ()
{
  return CommandPtr( new imapCommand( "LOGOUT", "" ) );
}

CommandPtr
imapCommand::clientStartTLS ()
{
  return CommandPtr( new imapCommand( "STARTTLS", "" ) );
}

CommandPtr
imapCommand::clientSetACL( const QString& box, const QString& user, const QString& acl )
{
  return CommandPtr( new imapCommand( "SETACL", QString( "\"" ) + KIMAP::encodeImapFolderName( box )
                          + "\" \"" + KIMAP::encodeImapFolderName( user )
                          + "\" \"" + KIMAP::encodeImapFolderName( acl ) + "\"" ) );
}

CommandPtr
imapCommand::clientDeleteACL( const QString& box, const QString& user )
{
  return CommandPtr( new imapCommand( "DELETEACL", QString( "\"" ) + KIMAP::encodeImapFolderName( box )
                          + "\" \"" + KIMAP::encodeImapFolderName( user )
                          + "\"" ) );
}

CommandPtr
imapCommand::clientGetACL( const QString& box )
{
  return CommandPtr( new imapCommand( "GETACL", QString( "\"" ) + KIMAP::encodeImapFolderName( box )
                          + "\"" ) );
}

CommandPtr
imapCommand::clientListRights( const QString& box, const QString& user )
{
  return CommandPtr( new imapCommand( "LISTRIGHTS", QString( "\"" ) + KIMAP::encodeImapFolderName( box )
                          + "\" \"" + KIMAP::encodeImapFolderName( user )
                          + "\"" ) );
}

CommandPtr
imapCommand::clientMyRights( const QString& box )
{
  return CommandPtr( new imapCommand( "MYRIGHTS", QString( "\"" ) + KIMAP::encodeImapFolderName( box )
                          + "\"" ) );
}

CommandPtr
imapCommand::clientSetAnnotation( const QString& box, const QString& entry, const QMap<QString, QString>& attributes )
{
  QString parameter = QString( "\"" ) + KIMAP::encodeImapFolderName( box )
                      + "\" \"" + KIMAP::encodeImapFolderName( entry )
                      + "\" (";
  for ( QMap<QString, QString>::ConstIterator it = attributes.begin(); it != attributes.end(); ++it ) {
    parameter += "\"";
    parameter += KIMAP::encodeImapFolderName( it.key() );
    parameter += "\" \"";
    parameter += KIMAP::encodeImapFolderName( it.value() );
    parameter += "\" ";
  }
  // Turn last space into a ')'
  parameter[parameter.length() - 1] = ')';

  return CommandPtr( new imapCommand( "SETANNOTATION", parameter ) );
}

CommandPtr
imapCommand::clientGetAnnotation( const QString& box, const QString& entry, const QStringList& attributeNames )
{
  QString parameter = QString( "\"" ) + KIMAP::encodeImapFolderName( box )
                          + "\" \"" + KIMAP::encodeImapFolderName( entry )
                          + "\" ";
  if ( attributeNames.count() == 1 ) {
    parameter += "\"" + KIMAP::encodeImapFolderName( attributeNames.first() ) + '"';
  } else {
    parameter += '(';
    for ( QStringList::ConstIterator it = attributeNames.begin(); it != attributeNames.end(); ++it ) {
      parameter += "\"" + KIMAP::encodeImapFolderName( *it ) + "\" ";
    }
    // Turn last space into a ')'
    parameter[parameter.length() - 1] = ')';
  }
  return CommandPtr( new imapCommand( "GETANNOTATION", parameter ) );
}

CommandPtr
imapCommand::clientNamespace()
{
  return CommandPtr( new imapCommand( "NAMESPACE", "" ) );
}

CommandPtr
imapCommand::clientGetQuotaroot( const QString& box )
{
  QString parameter = QString( "\"" ) + KIMAP::encodeImapFolderName( box ) + '"';
  return CommandPtr( new imapCommand( "GETQUOTAROOT", parameter ) );
}

CommandPtr
imapCommand::clientCustom( const QString& command, const QString& arguments )
{
  return CommandPtr( new imapCommand( command, arguments ) );
}
