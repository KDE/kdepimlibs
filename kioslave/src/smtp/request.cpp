/*  -*- c++ -*-
    request.cc

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

#include "request.h"
#include "smtp_debug.h"

#include <kurl.h>
#include <QtCore/QUrl>
#include <kdebug.h>
#include <QDebug>

#include <assert.h>

namespace KioSMTP {

  Request Request::fromURL( const KUrl & url ) {
    Request request;

    const QStringList query = url.query().mid(1).split( QLatin1Char('&') );
#ifndef NDEBUG
    qCDebug(SMTP_LOG) << "Parsing request from query:\n" << query.join( QLatin1String("\n") );
#endif
    for ( QStringList::const_iterator it = query.begin() ; it != query.end() ; ++it ) {
      int equalsPos = (*it).indexOf( QLatin1Char('=') );
      if ( equalsPos <= 0 )
        continue;

      const QString key = (*it).left( equalsPos ).toLower();
      const QString value = QUrl::fromPercentEncoding( (*it).mid( equalsPos + 1 ).toLatin1() ); //krazy:exclude=qclasses

      if ( key == QLatin1String("to") )
        request.addTo( value );
      else if ( key == QLatin1String("cc") )
        request.addCc( value );
      else if ( key == QLatin1String("bcc") )
        request.addBcc( value );
      else if ( key == QLatin1String("headers") ) {
        request.setEmitHeaders( value == QLatin1String("0") );
        request.setEmitHeaders( false ); // ### ???
      }
      else if ( key == QLatin1String("subject") )
        request.setSubject( value );
      else if ( key == QLatin1String("from") )
        request.setFromAddress( value );
      else if ( key == QLatin1String("profile") )
        request.setProfileName( value );
      else if ( key == QLatin1String("hostname") )
        request.setHeloHostname( value );
      else if ( key == QLatin1String("body") )
        request.set8BitBody( value.toUpper() == QLatin1String("8BIT") );
      else if ( key == QLatin1String("size") )
        request.setSize( value.toUInt() );
      else
        qCWarning(SMTP_LOG) << "while parsing query: unknown query item \""
                       << key << "\" with value \"" << value << "\"" << endl;
    }

    return request;
  }

  QByteArray Request::heloHostnameCString() const {
    return QUrl::toAce( heloHostname() ); //krazy:exclude=qclasses
  }

  static bool isUsAscii( const QString & s ) {
    for ( int i = 0 ; i < s.length() ; ++i )
      if ( s[i].unicode() > 127 ) return false;
    return true;
  }



  static inline bool isSpecial( char ch ) {
    static const QByteArray specials = "()<>[]:;@\\,.\"";
    return specials.indexOf( ch ) >= 0;
  }



  static inline bool needsQuoting( char ch ) {
    return ch == '\\' || ch == '"' || ch == '\n' ;
  }



  static inline QByteArray rfc2047Encode( const QString & s ) {
    QByteArray r = s.trimmed().toUtf8().toBase64();
    return "=?utf-8?b?" + r + "?=" ; // use base64 since that always gives a valid encoded-word
  }



  static QByteArray quote( const QString & s ) {
    assert( isUsAscii( s ) );

    QByteArray r( s.length() * 2, 0 );
    bool needsQuotes = false;

    unsigned int j = 0;
    for ( int i = 0 ; i < s.length() ; ++i ) {
      char ch = s[i].toLatin1();
      if ( isSpecial( ch ) ) {
        if ( needsQuoting( ch ) )
          r[j++] = '\\';
        needsQuotes = true;
      }
      r[j++] = ch;
    }
    r.truncate( j );

    if ( needsQuotes )
      return '"' + r + '"';
    else
      return r;
  }



  static QByteArray formatFromAddress( const QString & fromRealName, const QString & fromAddress ) {
    if ( fromRealName.isEmpty() )
      return fromAddress.toLatin1(); // no real name: return "joe@user.org"

    // return "Joe User <joe@user.org>", "\"User, Joe\" <joe@user.org>"
    // or "=?utf-8?q?Joe_User?= <joe@user.org>", depending on real name's nature.
    QByteArray r = isUsAscii( fromRealName ) ? quote( fromRealName ) : rfc2047Encode( fromRealName );
    return r + " <" + fromAddress.toLatin1() + '>';
  }



  static QByteArray formatSubject( QString s ) {
    if ( isUsAscii( s ) )
      return s.remove( QLatin1Char('\n') ).toLatin1(); // don't break header folding,
                                          // so remove any line break
                                          // that happen to be around
    else
      return rfc2047Encode( s );
  }



  QByteArray Request::headerFields( const QString & fromRealName ) const {
    if ( !emitHeaders() )
      return 0;

    assert( hasFromAddress() ); // should have been checked for by
                                // caller (MAIL FROM comes before DATA)

    QByteArray result = "From: " + formatFromAddress( fromRealName, fromAddress() ) + "\r\n";

    if ( !subject().isEmpty() )
      result += "Subject: " + formatSubject( subject() ) + "\r\n";
    if ( !to().empty() )
      result += QByteArray( "To: " ) + to().join( QLatin1String(",\r\n\t") /* line folding */ ).toLatin1() + "\r\n";
    if ( !cc().empty() )
      result += QByteArray( "Cc: " ) + cc().join( QLatin1String(",\r\n\t") /* line folding */ ).toLatin1() + "\r\n";
    return result;
  }

} // namespace KioSMTP
