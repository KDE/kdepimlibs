/*  -*- c++ -*-
    capabilities.cc

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

#include "capabilities.h"
#include "response.h"

namespace KioSMTP {

  Capabilities Capabilities::fromResponse( const Response & ehlo ) {
    Capabilities c;

    // first, check whether the response was valid and indicates success:
    if ( !ehlo.isOk()
         || ehlo.code() / 10 != 25 // ### restrict to 250 only?
         || ehlo.lines().empty() )
      return c;

    QCStringList l = ehlo.lines();

    for ( QCStringList::const_iterator it = ++l.constBegin() ; it != l.constEnd() ; ++it )
      c.add( QString::fromLatin1(*it) );

    return c;
  }

  void Capabilities::add( const QString & cap, bool replace ) {
    QStringList tokens = cap.toUpper().split( QLatin1Char(' ') );
    if ( tokens.empty() )
      return;
    QString name = tokens.front(); tokens.pop_front();
    add( name, tokens, replace );
  }

  void Capabilities::add( const QString & name, const QStringList & args, bool replace ) {
    if ( replace )
      mCapabilities[name] = args;
    else
      mCapabilities[name] += args;
  }

  QString Capabilities::createSpecialResponse( bool tls ) const {
    QStringList result;
    if ( tls )
      result.push_back( QLatin1String("STARTTLS") );
    result += saslMethodsQSL();
    if ( have( "PIPELINING" ) )
      result.push_back( QLatin1String("PIPELINING") );
    if ( have( "8BITMIME" ) )
      result.push_back( QLatin1String("8BITMIME") );
    if ( have( "SIZE" ) ) {
      bool ok = false;
      unsigned int size = 0;
      if ( !mCapabilities[ QLatin1String("SIZE") ].isEmpty() )
        mCapabilities[ QLatin1String("SIZE") ].front().toUInt( &ok );
      if ( ok && !size )
        result.push_back( QLatin1String("SIZE=*") ); // any size
      else if ( ok )
        result.push_back( QString::fromLatin1("SIZE=%1").arg( size ) ); // fixed max
      else
        result.push_back( QLatin1String("SIZE") ); // indetermined
    }
    return result.join( QLatin1String(" ") );
  }

  QStringList Capabilities::saslMethodsQSL() const {
    QStringList result;
    for ( QMap<QString,QStringList>::const_iterator it = mCapabilities.begin();
          it != mCapabilities.end(); ++it ) {
      if ( it.key() == QLatin1String("AUTH") ) {
        result += it.value();
      } else if ( it.key().startsWith( QLatin1String( "AUTH=" ) ) ) {
        result.push_back( it.key().mid( qstrlen("AUTH=") ) );
        result += it.value();
      }
    }
    result.sort();
    for (int i = 0, j = 1; j < result.count(); i = j++ ) {
      if ( result.at(i) == result.at(j) ) {
        result.removeAt( j-- );
      }
    }
    return result;
  }



} // namespace KioSMTP

