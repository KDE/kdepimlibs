/*
    kmime_content.h

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

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
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the Content class.

  @brief
  Defines the Content class.

  @authors the KMime authors (see AUTHORS file),
  Volker Krause \<volker.krause@rwth-aachen.de\>

TODO: possible glossary terms:
 content
   encoding, transfer type, disposition, description
 header
 body
 attachment
 charset
 article
*/

#ifndef __KMIME_CONTENT_H__
#define __KMIME_CONTENT_H__

#include <QtCore/QTextStream>
#include <QtCore/QByteArray>
#include <QtCore/QList>

#include "kmime.h"
#include "kmime_contentindex.h"
#include "kmime_util.h"
#include "kmime_headers.h"

namespace KMime {

class ContentPrivate;

/**
  @brief
  A class that encapsulates @ref MIME encoded Content.

  It parses the given data and creates a tree-like structure that
  represents the structure of the message.
*/
class KMIME_EXPORT Content
{
  public:

    typedef QList<KMime::Content*> List;

    /**
      Creates an empty Content object.
    */
    Content();

    /**
      Creates a Content object containing the given raw data.

      @param head is a QByteArray containing the header data.
      @param body is a QByteArray containing the body data.
    */
    Content( const QByteArray &head, const QByteArray &body );

    /**
      Destroys this Content object.
    */
    virtual ~Content();

    /**
      Returns true if this Content object is not empty.
    */
    bool hasContent() const;

    /**
      Sets the Content to the given raw data, containing the Content head and
      body separated by two linefeeds.

      @param l is a line-splitted list of the raw Content data.
    */
    void setContent( const QList<QByteArray> &l );

    /**
      Sets the Content to the given raw data, containing the Content head and
      body separated by two linefeeds.

      @param s is a QByteArray containing the raw Content data.
    */
    void setContent( const QByteArray &s );

    /**
      Parses the Contents, splitting into multiple sub-Contents.
    */
    virtual void parse();

    /**
      Call to generate the MIME structure of the message.
    */
    virtual void assemble();

    /**
      Clears the complete message and deletes all sub-Contents.
    */
    virtual void clear();

    /**
      Returns the Content header raw data.

      @see setHead().
    */
    QByteArray head() const;

    /**
      Sets the Content header raw data.

      @param head is a QByteArray containing the header data.

      @see head().
    */
    void setHead( const QByteArray &head );

    /**
      Extracts and removes the next header from @p head.
      The caller is responsible for deleting the returned header.

      @param head is a QByteArray containing the header data.
    */
    Headers::Generic *getNextHeader( QByteArray &head );

    virtual Headers::Base *getHeaderByType( const char *type );

    virtual void setHeader( Headers::Base *h );

    virtual bool removeHeader( const char *type );

    bool hasHeader( const char *type )
      { return getHeaderByType( type ) != 0; }

    /**
      Returns the Content type header.

      @param create if true, create the header if it doesn't exist yet.
    */
    Headers::ContentType *contentType( bool create=true )
      {
        Headers::ContentType *p=0;
        return getHeaderInstance( p, create );
      }

    /**
      Returns the Content transfer encoding.

      @param create if true, create the header if it doesn't exist yet.
    */
    Headers::ContentTransferEncoding *contentTransferEncoding( bool create=true )
      {
        Headers::ContentTransferEncoding *p=0;
        return getHeaderInstance( p, create );
      }

    /**
      Returns the Content disposition.

      @param create if true, create the header if it doesn't exist yet.
    */
    Headers::ContentDisposition *contentDisposition( bool create=true )
      {
        Headers::ContentDisposition *p=0;
        return getHeaderInstance( p, create );
      }

    /**
      Returns the Content description.

      @param create if true, create the header if it doesn't exist yet.
    */
    Headers::ContentDescription *contentDescription( bool create=true )
      {
        Headers::ContentDescription *p=0;
        return getHeaderInstance( p, create );
      }

    /**
      Returns the size of the Content body after encoding.
    */
    int size();

    /**
      Returns the size of this Content and all sub-Contents.
    */
    int storageSize() const;

    /**
      Line count of this Content and all sub-Contents.
    */
    int lineCount() const;

    /**
      Returns the Content body raw data.

      @see setBody().
    */
    QByteArray body() const;

    /**
      Sets the Content body raw data.

      @param body is a QByteArray containing the body data.

      @see body().
    */
    void setBody( const QByteArray &body );

    /**
      Returns a QByteArray containing the encoded Content, including the
      Content header and all sub-Contents.

      @param useCrLf if true, use @ref CRLF instead of @ref LF for linefeeds.
    */
    QByteArray encodedContent( bool useCrLf = false );

