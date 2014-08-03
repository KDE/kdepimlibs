/*  -*- c++ -*-
    kmime_codec_uuencode.h

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

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
  defines a @ref uuencode @ref Codec class.

  @brief
  Defines the UUCodec class.

  @authors Marc Mutz \<mutz@kde.org\>

  @glossary @anchor UUEncode @anchor uuencode @b uuencode:
  a binary to text encoding scheme. For more information, see the
  <a href="http://en.wikipedia.org/wiki/Uuencode"> Wikipedia Uuencode page</a>.
*/

#ifndef __KMIME_CODEC_UUENCODE_H__
#define __KMIME_CODEC_UUENCODE_H__

#include "kmime_codecs.h"

namespace KMime
{

/**
  @brief
  A class representing the @ref UUEncode @ref codec.
*/
class KMIME_EXPORT UUCodec : public Codec
{
protected:
    friend class Codec;
    /**
      Constructs a UUEncode codec.
    */
    UUCodec() : Codec() {}

public:
    /**
      Destroys the codec.
    */
    virtual ~UUCodec() {}

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const
    {
        return "x-uuencode";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    int maxEncodedSizeFor(int insize, bool withCRLF = false) const
    {
        (void)withCRLF;
        return insize; // we have no encoder!
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    int maxDecodedSizeFor(int insize, bool withCRLF = false) const
    {
        // assuming all characters are part of the uuencode stream (which
        // does almost never hold due to required linebreaking; but
        // additional non-uu chars don't affect the output size), each
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
      Codec::makeEncoder()
    */
    Decoder *makeDecoder(bool withCRLF = false) const;
};

} // namespace KMime

#endif // __KMIME_CODEC_UUENCODE_H__
