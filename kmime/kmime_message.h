/*
    kmime_message.h

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
#ifndef __KMIME_MESSAGE_H__
#define __KMIME_MESSAGE_H__

#include "kmime_export.h"
#include "kmime_content.h"
#include "kmime_headers.h"
#include "boolflags.h"

namespace boost {
  template <typename T> class shared_ptr;
}

namespace KMime {

class MessagePrivate;

/**
 * Represents a (email) message.
 *
 * Sample how to create a multipart message:
 * \code
 * // Set the multipart message.
 * Message *m = new Message;
 * Headers::ContentType *ct = m->contentType();
 * ct->setMimeType( "multipart/mixed" );
 * ct->setBoundary( multiPartBoundary() );
 * ct->setCategory( Headers::CCcontainer );
 * m->contentTransferEncoding()->clear();
 *
 * // Set the headers.
 * m->from()->fromUnicodeString( "some@mailaddy.com", "utf-8" );
 * m->to()->fromUnicodeString( "someother@mailaddy.com", "utf-8" );
 * m->cc()->fromUnicodeString( "some@mailaddy.com", "utf-8" );
 * m->date()->setDateTime( KDateTime::currentLocalDateTime() );
 * m->subject()->fromUnicodeString( "My Subject", "utf-8" );
 *
 * // Set the first multipart, the body message.
 * KMime::Content *b = new KMime::Content;
 * b->contentType()->setMimeType( "text/plain" );
 * b->setBody( "Some text..." );
 *
 * // Set the second multipart, the attachment.
 * KMime::Content *a = new KMime::Content;
 * KMime::Headers::ContentDisposition *d = new KMime::Headers::ContentDisposition( attachMessage );
 * d->setFilename( "cal.ics" );
 * d->setDisposition( KMime::Headers::CDattachment );
 * a->contentType()->setMimeType( "text/plain" );
 * a->setHeader( d );
 * a->setBody( "Some text in the attachment..." );
 *
 * // Attach the both multiparts and assemble the message.
 * m->addContent( b );
 * m->addContent( a );
 * m->assemble();
 * \endcode
 */
class KMIME_EXPORT Message : public Content
{
  public:
    /**
      A list of messages.
    */
    typedef QList<KMime::Message*> List;

    /**
      A shared pointer to a message object.
    */
    typedef boost::shared_ptr<Message> Ptr;

    /**
      Creates an empty Message.
    */
    Message();

    /**
      Destroys this Message.
    */
    ~Message();

    /* reimpl */
    virtual void parse();

    /* reimpl */
    virtual void clear();

    /* reimpl */
    virtual KDE_DEPRECATED KMime::Headers::Base *getHeaderByType( const char *type );

    /* reimpl */
    virtual KMime::Headers::Base *headerByType( const char *type );

    /* reimpl */
    virtual void setHeader( KMime::Headers::Base *h );

    /* reimpl */
    virtual bool removeHeader( const char *type );

    // KDE5: Why are these virtual?
    /**
      Returns the Message-ID header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::MessageID *messageID( bool create = true );

    /**
      Returns the Subject header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::Subject *subject( bool create = true );

    /**
      Returns the Date header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::Date *date( bool create = true );

    /**
      Returns the From header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::From *from( bool create = true );

    /**
      Returns the Organization header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::Organization *organization( bool create = true );

    /**
      Returns the Reply-To header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::ReplyTo *replyTo( bool create = true );

    /**
      Returns the To header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::To *to( bool create = true );

    /**
      Returns the Cc header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::Cc *cc( bool create = true );

    /**
      Returns the Bcc header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::Bcc *bcc( bool create = true );

    /**
      Returns the References header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::References *references( bool create = true );

    /**
      Returns the User-Agent header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::UserAgent *userAgent( bool create = true );

    /**
      Returns the In-Reply-To header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::InReplyTo *inReplyTo( bool create = true );

    /**
      Returns the Sender header.
      @param create If true, create the header if it doesn't exist yet.
    */
    virtual KMime::Headers::Sender *sender( bool create = true );

    /* reimpl */
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

    /**
      Returns the MIME type used for Messages
    */
    static QString mimeType();

  protected:
    /* reimpl */
    virtual QByteArray assembleHeaders();

    // @cond PRIVATE
    explicit Message( MessagePrivate *d );
    // @endcond

  private:
    Q_DECLARE_PRIVATE( Message )

}; // class Message

} // namespace KMime

#endif // __KMIME_MESSAGE_H__
