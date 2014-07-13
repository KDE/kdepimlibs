/*
    kmime_message.cpp

    KMime, the KDE Internet mail/usenet news message library.
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
#include "kmime_util_p.h"


using namespace KMime;

namespace KMime {

Message::Message()
  : Content( new MessagePrivate( this ) )
{
}

Message::Message(MessagePrivate * d)
  : Content( d )
{
}

Message::~Message()
{
}

QByteArray Message::assembleHeaders()
{
  // Create the mandatory fields (RFC5322) if they do not exist already.
  date( true );
  from( true );

  // Make sure the mandatory MIME-Version field (RFC2045) is present and valid.
  Headers::MIMEVersion *mimeVersion = header<Headers::MIMEVersion>( true );
  mimeVersion->from7BitString( "1.0" );

  // Assemble all header fields.
  return Content::assembleHeaders();
}

bool Message::isTopLevel() const
{
  return Content::isTopLevel();
}

Content *Message::mainBodyPart( const QByteArray &type )
{
  KMime::Content *c = this;
  while ( c ) {
    // not a multipart message
    const KMime::Headers::ContentType * const contentType = c->contentType();
    if ( !contentType->isMultipart() ) {
      if ( contentType->mimeType() == type || type.isEmpty() ) {
        return c;
      }
      return 0;
    }

    // empty multipart
    if ( c->contents().count() == 0 ) {
      return 0;
    }

    // multipart/alternative
    if ( contentType->subType() == "alternative" ) {
      if ( type.isEmpty() ) {
        return c->contents().first();
      }
      foreach ( Content *c1, c->contents() ) {
        if ( c1->contentType()->mimeType() == type ) {
          return c1;
        }
      }
      return 0;
    }

    c = c->contents().first();
  }

  return 0;
}

QString Message::mimeType()
{
  static const QString &message_rfc822 = QLatin1String( QLatin1String( "message/rfc822" ) );
  return message_rfc822;
}


// @cond PRIVATE
#define kmime_mk_header_accessor( type, method ) \
Headers::type *Message::method( bool create ) { \
  return header<Headers::type>( create ); \
}

kmime_mk_header_accessor( MessageID, messageID )
kmime_mk_header_accessor( Subject, subject )
kmime_mk_header_accessor( Date, date )
kmime_mk_header_accessor( Organization, organization )
kmime_mk_header_accessor( From, from )
kmime_mk_header_accessor( ReplyTo, replyTo )
kmime_mk_header_accessor( To, to )
kmime_mk_header_accessor( Cc, cc )
kmime_mk_header_accessor( Bcc, bcc )
kmime_mk_header_accessor( References, references )
kmime_mk_header_accessor( UserAgent, userAgent )
kmime_mk_header_accessor( InReplyTo, inReplyTo )
kmime_mk_header_accessor( Sender, sender )

#undef kmime_mk_header_accessor
// @endcond

}

