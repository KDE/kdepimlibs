/*  -*- c++ -*-

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2001-2002 Marc Mutz <mutz@kde.org>

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
  defines the Codec class.

  @brief
  Defines the classes Codec class.

  @authors Marc Mutz \<mutz@kde.org\>

  @glossary @anchor MIME @anchor mime @b MIME:
  <b>Multipurpose Internet Mail Extensions</b> or @acronym MIME is an
  Internet Standard that extends the format of e-mail to support text in
  character sets other than US-ASCII, non-text attachments, multi-part message
  bodies, and header information in non-ASCII character sets. Virtually all
  human-written Internet e-mail and a fairly large proportion of automated
  e-mail is transmitted via @acronym SMTP in MIME format. Internet e-mail is
  so closely associated with the SMTP and MIME standards that it is sometimes
  called SMTP/MIME e-mail. The content types defined by MIME standards are
  also of growing importance outside of e-mail, such as in communication
  protocols like @acronym HTTP for the World Wide Web. MIME is also a
  fundamental component of communication protocols such as  HTTP, which
  requires that data be transmitted in the context of e-mail-like messages,
  even though the data may not actually be e-mail.

  @glossary @anchor codec @anchor codecs @anchor Codec @anchor Codecs @b codec:
  a program capable of performing encoding and decoding on a digital data
  stream. Codecs encode data for storage or encryption and decode it for
  viewing or editing.

  @glossary @anchor CRLF @b CRLF: a "Carriage Return (0x0D)" followed by a
  "Line Feed (0x0A)", two ASCII control characters used to represent a
  newline on some operating systems, notably DOS and Microsoft Windows.

  @glossary @anchor LF @b LF: a "Line Feed (0x0A)" ASCII control character used
  to represent a newline on some operating systems, notably Unix, Unix-like,
  and Linux.
*/

#ifndef __KMIME_CODECS__
#define __KMIME_CODECS__

#include <QtCore/QByteArray>


#include "kmime_export.h"
#include <QDebug>
namespace KMime {

template <class Key, class T> class KAutoDeleteHash;

class Encoder;
class Decoder;

/**
  @brief
  An abstract base class of @ref codecs for common mail transfer encodings.

  Provides an abstract base class of @ref codecs like base64 and quoted-printable.
  Implemented as a singleton.
*/
class KMIME_EXPORT Codec
{
  protected:
    //@cond PRIVATE
    static KAutoDeleteHash<QByteArray, Codec> *all;
    static void cleanupCodec();
    //@endcond
    /**
      Contructs the codec.
    */
    Codec() {}

  public:
    /**
      Returns a codec associated with the specified @p name.

      @param name points to a character string containing a valid codec name.
    */
    static Codec *codecForName( const char *name );

    /**
      Returns a codec associated with the specified @p name.

      @param name is a QByteArray containing a valid codec name.
    */
    static Codec *codecForName( const QByteArray &name );

    /**
      Computes the maximum size, in characters, needed for the encoding.

      @param insize is the number of input characters to be encoded.
      @param withCRLF if true, make the newlines @ref CRLF; else use @ref LF.

      @return the maximum number of characters in the encoding.
    */
    virtual int maxEncodedSizeFor( int insize, bool withCRLF=false ) const = 0;

    /**
      Computes the maximum size, in characters, needed for the deccoding.

      @param insize is the number of input characters to be decoded.
      @param withCRLF if true, make the newlines @ref CRLF; else use @ref LF.

      @return the maximum number of characters in the decoding.
    */
    virtual int maxDecodedSizeFor( int insize, bool withCRLF=false ) const = 0;

    /**
      Creates the encoder for the codec.

      @param withCRLF if true, make the newlines @ref CRLF; else use @ref LF.

      @return a pointer to an instance of the codec's encoder.
    */
    virtual Encoder *makeEncoder( bool withCRLF=false ) const = 0;

