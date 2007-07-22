/*
    kmime_message.cpp

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

#include "kmime_message.h"
#include "kmime_message_p.h"

using namespace KMime;

namespace KMime {

Message::Message() : Content( new MessagePrivate( this ) ) {}

Message::Message(MessagePrivate * d) : Content( d ) {}

Message::~Message()
{}

void Message::parse()
{
  Q_D(Message);
  Content::parse();

  QByteArray raw;
  if ( !( raw = rawHeader( d->subject.type() ) ).isEmpty() )
    d->subject.from7BitString( raw );

  if ( !( raw = rawHeader( d->date.type() ) ).isEmpty() )
    d->date.from7BitString( raw );
}

QByteArray Message::assembleHeaders()
{
  Headers::Base *h;
  QByteArray newHead;

  //Message-ID
  if ( ( h = messageID( false ) ) != 0 )
    newHead += h->as7BitString() + '\n';

  //From
  h = from(); // "From" is mandatory
  newHead += h->as7BitString() + '\n';

  //Subject
  h = subject(); // "Subject" is mandatory
  newHead += h->as7BitString() + '\n';

  //To
  if ( ( h = to( false )) != 0 )
    newHead += h->as7BitString() + '\n';

  //Cc
  if ( ( h = cc( false )) != 0 )
    newHead += h->as7BitString() + '\n';

  //Reply-To
  if ( ( h = replyTo( false )) != 0 )
    newHead += h->as7BitString() + '\n';

  //Date
  h = date(); // "Date" is mandatory
  newHead += h->as7BitString() + '\n';

  //References
  if ( ( h = references( false )) != 0 )
    newHead += h->as7BitString() + '\n';

  //Organization
  if ( ( h = organization( false )) != 0 )
    newHead += h->as7BitString() + '\n';

  //UserAgent
  if ( ( h = userAgent( false )) != 0 )
    newHead += h->as7BitString() + '\n';

  // In-Reply-To
  if ( ( h = inReplyTo( false ) ) != 0 )
    newHead += h->as7BitString() + '\n';

  //Mime-Version
  newHead += "MIME-Version: 1.0\n";

  //X-Headers
  int pos = head().indexOf( "\nX-" );
  if ( pos > -1 ) { //we already have some x-headers => "recycle" them
    newHead += head().mid( pos + 1, head().length() - pos );
  } else {
    foreach ( Headers::Base *h, h_eaders ) {
      if ( h->isXHeader() ) {
        newHead += h->as7BitString() + '\n';
      }
    }
  }

  return newHead + Content::assembleHeaders();
}

void Message::clear()
{
  Q_D(Message);
  d->subject.clear();
  d->date.clear();
  Content::clear();
}

Headers::Base *Message::getHeaderByType( const char *type )
{
  Q_D(Message);
  if ( strcasecmp( "Subject", type ) == 0 ) {
    if ( d->subject.isEmpty() ) {
      return 0;
    } else {
      return &d->subject;
    }
  }
  else if ( strcasecmp("Date", type ) == 0 ){
    if ( d->date.isEmpty() ) {
      return 0;
    } else {
      return &d->date;
    }
  } else {
    return Content::getHeaderByType( type );
  }
}

void Message::setHeader( Headers::Base *h )
{
  Q_D(Message);
  bool del = true;
  if ( h->is( "Subject" ) ) {
    d->subject.fromUnicodeString( h->asUnicodeString(), h->rfc2047Charset() );
  } else if ( h->is( "Date" ) ) {
    d->date.setDateTime( (static_cast<Headers::Date*>( h))->dateTime() );
  } else {
    del = false;
    Content::setHeader( h );
  }

  if ( del ) delete h;
}

bool Message::removeHeader( const char *type )
{
  Q_D(Message);
  if ( strcasecmp( "Subject", type ) == 0 ) {
    d->subject.clear();
  } else if ( strcasecmp( "Date", type ) == 0 ) {
    d->date.clear();
  } else {
    return Content::removeHeader( type );
  }

  return true;
}

Headers::Subject* Message::subject(bool create)
{
  Q_D(Message);
  if ( !create && d->subject.isEmpty() )
    return 0;
  return &d->subject;
}

Headers::Date* Message::date( bool create )
{
  Q_D(Message);
  if ( !create && d->date.isEmpty() )
    return 0;
  return &d->date;
}

bool Message::isTopLevel() const
{
  return true;
}

Content* Message::mainBodyPart(const QByteArray & type)
{
  KMime::Content *c = this;
  while ( c ) {
    // not a multipart message
    if ( !c->contentType()->isMultipart() ) {
      if( c->contentType()->mimeType() == type || type.isEmpty() )
        return c;
      return 0;
    }

    // empty multipart
    if ( c->contents().count() == 0 )
      return 0;

    // multipart/alternative
    if ( c->contentType()->subType() == "alternative" ) {
      if ( type.isEmpty() )
        return c->contents().first();
      foreach ( Content* c, c->contents() ) {
        if ( c->contentType()->mimeType() == type )
          return c;
      }
      return 0;
    }

    c = c->contents().first();
  }

  return 0;
}

// @cond PRIVATE
#define kmime_mk_header_accessor( header, method ) \
Headers::header* Message::method( bool create ) { \
  Headers::header *p = 0; \
  return getHeaderInstance( p, create ); \
}

kmime_mk_header_accessor( MessageID, messageID )
kmime_mk_header_accessor( Organization, organization )
kmime_mk_header_accessor( From, from )
kmime_mk_header_accessor( ReplyTo, replyTo )
kmime_mk_header_accessor( To, to )
kmime_mk_header_accessor( Cc, cc )
kmime_mk_header_accessor( Bcc, bcc )
kmime_mk_header_accessor( References, references )
kmime_mk_header_accessor( UserAgent, userAgent )
kmime_mk_header_accessor( InReplyTo, inReplyTo )

#undef kmime_mk_header_accessor
// @endcond

}