    /**
      Returns the decoded Content body.
    */
    QByteArray decodedContent();

    /**
      Returns the decoded text. Additional to decodedContent(), this also
      applies charset decoding. If this is not a text Content, decodedText()
      returns an empty QString.

      @param trimText if true, then the decoded text will have all trailing
      whitespace removed.
      @param removeTrailingNewlines if true, then the decoded text will have
      all consecutive trailing newlines removed.

      The last trailing new line of the decoded text is always removed.

    */
    QString decodedText( bool trimText = false,
                         bool removeTrailingNewlines = false );

    /**
      Sets the Content body to the given string using the current charset.

      @param s Unicode-encoded string.
    */
    void fromUnicodeString( const QString &s );

    /**
      Returns the first Content with mimetype text/.
    */
    Content *textContent();

    /**
      Returns a list of attachments.

      @param incAlternatives if true, include multipart/alternative parts.
    */
    List attachments( bool incAlternatives = false );

    /**
      Returns a list of sub-Contents. By default only the top level Contents
      are returned.

      @param recursive if true, all parts in the message are returned. 
    */
    List contents( bool recursive = false ) const;

    /**
      Adds a new sub-Content, the current Content object is converted into a
      multipart/mixed Content node if it has been a single-part Content.

      @param c The new sub-Content.
      @param prepend if true, prepend to the Content list; else append
      to the Content list.

      @see removeContent().
    */
    void addContent( Content *c, bool prepend = false );

    /**
      Removes the given sub-Content, the current Content object is converted
      into a single-port Content if only one sub-Content is left.

      @param c The Content to remove.
      @param del if true, delete the removed Content object.

      @see addContent().
    */
    void removeContent( Content *c, bool del = false );

    void changeEncoding( Headers::contentEncoding e );

    /**
      Saves the encoded Content to the given textstream

      @param ts is the stream where the Content should be written to.
      @param scrambleFromLines: if true, replace "\nFrom " with "\n>From "
      in the stream. This is needed to avoid problem with mbox-files
    */
    void toStream( QTextStream &ts, bool scrambleFromLines = false );

    /**
      Returns the charset that is used for all headers and the body
      if the charset is not declared explictly.

      @see setDefaultCharset()
    */
    QByteArray defaultCharset() const;

    /**
      Sets the default charset.

      @param cs is a QByteArray containing the new default charset.

      @see defaultCharset().
    */
    void setDefaultCharset( const QByteArray &cs );

    /**
      Use the default charset even if a different charset is
      declared in the article.

      @see setForceDefaultCharset().
    */
    bool forceDefaultCharset() const;

    /**
      Enables/disables the force mode, housekeeping.
      works correctly only when the article is completely empty or
      completely loaded.

      @param b if true, force the default charset to be used.

      @see forceDefaultCharset().
    */
    virtual void setForceDefaultCharset( bool b );

    /**
      Returns the Content specified by the given index.
      If the index doesn't point to an Content, 0 is returned, if the index
      is invalid (empty), this Content is returned.

      @param index the Content index
    */
    Content *content( const ContentIndex &index ) const;

    /**
      Returns the ContentIndex for the given Content, an invalid index
      if the Content is not found withing the hierarchy.
      @param content the Content object to search.
    */
    ContentIndex indexForContent( Content *content ) const;

    /**
      Returns true if this is the top-level node in the MIME tree, ie. if this
      is actually a message or news article.
    */
    virtual bool isTopLevel() const;

  protected:
    /**
      Reimplement this method if you need to assemble additional headers in a
      derived class. Don't forget to call the implementation of the base class.
      @return The raw, assembled headers.
    */
    virtual QByteArray assembleHeaders();

    QByteArray rawHeader( const char *name );
    bool decodeText();
    template <class T> T *getHeaderInstance( T *ptr, bool create );

    Headers::Base::List h_eaders;

  private:
    ContentPrivate *const d;
};

// some compilers (for instance Compaq C++) need template inline functions
// here rather than in the *.cpp file

template <class T> T *Content::getHeaderInstance( T *ptr, bool create )
{
  T dummy; //needed to access virtual member T::type()

  ptr=static_cast <T*> ( getHeaderByType( dummy.type() ) );
  if ( !ptr && create ) { //no such header found, but we need one => create it
    ptr = new T( this );
    h_eaders.append( ptr );
  }

  return ptr;
}

} // namespace KMime

#endif // __KMIME_CONTENT_H__