    /**
      Creates the decoder for the codec.

      @param withCRLF if true, make the newlines @ref CRLF; else use @ref LF.

      @return a pointer to an instance of the codec's decoder.
    */
    virtual Decoder *makeDecoder( bool withCRLF=false ) const = 0;

    /**
      Convenience wrapper that can be used for small chunks of data
      when you can provide a large enough buffer. The default
      implementation creates an Encoder and uses it.

      Encodes a chunk of bytes starting at @p scursor and extending to
      @p send into the buffer described by @p dcursor and @p dend.

      This function doesn't support chaining of blocks. The returned
      block cannot be added to, but you don't need to finalize it, too.

      Example usage (@p in contains the input data):
      <pre>
      KMime::Codec *codec = KMime::Codec::codecForName( "base64" );
      kFatal( !codec ) << "no base64 codec found!?";
      QByteArray out( in.size()*1.4 ); // crude maximal size of b64 encoding
      QByteArray::Iterator iit = in.begin();
      QByteArray::Iterator oit = out.begin();
      if ( !codec->encode( iit, in.end(), oit, out.end() ) ) {
        qDebug() << "output buffer too small";
        return;
      }
      qDebug() << "Size of encoded data:" << oit - out.begin();
      </pre>

      @param scursor is a pointer to the start of the input buffer.
      @param send is a pointer to the end of the input buffer.
      @param dcursor is a pointer to the start of the output buffer.
      @param dend is a pointer to the end of the output buffer.
      @param withCRLF if true, make the newlines @ref CRLF; else use @ref LF.

      @return false if the encoded data didn't fit into the output buffer;
      true otherwise.
    */
    virtual bool encode( const char* &scursor, const char * const send,
                         char* &dcursor, const char * const dend,
                         bool withCRLF=false ) const;

    /**
      Convenience wrapper that can be used for small chunks of data
      when you can provide a large enough buffer. The default
      implementation creates a Decoder and uses it.

      Decodes a chunk of bytes starting at @p scursor and extending to
      @p send into the buffer described by @p dcursor and @p dend.

      This function doesn't support chaining of blocks. The returned
      block cannot be added to, but you don't need to finalize it, too.

      Example usage (@p in contains the input data):
      <pre>
      KMime::Codec *codec = KMime::Codec::codecForName( "base64" );
      kFatal( !codec ) << "no base64 codec found!?";
      QByteArray out( in.size() ); // good guess for any encoding...
      QByteArray::Iterator iit = in.begin();
      QByteArray::Iterator oit = out.begin();
      if ( !codec->decode( iit, in.end(), oit, out.end() ) ) {
        qDebug() << "output buffer too small";
        return;
      }
      qDebug() << "Size of decoded data:" << oit - out.begin();
      </pre>

      @param scursor is a pointer to the start of the input buffer.
      @param send is a pointer to the end of the input buffer.
      @param dcursor is a pointer to the start of the output buffer.
      @param dend is a pointer to the end of the output buffer.
      @param withCRLF if true, make the newlines @ref CRLF; else use @ref LF.

      @return false if the decoded data didn't fit into the output buffer;
      true otherwise.
    */
    virtual bool decode( const char* &scursor, const char * const send,
                         char* &dcursor, const char * const dend,
                         bool withCRLF=false ) const;

    /**
      Even more convenient, but also a bit slower and more memory
      intensive, since it allocates storage for the worst case and then
      shrinks the result QByteArray to the actual size again.

      For use with small @p src.

      @param src is a QByteArray containing the data to encode.
      @param withCRLF if true, make the newlines @ref CRLF; else use @ref LF.
    */
    virtual QByteArray encode( const QByteArray &src, bool withCRLF=false ) const;

    /**
      Even more convenient, but also a bit slower and more memory
      intensive, since it allocates storage for the worst case and then
      shrinks the result QByteArray to the actual size again.

      For use with small @p src.

      @param src is a QByteArray containing the data to decode.
      @param withCRLF if true, make the newlines @ref CRLF; else use @ref LF.
    */
    virtual QByteArray decode( const QByteArray &src, bool withCRLF=false ) const;

