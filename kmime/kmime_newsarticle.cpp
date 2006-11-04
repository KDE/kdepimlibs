/*
    kmime_newsarticle.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kmime_newsarticle.h"

using namespace KMime;

namespace KMime {

void NewsArticle::parse()
{
  Message::parse();

  QByteArray raw;

  if ( !( raw = rawHeader( l_ines.type() ) ).isEmpty() )
    l_ines.from7BitString( raw );
}

QByteArray NewsArticle::assembleHeaders()
{
  Headers::Base *h;
  QByteArray newHead;

  //Control
  if ( ( h = control( false ) ) != 0 )
    newHead += h->as7BitString() + '\n';

  //Supersedes
  if ( ( h = supersedes( false ) ) != 0 )
    newHead += h->as7BitString() + '\n';

  //Newsgroups
  if ( ( h = newsgroups( false ) ) != 0 )
    newHead += h->as7BitString() + '\n';

  //Followup-To
  if ( ( h = followUpTo( false ) ) != 0 )
    newHead+=h->as7BitString() + '\n';

  //Mail-Copies-To
  if ( ( h = mailCopiesTo( false ) ) != 0 )
    newHead += h->as7BitString() + '\n';

  //Lines
  h = lines(); // "Lines" is mandatory
  newHead += h->as7BitString() + '\n';

  newHead += Message::assembleHeaders();
  return newHead;
}

void NewsArticle::clear()
{
  l_ines.clear();
  Message::clear();
}

Headers::Base * NewsArticle::getHeaderByType( const char *type )
{
  if ( strcasecmp( "Lines", type ) == 0 ) {
    if ( l_ines.isEmpty() ) {
      return 0;
    } else {
      return &l_ines;
    }
  } else {
    return Message::getHeaderByType( type );
  }
}

void NewsArticle::setHeader( Headers::Base *h )
{
  bool del = true;
  if ( h->is( "Lines" ) ) {
    l_ines.setNumberOfLines( (static_cast<Headers::Lines*>(h))->numberOfLines() );
  } else {
    del = false;
    Message::setHeader( h );
  }

  if ( del ) delete h;
}

bool NewsArticle::removeHeader( const char *type )
{
  if ( strcasecmp( "Lines", type ) == 0 ) {
    l_ines.clear();
  } else {
    return Message::removeHeader( type );
  }

  return true;
}

} // namespace KMime
