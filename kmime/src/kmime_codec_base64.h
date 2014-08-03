/*  -*- c++ -*-
    kmime_codec_base64.h

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
  defines the @ref Base64 and @ref RFC2047B @ref Codec classes.

  @brief
  Defines the Base64Codec and Rfc2047BEncodingCodec classes.

  @authors Marc Mutz \<mutz@kde.org\>

  @glossary @anchor Base64 @anchor base64 @b base64:
  a binary to text encoding scheme based on @ref RFC1421.

  @glossary @anchor RFC1421 @anchor rfc1421 @b RFC @b 1421:
  RFC that defines the <a href="http://tools.ietf.org/html/rfc1421">
  Privacy Enhancement for Internet Electronic Mail:  Part I:
  Message Encryption and Authentication Procedures</a>.

  @glossary @anchor RFC2045 @anchor rfc2045 @b RFC @b 2045:
  RFC that defines the <a href="http://tools.ietf.org/html/rfc2045">
  MIME Part One: Format of Internet Message Bodies</a>.

  @glossary @anchor RFC2047 @anchor rfc2047 @b RFC @b 2047:
  RFC that defines the <a href="http://tools.ietf.org/html/rfc2047">
  MIME Part Three: Message Header Extensions for Non-ASCII Text</a>.

  @glossary @anchor RFC2047B @anchor rfc2047b @b RFC @b 2047B:
  Section 4.1 of @ref RFC2047.
*/

#ifndef __KMIME_CODEC_BASE64__
#define __KMIME_CODEC_BASE64__

#include "kmime_codecs.h"

namespace KMime
{

/**
  @brief
  A class representing the @ref codec for @ref Base64 as specified in
  @ref RFC2045
*/
class KMIME_EXPORT Base64Codec : public Codec
{
protected:
    friend class Codec;
    /**
      Constructs a Base64 codec.
    */
    Base64Codec() : Codec() {}

public:
    /**
      Destroys the codec.
    */
    virtual ~Base64Codec() {}

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const
    {
        return "base64";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    int maxEncodedSizeFor(int insize, bool withCRLF = false) const
    {
        // first, the total number of 4-char packets will be:
        int totalNumPackets = (insize + 2) / 3;
        // now, after every 76/4'th packet there needs to be a linebreak:
        int numLineBreaks = totalNumPackets / (76 / 4);
        // and at the very end, too:
        ++numLineBreaks;
        // putting it all together, we have:
        return 4 * totalNumPackets + (withCRLF ? 2 : 1) * numLineBreaks;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    int maxDecodedSizeFor(int insize, bool withCRLF = false) const
    {
        // assuming all characters are part of the base64 stream (which
        // does almost never hold due to required linebreaking; but
        // additional non-base64 chars don't affect the output size), each
        // 4-tupel of them becomes a 3-tupel in the decoded octet
        // stream. So:
        int result = ((insize + 3) / 4) * 3;
        // but all of them may be \n, so
        if (withCRLF) {
            result *= 2; // :-o
        }

        return result;
    }

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
  A class representing the @ref codec for the B encoding as specified
  in @ref RFC2047B.
*/
class KMIME_EXPORT Rfc2047BEncodingCodec : public Base64Codec
{
protected:
    friend class Codec;
    /**
      Constructs a RFC2047B codec.
    */
    Rfc2047BEncodingCodec() : Base64Codec() {}

public:
    /**
      Destroys the codec.
    */
    virtual ~Rfc2047BEncodingCodec() {}

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const
    {
        return "b";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    int maxEncodedSizeFor(int insize, bool withCRLF = false) const
    {
        Q_UNUSED(withCRLF);
        // Each (begun) 3-octet triple becomes a 4 char quartet, so:
        return ((insize + 2) / 3) * 4;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    int maxDecodedSizeFor(int insize, bool withCRLF = false) const
    {
        Q_UNUSED(withCRLF);
        // Each 4-char quartet becomes a 3-octet triple, the last one
        // possibly even less. So:
        return ((insize + 3) / 4) * 3;
    }

    /**
      @copydoc
      Codec::makeEncoder()
    */
    Encoder *makeEncoder(bool withCRLF = false) const;
};

} // namespace KMime

#endif // __KMIME_CODEC_BASE64__
