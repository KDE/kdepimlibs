/*
    kmime_message.h

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
#ifndef __KMIME_MESSAGE_H__
#define __KMIME_MESSAGE_H__

#include "kmime_export.h"
#include "kmime_content.h"
#include "kmime_headers.h"
#include "boolflags.h"

namespace KMime {

class MessagePrivate;

/**
  Represents a (email) message.
*/
class KMIME_EXPORT Message : public Content
{
  public:
    typedef QList<KMime::Message*> List;

    /** Constructor. Creates an empty message. */
    Message();
    ~Message();

    //content handling
    virtual void parse();
    virtual void clear();

    //header access
    virtual KMime::Headers::Base *getHeaderByType( const char *type );
    virtual void setHeader( KMime::Headers::Base *h );
    virtual bool removeHeader( const char *type );

    /**
      Returns the message MessageID
    */
    virtual KMime::Headers::MessageID *messageID( bool create=true )
      {
        KMime::Headers::MessageID *p=0;
        return getHeaderInstance( p, create );
      }

    virtual KMime::Headers::Subject *subject( bool create=true );

    virtual KMime::Headers::Date *date( bool create=true );

    virtual KMime::Headers::From *from( bool create=true )
      {
        KMime::Headers::From *p=0;
        return getHeaderInstance( p, create );
      }

    virtual KMime::Headers::Organization *organization( bool create=true )
      {
        KMime::Headers::Organization *p=0;
        return getHeaderInstance( p, create );
      }

    virtual KMime::Headers::ReplyTo *replyTo( bool create=true )
      {
        KMime::Headers::ReplyTo *p=0;
        return getHeaderInstance( p, create );
      }

    virtual KMime::Headers::To *to( bool create=true )
      {
        KMime::Headers::To *p=0;
        return getHeaderInstance( p, create );
      }

    virtual KMime::Headers::Cc *cc( bool create=true )
      {
        KMime::Headers::Cc *p=0;
        return getHeaderInstance( p, create );
      }

    virtual KMime::Headers::Bcc *bcc( bool create=true )
      {
        KMime::Headers::Bcc *p=0;
        return getHeaderInstance( p, create );
      }

    virtual KMime::Headers::References *references( bool create=true )
      {
        KMime::Headers::References *p=0;
        return getHeaderInstance( p, create );
      }

    virtual KMime::Headers::UserAgent *userAgent( bool create=true )
      {
        KMime::Headers::UserAgent *p=0;
        return getHeaderInstance( p, create );
      }

    virtual KMime::Headers::InReplyTo *inReplyTo( bool create=true );

    virtual bool isTopLevel() const;

    /**
      Returns the first main body part of a given type, taking multipart/mixed
      and multipart/alternative nodes into consideration.
      Eg. \c bodyPart("text/html") will return a html content object if that is
      provided in a multipart/alternative node, but not if it's the non-first
      child node of a multipart/mixed node (ie. an attachment).
      @param type The mimetype of the body part, if not given, the first
      body part will be returned, regardless of it's type.
    */
    Content* mainBodyPart( const QByteArray &type = QByteArray() );

  protected:
    virtual QByteArray assembleHeaders();

    // @cond PRIVATE
    explicit Message( MessagePrivate *d );
    // @endcond

  private:
    Q_DECLARE_PRIVATE( Message )

}; // class Message

} // namespace KMime

#endif // __KMIME_MESSAGE_H__