    /**
      Returns the name of the encoding. Guaranteed to be lowercase.
    */
    virtual const char *name() const = 0;

    /**
      Destroys the codec.
    */
    virtual ~Codec() {}

  private:
    /**
      Fills the KAutoDeleteHash with all the supported codecs.
    */
    static void fillDictionary();
};

/**
  @brief Stateful CTE decoder class

  Stateful decoder class, modelled after QTextDecoder.

  @section Overview

  KMime decoders are designed to be able to process encoded data in
  chunks of arbitrary size and to work with output buffers of also
  arbitrary size. They maintain any state necessary to go on where
  the previous call left off.

  The class consists of only two methods of interest: see decode,
  which decodes an input block and finalize, which flushes any
  remaining data to the output stream.

  Typically, you will create a decoder instance, call decode as
  often as necessary, then call finalize (most often a single
  call suffices, but it might be that during that call the output
  buffer is filled, so you should be prepared to call finalize
  as often as necessary, ie. until it returns @p true).

  @section Return Values

  Both methods return @p true to indicate that they've finished their
  job. For decode, a return value of @p true means that the
  current input block has been finished (@p false most often means
  that the output buffer is full, but that isn't required
  behavior. The decode call is free to return at arbitrary
  times during processing).

  For finalize, a return value of @p true means that all data
  implicitly or explicitly stored in the decoder instance has been
  flushed to the output buffer. A @p false return value should be
  interpreted as "check if the output buffer is full and call me
  again", just as with decode.

  @section Usage Pattern

  Since the decoder maintains state, you can only use it once. After
  a sequence of input blocks has been processed, you finalize
  the output and then delete the decoder instance. If you want to
  process another input block sequence, you create a new instance.

  Typical usage (@p in contains the (base64-encoded) input data),
  taking into account all the conventions detailed above:

  <pre>
  KMime::Codec *codec = KMime::Codec::codecForName( "base64" );
  kFatal( !codec ) << "No codec found for base64!";
  KMime::Decoder *dec = codec->makeDecoder();
  assert( dec ); // should not happen
  QByteArray out( 256 ); // small buffer is enough ;-)
  QByteArray::Iterator iit = in.begin();
  QByteArray::Iterator oit = out.begin();
  // decode the chunk
  while ( !dec->decode( iit, in.end(), oit, out.end() ) )
    if ( oit == out.end() ) { // output buffer full, process contents
      do_something_with( out );
      oit = out.begin();
    }
  // repeat while loop for each input block
  // ...
  // finish (flush remaining data from decoder):
  while ( !dec->finish( oit, out.end() ) )
    if ( oit == out.end() ) { // output buffer full, process contents
      do_something_with( out );
      oit = out.begin();
    }
  // now process last chunk:
  out.resize( oit - out.begin() );
  do_something_with( out );
  // _delete_ the decoder, but not the codec:
  delete dec;
  </pre>
*/
class Decoder
{
  protected:
    friend class Codec;
    /**
      Protected constructor. Use KMime::Codec::makeDecoder to create an
      instance.

      @param withCRLF if true, make the newlines @ref CRLF; else use @ref LF.
    */
    Decoder( bool withCRLF=false )
      : mWithCRLF( withCRLF ) {}

  public:
    /**
      Destroys the decoder.
    */
    virtual ~Decoder() {}

    /**
      Decodes a chunk of data, maintaining state information between
      calls. See class decumentation for calling conventions.

      @param scursor is a pointer to the start of the input buffer.
      @param send is a pointer to the end of the input buffer.
      @param dcursor is a pointer to the start of the output buffer.
      @param dend is a pointer to the end of the output buffer.
    */
    virtual bool decode( const char* &scursor, const char * const send,
                         char* &dcursor, const char * const dend ) = 0;

