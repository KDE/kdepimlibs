/*
    kmime_content.cpp

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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
  Volker Krause \<vkrause@kde.org\>
*/

#include "kmime_content.h"
#include "kmime_content_p.h"
#include "kmime_codecs.h"
#include "kmime_message.h"
#include "kmime_header_parsing.h"
#include "kmime_header_parsing_p.h"
#include "kmime_parsers.h"
#include "kmime_util_p.h"

#include <kcharsets.h>
#include <kcodecs.h>
#include <kglobal.h>
#include <klocale.h>
#include <klocalizedstring.h>
#include <qdebug.h>

#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QByteArray>
#include <KLocale>

using namespace KMime;

namespace KMime {

Content::Content()
  : d_ptr( new ContentPrivate( this ) )
{
}

Content::Content( Content *parent )
  : d_ptr( new ContentPrivate( this ) )
{
  d_ptr->parent = parent;
}

Content::Content( const QByteArray &h, const QByteArray &b )
  : d_ptr( new ContentPrivate( this ) )
{
  d_ptr->head = h;
  d_ptr->body = b;
}

Content::Content( const QByteArray &h, const QByteArray &b, Content *parent )
  : d_ptr( new ContentPrivate( this ) )
{
  d_ptr->head = h;
  d_ptr->body = b;
  d_ptr->parent = parent;
}

Content::Content( ContentPrivate *d )
  : d_ptr( d )
{
}

Content::~Content()
{
  qDeleteAll( h_eaders );
  h_eaders.clear();
  delete d_ptr;
  d_ptr = 0;
}

bool Content::hasContent() const
{
  return !d_ptr->head.isEmpty() || !d_ptr->body.isEmpty() || !d_ptr->contents().isEmpty();
}

void Content::setContent( const QList<QByteArray> &l )
{
  Q_D( Content );
  //qDebug("Content::setContent( const QList<QByteArray> &l ) : start");
  d->head.clear();
  d->body.clear();

  //usage of textstreams is much faster than simply appending the strings
  QTextStream hts( &( d->head ), QIODevice::WriteOnly );
  QTextStream bts( &( d->body ), QIODevice::WriteOnly );
  hts.setCodec( "ISO 8859-1" );
  bts.setCodec( "ISO 8859-1" );

  bool isHead = true;
  foreach ( const QByteArray& line, l ) {
    if ( isHead && line.isEmpty() ) {
      isHead = false;
      continue;
    }
    if ( isHead ) {
      hts << line << "\n";
    } else {
      bts << line << "\n";
    }
  }

  //qDebug("Content::setContent( const QList<QByteArray> & l ) : finished");
}

void Content::setContent( const QByteArray &s )
{
  Q_D( Content );
  KMime::HeaderParsing::extractHeaderAndBody( s, d->head, d->body );
}

QByteArray Content::head() const
{
  return d_ptr->head;
}

void Content::setHead( const QByteArray &head )
{
  d_ptr->head = head;
  if ( !head.endsWith( '\n' ) ) {
    d_ptr->head += '\n';
  }
}

QByteArray Content::body() const
{
  return d_ptr->body;
}

void Content::setBody( const QByteArray &body )
{
  d_ptr->body = body;
}

QByteArray Content::preamble() const
{
  return d_ptr->preamble;
}

void Content::setPreamble( const QByteArray &preamble )
{
  d_ptr->preamble = preamble;
}


QByteArray Content::epilogue() const
{
  return d_ptr->epilogue;
}

void Content::setEpilogue( const QByteArray &epilogue )
{
  d_ptr->epilogue = epilogue;
}

void Content::parse()
{
  Q_D( Content );

  // Clean up old headers and parse them again.
  qDeleteAll( h_eaders );
  h_eaders.clear();
  h_eaders = HeaderParsing::parseHeaders( d->head );
  foreach ( Headers::Base *h, h_eaders ) {
    h->setParent( this );
  }

  // If we are frozen, save the body as-is. This is done because parsing
  // changes the content (it loses preambles and epilogues, converts uuencode->mime, etc.)
  if ( d->frozen ) {
    d->frozenBody = d->body;
  }

  // Clean up old sub-Contents and parse them again.
  qDeleteAll( d->multipartContents );
  d->multipartContents.clear();
  d->clearBodyMessage();
  Headers::ContentType *ct = contentType();
  if ( ct->isText() ) {
    // This content is either text, or of unknown type.

    if ( d->parseUuencoded() ) {
      // This is actually uuencoded content generated by broken software.
    } else if ( d->parseYenc() ) {
      // This is actually yenc content generated by broken software.
    } else {
      // This is just plain text.
    }
  } else if ( ct->isMultipart() ) {
    // This content claims to be MIME multipart.

    if ( d->parseMultipart() ) {
      // This is actual MIME multipart content.
    } else {
      // Parsing failed; treat this content as "text/plain".
      ct->setMimeType( "text/plain" );
      ct->setCharset( "US-ASCII" );
    }
  } else {
    // This content is something else, like an encapsulated message or a binary attachment
    // or something like that
    if ( bodyIsMessage() ) {
      d->bodyAsMessage = Message::Ptr( new Message );
      d->bodyAsMessage->setContent( d->body );
      d->bodyAsMessage->setFrozen( d->frozen );
      d->bodyAsMessage->parse();
      d->bodyAsMessage->d_ptr->parent = this;

      // Clear the body, as it is now represented by d->bodyAsMessage. This is the same behavior
      // as with multipart contents, since parseMultipart() clears the body as well
      d->body.clear();
    }
  }
}

bool Content::isFrozen() const
{
  return d_ptr->frozen;
}

void Content::setFrozen( bool frozen )
{
  d_ptr->frozen = frozen;
}

void Content::assemble()
{
  Q_D( Content );
  if ( d->frozen ) {
    return;
  }

  d->head = assembleHeaders();
  foreach ( Content *c, contents() ) {
    c->assemble();
  }
}

QByteArray Content::assembleHeaders()
{
  QByteArray newHead;
  foreach ( const Headers::Base *h, h_eaders ) {
    if ( !h->isEmpty() ) {
      newHead += h->as7BitString() + '\n';
    }
  }

  return newHead;
}

void Content::clear()
{
  Q_D( Content );
  qDeleteAll( h_eaders );
  h_eaders.clear();
  clearContents();
  d->head.clear();
  d->body.clear();
}

void Content::clearContents( bool del )
{
  Q_D( Content );
  if ( del ) {
    qDeleteAll( d->multipartContents );
  }
  d->multipartContents.clear();
  d->clearBodyMessage();
}

QByteArray Content::encodedContent( bool useCrLf )
{
  Q_D( Content );
  QByteArray e;

  // Head.
  e = d->head;
  e += '\n';
  e += encodedBody();

  if ( useCrLf ) {
    return LFtoCRLF( e );
  } else {
    return e;
  }
}

QByteArray Content::encodedBody()
{
  Q_D( Content );
  QByteArray e;
  // Body.
  if ( d->frozen ) {
    // This Content is frozen.
    if ( d->frozenBody.isEmpty() ) {
      // This Content has never been parsed.
      e += d->body;
    } else {
      // Use the body as it was before parsing.
      e += d->frozenBody;
    }
  } else if ( bodyIsMessage() && d->bodyAsMessage ) {
    // This is an encapsulated message
    // No encoding needed, as the ContentTransferEncoding can only be 7bit
    // for encapsulated messages
    e += d->bodyAsMessage->encodedContent();
  } else if ( !d->body.isEmpty() ) {
    // This is a single-part Content.
    Headers::ContentTransferEncoding *enc = contentTransferEncoding();

    if ( enc->needToEncode() ) {
      if ( enc->encoding() == Headers::CEquPr ) {
        e += KCodecs::quotedPrintableEncode( d->body, false );
      } else {
        e += KCodecs::base64Encode( d->body, true );
        e += '\n';
      }
    } else {
      e += d->body;
    }
  }

  if ( !d->frozen && !d->multipartContents.isEmpty() ) {
    // This is a multipart Content.
    Headers::ContentType *ct=contentType();
    QByteArray boundary = "\n--" + ct->boundary();

    if ( !d->preamble.isEmpty() ) {
      e += d->preamble;
    }

    //add all (encoded) contents separated by boundaries
    foreach ( Content *c, d->multipartContents ) {
      e += boundary + '\n';
      e += c->encodedContent( false );  // don't convert LFs here, we do that later!!!!!
    }
    //finally append the closing boundary
    e += boundary+"--\n";

    if ( !d->epilogue.isEmpty() ) {
      e += d->epilogue;
    }
  }
  return e;
}

QByteArray Content::decodedContent()
{
  QByteArray ret;
  Headers::ContentTransferEncoding *ec=contentTransferEncoding();
  bool removeTrailingNewline=false;

  if ( d_ptr->body.length() == 0 ) {
    return ret;
  }

  if ( ec->decoded() ) {
    ret = d_ptr->body;
    //Laurent Fix bug #311267
    //removeTrailingNewline = true;
  } else {
    switch ( ec->encoding() ) {
    case Headers::CEbase64 :
    {
      KMime::Codec *codec = KMime::Codec::codecForName( "base64" );
      Q_ASSERT( codec );
      ret.resize( codec->maxDecodedSizeFor( d_ptr->body.size() ) );
      KMime::Decoder* decoder = codec->makeDecoder();
      QByteArray::const_iterator inputIt = d_ptr->body.constBegin();
      QByteArray::iterator resultIt = ret.begin();
      decoder->decode( inputIt, d_ptr->body.constEnd(), resultIt, ret.end() );
      ret.truncate( resultIt - ret.begin() );
      break;
    }
    case Headers::CEquPr :
      ret = KCodecs::quotedPrintableDecode( d_ptr->body );
      removeTrailingNewline = true;
      break;
    case Headers::CEuuenc :
      KCodecs::uudecode( d_ptr->body, ret );
      break;
    case Headers::CEbinary :
      ret = d_ptr->body;
      removeTrailingNewline = false;
      break;
    default :
      ret = d_ptr->body;
      removeTrailingNewline = true;
    }
  }

  if ( removeTrailingNewline && ( ret.size() > 0 ) && ( ret[ret.size() - 1] == '\n' ) ) {
    ret.resize( ret.size() - 1 );
  }

  return ret;
}

QString Content::decodedText( bool trimText, bool removeTrailingNewlines )
{
  if ( !decodeText() ) { //this is not a text content !!
    return QString();
  }

  bool ok = true;
  QTextCodec *codec =
    KCharsets::charsets()->codecForName( QLatin1String( contentType()->charset() ), ok );
  if ( !ok  || codec == NULL ) { // no suitable codec found => try local settings and hope the best ;-)
    codec = KLocale::global()->codecForEncoding();
    QByteArray chset = KLocale::global()->encoding();
    contentType()->setCharset( chset );
  }

  QString s = codec->toUnicode( d_ptr->body.data(), d_ptr->body.length() );

  if ( trimText || removeTrailingNewlines ) {
    int i;
    for ( i = s.length() - 1; i >= 0; --i ) {
      if ( trimText ) {
        if ( !s[i].isSpace() ) {
          break;
        }
      }
      else {
        if ( s[i] != QLatin1Char( '\n' ) ) {
          break;
        }
      }
    }
    s.truncate( i + 1 );
  } else {
    if ( s.right( 1 ) == QLatin1String( "\n" ) ) {
      s.truncate( s.length() - 1 ); // remove trailing new-line
    }
  }

  return s;
}

void Content::fromUnicodeString( const QString &s )
{
  bool ok = true;
  QTextCodec *codec =
    KCharsets::charsets()->codecForName( QLatin1String( contentType()->charset() ), ok );

  if ( !ok ) { // no suitable codec found => try local settings and hope the best ;-)
    codec = KLocale::global()->codecForEncoding();
    QByteArray chset = KLocale::global()->encoding();
    contentType()->setCharset( chset );
  }

  d_ptr->body = codec->fromUnicode( s );
  contentTransferEncoding()->setDecoded( true ); //text is always decoded
}

Content *Content::textContent()
{
  Content *ret=0;

  //return the first content with mimetype=text/*
  if ( contentType()->isText() ) {
    ret = this;
  } else {
    foreach ( Content *c, d_ptr->contents() ) {
      if ( ( ret = c->textContent() ) != 0 ) {
        break;
      }
    }
  }
  return ret;
}

Content::List Content::attachments( bool incAlternatives )
{
  List attachments;
  if ( d_ptr->contents().isEmpty() ) {
    attachments.append( this );
  } else {
    foreach ( Content *c, d_ptr->contents() ) {
      if ( !incAlternatives &&
           c->contentType()->category() == Headers::CCalternativePart ) {
        continue;
      } else {
        attachments += c->attachments( incAlternatives );
      }
    }
  }

  if ( isTopLevel() ) {
    Content *text = textContent();
    if ( text ) {
      attachments.removeAll( text );
    }
  }
  return attachments;
}

Content::List Content::contents() const
{
  return d_ptr->contents();
}

void Content::addContent( Content *c, bool prepend )
{
  Q_D( Content );

  // This method makes no sense for encapsulated messages
  Q_ASSERT( !bodyIsMessage() );

  // If this message is single-part; make it multipart first.
  if( d->multipartContents.isEmpty() && !contentType()->isMultipart() ) {
    // The current body will be our first sub-Content.
    Content *main = new Content( this );

    // Move the MIME headers to the newly created sub-Content.
    // NOTE: The other headers (RFC5322 headers like From:, To:, as well as X-headers
    // are not moved to the subcontent; they remain with the top-level content.
    for ( Headers::Base::List::iterator it = h_eaders.begin();
          it != h_eaders.end(); ) {
      if ( (*it)->isMimeHeader() ) {
        // Add to new content.
        main->setHeader( *it );
        // Remove from this content.
        it = h_eaders.erase( it );
      } else {
        ++it;
      }
    }

    // Adjust the Content-Type of the newly created sub-Content.
    main->contentType()->setCategory( Headers::CCmixedPart );

    // Move the body to the new subcontent.
    main->setBody( d->body );
    d->body.clear();

    // Add the subcontent.
    d->multipartContents.append( main );

    // Convert this content to "multipart/mixed".
    Headers::ContentType *ct = contentType();
    ct->setMimeType( "multipart/mixed" );
    ct->setBoundary( multiPartBoundary() );
    ct->setCategory( Headers::CCcontainer );
    contentTransferEncoding()->clear();  // 7Bit, decoded.
  }

  // Add the new content.
  if( prepend ) {
    d->multipartContents.prepend( c );
  } else {
    d->multipartContents.append( c );
  }

  if( c->parent() != this ) {
    // If the content was part of something else, this will remove it from there.
    c->setParent( this );
  }
}

void Content::removeContent( Content *c, bool del )
{
  Q_D( Content );
  if ( d->multipartContents.isEmpty() || !d->multipartContents.contains( c ) ) {
    return;
  }

  // This method makes no sense for encapsulated messages.
  // Should be covered by the above assert already, though.
  Q_ASSERT( !bodyIsMessage() );

  d->multipartContents.removeAll( c );
  if ( del ) {
    delete c;
  } else {
    c->d_ptr->parent = 0;
  }

  // If only one content is left, turn this content into a single-part.
  if( d->multipartContents.count() == 1 ) {
    Content *main = d->multipartContents.first();

    // Move all headers from the old subcontent to ourselves.
    // NOTE: This also sets the new Content-Type.
    foreach( Headers::Base *h, main->h_eaders ) {
      setHeader( h ); // Will remove the old one if present.
    }
    main->h_eaders.clear();

    // Move the body.
    d->body = main->body();

    // Delete the old subcontent.
    delete main;
    d->multipartContents.clear();
  }
}

void Content::changeEncoding( Headers::contentEncoding e )
{
  // This method makes no sense for encapsulated messages, they are always 7bit
  // encoded.
  Q_ASSERT( !bodyIsMessage() );

  Headers::ContentTransferEncoding *enc = contentTransferEncoding();
  if( enc->encoding() == e ) {
    // Nothing to do.
    return;
  }

  if( decodeText() ) {
    // This is textual content.  Textual content is stored decoded.
    Q_ASSERT( enc->decoded() );
    enc->setEncoding( e );
  } else {
    // This is non-textual content.  Re-encode it.
    if( e == Headers::CEbase64 ) {
      d_ptr->body = KCodecs::base64Encode( decodedContent(), true );
      d_ptr->body.append( "\n" );
      enc->setEncoding( e );
      enc->setDecoded( false );
    } else {
      // It only makes sense to convert binary stuff to base64.
      Q_ASSERT( false );
    }
  }
}

void Content::toStream( QTextStream &ts, bool scrambleFromLines )
{
  QByteArray ret = encodedContent( false );

  if ( scrambleFromLines ) {
    // FIXME Why are only From lines with a preceding empty line considered?
    //       And, of course, all lines starting with >*From have to be escaped
    //       because otherwise the transformation is not revertable.
    ret.replace( "\n\nFrom ", "\n\n>From ");
  }
  ts << ret;
}

Headers::Generic *Content::getNextHeader( QByteArray &head )
{
  return d_ptr->nextHeader( head );
}

Headers::Generic *Content::nextHeader( QByteArray &head )
{
  return d_ptr->nextHeader( head );
}

Headers::Generic *ContentPrivate::nextHeader( QByteArray &_head )
{
  Headers::Base *header = HeaderParsing::extractFirstHeader( _head );
  if ( !header ) {
    return 0;
  }
  // Convert it from the real class to Generic.
  Headers::Generic *ret = new Headers::Generic( header->type(), q_ptr );
  ret->from7BitString( header->as7BitString() );
  return ret;
}

Headers::Base *Content::getHeaderByType( const char *type )
{
  return headerByType( type );
}

Headers::Base *Content::headerByType( const char *type )
{
  Q_ASSERT( type  && *type );

  foreach( Headers::Base *h, h_eaders ) {
    if( h->is( type ) ) {
      return h; // Found.
    }
  }

  return 0; // Not found.
}

Headers::Base::List Content::headersByType( const char *type )
{
  Q_ASSERT( type && *type );

  Headers::Base::List result;

  foreach( Headers::Base *h, h_eaders ) {
    if( h->is( type ) ) {
      result << h;
    }
  }

  return result;
}

void Content::setHeader( Headers::Base *h )
{
  Q_ASSERT( h );
  removeHeader( h->type() );
  appendHeader( h );
}

void Content::appendHeader( Headers::Base *h )
{
  h_eaders.append( h );
  h->setParent( this );
}

void Content::prependHeader( Headers::Base *h )
{
  h_eaders.prepend( h );
  h->setParent( this );
}

bool Content::removeHeader( const char *type )
{
  for ( Headers::Base::List::iterator it = h_eaders.begin();
        it != h_eaders.end(); ++it )
    if ( (*it)->is(type) ) {
      delete (*it);
      h_eaders.erase( it );
      return true;
    }

  return false;
}

bool Content::hasHeader( const char *type )
{
  return headerByType( type ) != 0;
}

int Content::size()
{
  int ret = d_ptr->body.length();

  if ( contentTransferEncoding()->encoding() == Headers::CEbase64 ) {
    KMime::Codec *codec = KMime::Codec::codecForName( "base64" );
    return codec->maxEncodedSizeFor(ret);
  }

  // Not handling quoted-printable here since that requires actually
  // converting the content, and that is O(size_of_content).
  // For quoted-printable, this is only an approximate size.

  return ret;
}

int Content::storageSize() const
{
  const Q_D( Content );
  int s = d->head.size();

  if ( d->contents().isEmpty() ) {
    s += d->body.size();
  } else {

    // FIXME: This should take into account the boundary headers that are added in
    //        encodedContent!
    foreach ( Content *c, d->contents() ) {
      s += c->storageSize();
    }
  }

  return s;
}

int Content::lineCount() const
{
  const Q_D( Content );
  int ret = 0;
  if ( !isTopLevel() ) {
    ret += d->head.count( '\n' );
  }
  ret += d->body.count( '\n' );

  foreach ( Content *c, d->contents() ) {
    ret += c->lineCount();
  }

  return ret;
}

QByteArray Content::rawHeader( const char *name ) const
{
  return KMime::extractHeader( d_ptr->head, name );
}

QList<QByteArray> Content::rawHeaders( const char *name ) const
{
  return KMime::extractHeaders( d_ptr->head, name );
}

bool Content::decodeText()
{
  Q_D( Content );
  Headers::ContentTransferEncoding *enc = contentTransferEncoding();

  if ( !contentType()->isText() ) {
    return false; //non textual data cannot be decoded here => use decodedContent() instead
  }
  if ( enc->decoded() ) {
    return true; //nothing to do
  }

  switch( enc->encoding() )
  {
  case Headers::CEbase64 :
    d->body = KCodecs::base64Decode( d->body );
    d->body.append( "\n" );
    break;
  case Headers::CEquPr :
    d->body = KCodecs::quotedPrintableDecode( d->body );
    break;
  case Headers::CEuuenc :
    d->body = KCodecs::uudecode( d->body );
    d->body.append( "\n" );
    break;
  case Headers::CEbinary :
    // nothing to decode
    d->body.append( "\n" );
  default :
    break;
  }
  enc->setDecoded( true );
  return true;
}

QByteArray Content::defaultCharset() const
{
  return d_ptr->defaultCS;
}

void Content::setDefaultCharset( const QByteArray &cs )
{
  d_ptr->defaultCS = KMime::cachedCharset( cs );

  foreach ( Content *c, d_ptr->contents() ) {
    c->setDefaultCharset( cs );
  }

  // reparse the part and its sub-parts in order
  // to clear cached header values
  parse();
}

bool Content::forceDefaultCharset() const
{
  return d_ptr->forceDefaultCS;
}

void Content::setForceDefaultCharset( bool b )
{
  d_ptr->forceDefaultCS = b;

  foreach ( Content *c, d_ptr->contents() ) {
    c->setForceDefaultCharset( b );
  }

  // reparse the part and its sub-parts in order
  // to clear cached header values
  parse();
}

Content * KMime::Content::content( const ContentIndex &index ) const
{
  if ( !index.isValid() ) {
    return const_cast<KMime::Content*>( this );
  }
  ContentIndex idx = index;
  unsigned int i = idx.pop() - 1; // one-based -> zero-based index
  if ( i < (unsigned int)d_ptr->contents().size() ) {
    return d_ptr->contents()[i]->content( idx );
  } else {
    return 0;
  }
}

ContentIndex KMime::Content::indexForContent( Content * content ) const
{
  int i = d_ptr->contents().indexOf( content );
  if ( i >= 0 ) {
    ContentIndex ci;
    ci.push( i + 1 ); // zero-based -> one-based index
    return ci;
  }
  // not found, we need to search recursively
  for ( int i = 0; i < d_ptr->contents().size(); ++i ) {
    ContentIndex ci = d_ptr->contents()[i]->indexForContent( content );
    if ( ci.isValid() ) {
      // found it
      ci.push( i + 1 ); // zero-based -> one-based index
      return ci;
    }
  }
  return ContentIndex(); // not found
}

bool Content::isTopLevel() const
{
  return d_ptr->parent == 0;
}

void Content::setParent( Content *parent )
{
  // Make sure the Content is only in the contents list of one parent object
  Content *oldParent = d_ptr->parent;
  if ( oldParent ) {
    if ( !oldParent->contents().isEmpty() && oldParent->contents().contains( this ) ) {
      oldParent->removeContent( this );
    }
  }

  d_ptr->parent = parent;
  if ( parent ) {
    if ( !parent->contents().isEmpty() && !parent->contents().contains( this ) ) {
      parent->addContent( this );
    }
  }
}

Content *Content::parent() const
{
  return d_ptr->parent;
}

Content *Content::topLevel() const
{
  Content *top = const_cast<Content*>(this);
  Content *c = parent();
  while ( c ) {
    top = c;
    c = c->parent();
  }

  return top;
}

ContentIndex Content::index() const
{
  Content* top = topLevel();
  if ( top ) {
    return top->indexForContent( const_cast<Content*>(this) );
  }

  return indexForContent( const_cast<Content*>(this)  );
}

Message::Ptr Content::bodyAsMessage() const
{
  if ( bodyIsMessage() && d_ptr->bodyAsMessage ) {
    return d_ptr->bodyAsMessage;
  } else {
    return Message::Ptr();
  }
}

bool Content::bodyIsMessage() const
{
  // Use const_case here to work around API issue that neither header() nor hasHeader() are
  // const, even though they should be
  return const_cast<Content*>( this )->header<Headers::ContentType>( false ) &&
         const_cast<Content*>( this )->header<Headers::ContentType>( true )
                 ->mimeType().toLower() == "message/rfc822";
}

// @cond PRIVATE
#define kmime_mk_header_accessor( type, method ) \
Headers::type *Content::method( bool create ) { \
  return header<Headers::type>( create ); \
}

kmime_mk_header_accessor( ContentType, contentType )
kmime_mk_header_accessor( ContentTransferEncoding, contentTransferEncoding )
kmime_mk_header_accessor( ContentDisposition, contentDisposition )
kmime_mk_header_accessor( ContentDescription, contentDescription )
kmime_mk_header_accessor( ContentLocation, contentLocation )
kmime_mk_header_accessor( ContentID, contentID )

#undef kmime_mk_header_accessor
// @endcond


void ContentPrivate::clearBodyMessage()
{
  bodyAsMessage.reset();
}

Content::List ContentPrivate::contents() const
{
  Q_ASSERT( multipartContents.isEmpty() || !bodyAsMessage );
  if ( bodyAsMessage )
    return Content::List() << bodyAsMessage.get();
  else
    return multipartContents;
}

bool ContentPrivate::parseUuencoded()
{
  Q_Q( Content );
  Parser::UUEncoded uup( body, KMime::extractHeader( head, "Subject" ) );
  if( !uup.parse() ) {
    return false; // Parsing failed.
  }

  Headers::ContentType *ct = q->contentType();
  ct->clear();

  if( uup.isPartial() ) {
    // This seems to be only a part of the message, so we treat it as "message/partial".
    ct->setMimeType( "message/partial" );
    //ct->setId( uniqueString() ); not needed yet
    ct->setPartialParams( uup.partialCount(), uup.partialNumber() );
    q->contentTransferEncoding()->setEncoding( Headers::CE7Bit );
  } else {
    // This is a complete message, so treat it as "multipart/mixed".
    body.clear();
    ct->setMimeType( "multipart/mixed" );
    ct->setBoundary( multiPartBoundary() );
    ct->setCategory( Headers::CCcontainer );
    q->contentTransferEncoding()->clear(); // 7Bit, decoded.

    // Add the plain text part first.
    Q_ASSERT( multipartContents.count() == 0 );
    {
      Content *c = new Content( q );
      c->contentType()->setMimeType( "text/plain" );
      c->contentTransferEncoding()->setEncoding( Headers::CE7Bit );
      c->setBody( uup.textPart() );
      multipartContents.append( c );
    }

    // Now add each of the binary parts as sub-Contents.
    for( int i = 0; i < uup.binaryParts().count(); ++i ) {
      Content *c = new Content( q );
      c->contentType()->setMimeType( uup.mimeTypes().at( i ) );
      c->contentType()->setName( QLatin1String( uup.filenames().at( i ) ), QByteArray( /*charset*/ ) );
      c->contentTransferEncoding()->setEncoding( Headers::CEuuenc );
      c->contentTransferEncoding()->setDecoded( false );
      c->contentDisposition()->setDisposition( Headers::CDattachment );
      c->contentDisposition()->setFilename( QLatin1String( uup.filenames().at( i ) ) );
      c->setBody( uup.binaryParts().at( i ) );
      c->changeEncoding( Headers::CEbase64 ); // Convert to base64.
      multipartContents.append( c );
    }
  }

  return true; // Parsing successful.
}

bool ContentPrivate::parseYenc()
{
  Q_Q( Content );
  Parser::YENCEncoded yenc( body );
  if ( !yenc.parse() ) {
    return false; // Parsing failed.
  }

  Headers::ContentType *ct = q->contentType();
  ct->clear();

  if ( yenc.isPartial() ) {
    // Assume there is exactly one decoded part.  Treat this as "message/partial".
    ct->setMimeType( "message/partial" );
    //ct->setId( uniqueString() ); not needed yet
    ct->setPartialParams( yenc.partialCount(), yenc.partialNumber() );
    q->contentTransferEncoding()->setEncoding( Headers::CEbinary );
    q->changeEncoding( Headers::CEbase64 ); // Convert to base64.
  } else {
    // This is a complete message, so treat it as "multipart/mixed".
    body.clear();
    ct->setMimeType( "multipart/mixed" );
    ct->setBoundary( multiPartBoundary() );
    ct->setCategory( Headers::CCcontainer );
    q->contentTransferEncoding()->clear(); // 7Bit, decoded.

    // Add the plain text part first.
    Q_ASSERT( multipartContents.count() == 0 );
    {
      Content *c = new Content( q );
      c->contentType()->setMimeType( "text/plain" );
      c->contentTransferEncoding()->setEncoding( Headers::CE7Bit );
      c->setBody( yenc.textPart() );
      multipartContents.append( c );
    }

    // Now add each of the binary parts as sub-Contents.
    for ( int i=0; i<yenc.binaryParts().count(); i++ ) {
      Content *c = new Content( q );
      c->contentType()->setMimeType( yenc.mimeTypes().at( i ) );
      c->contentType()->setName( QLatin1String( yenc.filenames().at( i ) ), QByteArray( /*charset*/ ) );
      c->contentTransferEncoding()->setEncoding( Headers::CEbinary );
      c->contentDisposition()->setDisposition( Headers::CDattachment );
      c->contentDisposition()->setFilename( QLatin1String( yenc.filenames().at( i ) ) );
      c->setBody( yenc.binaryParts().at( i ) ); // Yenc bodies are binary.
      c->changeEncoding( Headers::CEbase64 ); // Convert to base64.
      multipartContents.append( c );
    }
  }

  return true; // Parsing successful.
}

bool ContentPrivate::parseMultipart()
{
  Q_Q( Content );
  const Headers::ContentType *ct = q->contentType();
  const QByteArray boundary = ct->boundary();
  if ( boundary.isEmpty() ) {
    return false; // Parsing failed; invalid multipart content.
  }
  Parser::MultiPart mpp( body, boundary );
  if ( !mpp.parse() ) {
    return false; // Parsing failed.
  }

  preamble = mpp.preamble();
  epilogue = mpp.epilouge();

  // Determine the category of the subparts (used in attachments()).
  Headers::contentCategory cat;
  if ( ct->isSubtype( "alternative" ) ) {
    cat = Headers::CCalternativePart;
  } else {
    cat = Headers::CCmixedPart; // Default to "mixed".
  }

  // Create a sub-Content for every part.
  Q_ASSERT( multipartContents.isEmpty() );
  body.clear();
  QList<QByteArray> parts = mpp.parts();
  foreach ( const QByteArray &part, mpp.parts() ) {
    Content *c = new Content( q );
    c->setContent( part );
    c->setFrozen( frozen );
    c->parse();
    c->contentType()->setCategory( cat );
    multipartContents.append( c );
  }

  return true; // Parsing successful.
}

} // namespace KMime
