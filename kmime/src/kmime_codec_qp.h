/*  -*- c++ -*-
    kmime_codec_qp.h

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
  defines the @ref QuotedPrintable, @ref  RFC2047Q, and
  @ref RFC2231 @ref Codec classes.

  @brief
  Defines the classes QuotedPrintableCodec, Rfc2047QEncodingCodec, and
  Rfc2231EncodingCodec.

  @authors Marc Mutz \<mutz@kde.org\>

  @glossary @anchor QuotedPrintable @anchor quotedprintable @b quoted-printable:
  a binary to text encoding scheme based on Section 6.7 of @ref RFC2045.

  @glossary @anchor RFC2047Q @anchor rfc2047q @b RFC @b 2047Q:
  Section 4.2 of @ref RFC2047.

  @glossary @anchor RFC2231 @anchor rfc2231 @b RFC @b 2231:
  RFC that defines the <a href="http://tools.ietf.org/html/rfc2231">
  MIME Parameter Value and Encoded Word Extensions: Character Sets, Languages,
  and Continuations</a>.
*/

#ifndef __KMIME_CODEC_QP__
#define __KMIME_CODEC_QP__

#include "kmime_codecs.h"

namespace KMime
{

/**
  @brief
  A class representing the @ref codec for @ref QuotedPrintable as specified in
  @ref RFC2045 (section 6.7).
*/
class KMIME_EXPORT QuotedPrintableCodec : public Codec
{
protected:
    friend class Codec;
    /**
      Constructs a QuotedPrintable codec.
    */
    QuotedPrintableCodec() : Codec() {}

public:
    /**
      Destroys the codec.
    */
    virtual ~QuotedPrintableCodec() {}

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const
    {
        return "quoted-printable";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    int maxEncodedSizeFor(int insize, bool withCRLF = false) const
    {
        // all chars encoded:
        int result = 3 * insize;
        // then after 25 hexchars comes a soft linebreak: =(\r)\n
        result += (withCRLF ? 3 : 2) * (insize / 25);

        return result;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    int maxDecodedSizeFor(int insize, bool withCRLF = false) const;

    /**
      @copydoc
      Codec::makeEncoder()
    */
    Encoder *makeEncoder(bool withCRLF = false) const;

    /**
      @copydoc
      Codec::makeDecoder()
    */
    Decoder *makeDecoder(bool withCRLF = false) const;
};

/**
  @brief
  A class representing the @ref codec for the Q encoding as specified
  in @ref RFC2047Q.
*/
class KMIME_EXPORT Rfc2047QEncodingCodec : public Codec
{
protected:
    friend class Codec;
    /**
      Constructs a RFC2047Q codec.
    */
    Rfc2047QEncodingCodec() : Codec() {}

public:
    /**
      Destroys the codec.
    */
    virtual ~Rfc2047QEncodingCodec() {}

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const
    {
        return "q";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    int maxEncodedSizeFor(int insize, bool withCRLF = false) const
    {
        Q_UNUSED(withCRLF);
        // this one is simple: We don't do linebreaking, so all that can
        // happen is that every char needs encoding, so:
        return 3 * insize;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    int maxDecodedSizeFor(int insize, bool withCRLF = false) const;

    /**
      @copydoc
      Codec::makeEncoder()
    */
    Encoder *makeEncoder(bool withCRLF = false) const;

    /**
      @copydoc
      Codec::makeDecoder()
    */
    Decoder *makeDecoder(bool withCRLF = false) const;
};

/**
  @brief
  A class representing the @ref codec for @ref RFC2231.
*/
class KMIME_EXPORT Rfc2231EncodingCodec : public Codec
{
protected:
    friend class Codec;
    /**
      Constructs a RFC2231 codec.
    */
    Rfc2231EncodingCodec() : Codec() {}

public:
    /**
      Destroys the codec.
    */
    virtual ~Rfc2231EncodingCodec() {}

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const
    {
        return "x-kmime-rfc2231";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    int maxEncodedSizeFor(int insize, bool withCRLF = false) const
    {
        Q_UNUSED(withCRLF);
        // same as for "q" encoding:
        return 3 * insize;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    int maxDecodedSizeFor(int insize, bool withCRLF = false) const;

    /**
      @copydoc
      Codec::makeEncoder()
    */
    Encoder *makeEncoder(bool withCRLF = false) const;

    /**
      @copydoc
      Codec::makeDecoder()
    */
    Decoder *makeDecoder(bool withCRLF = false) const;
};

} // namespace KMime

#endif // __KMIME_CODEC_QP__