    /**
      Call this method to finalize the output stream. Writes all
      remaining data and resets the decoder. See KMime::Codec for
      calling conventions.

      @param dcursor is a pointer to the start of the output buffer.
      @param dend is a pointer to the end of the output buffer.
    */
    virtual bool finish( char* &dcursor, const char * const dend ) = 0;

  protected:
    //@cond PRIVATE
    const bool mWithCRLF;
    //@endcond
};

/**
  @brief
  Stateful encoder class.

  Stateful encoder class, modeled after QTextEncoder.
*/
class Encoder
{
  protected:
    friend class Codec;
    /**
      Protected constructor. Use KMime::Codec::makeEncoder if you want one.

      @param withCRLF if true, make the newlines @ref CRLF; else use @ref LF.
    */
    explicit Encoder( bool withCRLF=false )
      : mOutputBufferCursor( 0 ), mWithCRLF( withCRLF ) {}

  public:
    /**
      Destroys the encoder.
    */
    virtual ~Encoder() {}

    /**
      Encodes a chunk of data, maintaining state information between
      calls. See KMime::Codec for calling conventions.

      @param scursor is a pointer to the start of the input buffer.
      @param send is a pointer to the end of the input buffer.
      @param dcursor is a pointer to the start of the output buffer.
      @param dend is a pointer to the end of the output buffer.
    */
    virtual bool encode( const char* &scursor, const char * const send,
                         char* &dcursor, const char * const dend ) = 0;

    /**
      Call this method to finalize the output stream. Writes all remaining
      data and resets the encoder. See KMime::Codec for calling conventions.

      @param dcursor is a pointer to the start of the output buffer.
      @param dend is a pointer to the end of the output buffer.
    */
    virtual bool finish( char* &dcursor, const char * const dend ) = 0;

  protected:
    /**
      The maximum number of characters permitted in the output buffer.
    */
    enum {
      maxBufferedChars = 8  /**< Eight */
    };

    /**
      Writes character @p ch to the output stream or the output buffer,
      depending on whether or not the output stream has space left.

      @param ch is the character to write.
      @param dcursor is a pointer to the start of the output buffer.
      @param dend is a pointer to the end of the output buffer.

      @return true if written to the output stream; else false if buffered.
    */
    bool write( char ch, char* &dcursor, const char * const dend )
      {
        if ( dcursor != dend ) {
          // if there's space in the output stream, write there:
          *dcursor++ = ch;
          return true;
        } else {
          // else buffer the output:
          if ( mOutputBufferCursor >= maxBufferedChars ) {
             qCritical()
               << "KMime::Encoder: internal buffer overflow!";
          }
          mOutputBuffer[ mOutputBufferCursor++ ] = ch;
          return false;
        }
      }

    /**
      Writes characters from the output buffer to the output stream.
      Implementations of encode and finish should call this
      at the very beginning and for each iteration of the while loop.

      @param dcursor is a pointer to the start of the output buffer.
      @param dend is a pointer to the end of the output buffer.

      @return true if all chars could be written, false otherwise
    */
    bool flushOutputBuffer( char* &dcursor, const char * const dend );

    /**
      Convenience function. Outputs @ref LF or @ref CRLF, based on the
      state of mWithCRLF.

      @param dcursor is a pointer to the start of the output buffer.
      @param dend is a pointer to the end of the output buffer.
    */
    bool writeCRLF( char* &dcursor, const char * const dend )
      {
        if ( mWithCRLF ) {
          write( '\r', dcursor, dend );
        }
        return write( '\n', dcursor, dend );
      }

  private:
    /**
      An output buffer to simplify some codecs.
      Used with write() and flushOutputBuffer().
    */
    //@cond PRIVATE
    char mOutputBuffer[ maxBufferedChars ];
    //@endcond

  protected:
    //@cond PRIVATE
    uchar mOutputBufferCursor;
    const bool mWithCRLF;
    //@endcond
};

} // namespace KMime

#endif // __KMIME_CODECS__
