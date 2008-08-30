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
#include "kmime_message_p.h"
#include "kmime_util_p.h"

using namespace KMime;

namespace KMime {

class NewsArticlePrivate : public MessagePrivate
{
  public:
    NewsArticlePrivate( NewsArticle *q ) : MessagePrivate( q )
    {
      lines.setParent( q );
    }

    KMime::Headers::Lines lines;

    Q_DECLARE_PUBLIC(NewsArticle)
};

NewsArticle::NewsArticle() : Message( new NewsArticlePrivate( this ) ) {}

NewsArticle::~NewsArticle() {}

void NewsArticle::parse()
{
  Q_D(NewsArticle);
  Message::parse();

  QByteArray raw;

  if ( !( raw = rawHeader( d->lines.type() ) ).isEmpty() )
    d->lines.from7BitString( raw );
}

QByteArray NewsArticle::assembleHeaders()
{
  Q_D(NewsArticle);
  Headers::Base *h;
  QByteArray newHead;

  //Control
  if ( ( h = control( false ) ) != 0 && !h->isEmpty() ) {
    newHead += h->as7BitString() + '\n';
    KMime::removeHeader( d->head, h->type() );
  }

  //Supersedes
  if ( ( h = supersedes( false ) ) != 0 && !h->isEmpty() ) {
    newHead += h->as7BitString() + '\n';
    KMime::removeHeader( d->head, h->type() );
  }

  //Newsgroups
  if ( ( h = newsgroups( false ) ) != 0 && !h->isEmpty() ) {
    newHead += h->as7BitString() + '\n';
    KMime::removeHeader( d->head, h->type() );
  }

  //Followup-To
  if ( ( h = followUpTo( false ) ) != 0 && !h->isEmpty() ) {
    newHead+=h->as7BitString() + '\n';
    KMime::removeHeader( d->head, h->type() );
  }

  //Mail-Copies-To
  if ( ( h = mailCopiesTo( false ) ) != 0 && !h->isEmpty() ) {
    newHead += h->as7BitString() + '\n';
    KMime::removeHeader( d->head, h->type() );
  }

  //Lines
  h = lines(); // "Lines" is mandatory
  newHead += h->as7BitString() + '\n';
  KMime::removeHeader( d->head, h->type() );

  newHead += Message::assembleHeaders();
  return newHead;
}

void NewsArticle::clear()
{
  Q_D(NewsArticle);
  d->lines.clear();
  Message::clear();
}

Headers::Base * NewsArticle::getHeaderByType( const char *type )
{
    return headerByType( type );
}

Headers::Base * NewsArticle::headerByType( const char *type )
{
  Q_D(NewsArticle);
  if ( strcasecmp( "Lines", type ) == 0 ) {
    if ( d->lines.isEmpty() ) {
      return 0;
    } else {
      return &d->lines;
    }
  } else {
    return Message::headerByType( type );
  }
}

void NewsArticle::setHeader( Headers::Base *h )
{
  Q_D(NewsArticle);
  bool del = true;
  if ( h->is( "Lines" ) ) {
    d->lines.setNumberOfLines( (static_cast<Headers::Lines*>(h))->numberOfLines() );
  } else {
    del = false;
    Message::setHeader( h );
  }

  if ( del ) delete h;
}

bool NewsArticle::removeHeader( const char *type )
{
  Q_D(NewsArticle);
  if ( strcasecmp( "Lines", type ) == 0 ) {
    d->lines.clear();
  } else {
    return Message::removeHeader( type );
  }

  return true;
}

Headers::Lines* NewsArticle::lines(bool create)
{
  Q_D(NewsArticle);
  if ( !create && d->lines.isEmpty() )
    return 0;
  return &d->lines;
}

// @cond PRIVATE
#define kmime_mk_header_accessor( header, method ) \
Headers::header* NewsArticle::method( bool create ) { \
  Headers::header *p = 0; \
  return headerInstance( p, create ); \
}

kmime_mk_header_accessor( Control, control )
kmime_mk_header_accessor( Supersedes, supersedes )
kmime_mk_header_accessor( MailCopiesTo, mailCopiesTo )
kmime_mk_header_accessor( Newsgroups, newsgroups )
kmime_mk_header_accessor( FollowUpTo, followUpTo )

#undef kmime_mk_header_accessor
// @endcond

} // namespace KMime
