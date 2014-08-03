/*  -*- c++ -*-
    kmime_codec_identity.h

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2004 Marc Mutz <mutz@kde.org>

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
  defines the Identity, @ref seven-bit-text, @ref eight-bit-text,
  and @ref eight-bit-binary @ref Codec classes.

  @brief
  Defines the classes IdentityCodec, SevenBitCodec, EightBitCodec,
  and BinaryCodec.

  @authors Marc Mutz \<mutz@kde.org\>
*/

#ifndef __KMIME_CODEC_IDENTITY_H__
#define __KMIME_CODEC_IDENTITY_H__

#include "kmime_codecs.h"

class QByteArray;

namespace KMime
{

/**
  @brief
  A class representing the Identify @ref codec.
*/
class KMIME_EXPORT IdentityCodec : public Codec
{
protected:
    friend class Codec;
    /**
      Constructs the Identity codec.
    */
    IdentityCodec() : Codec() {}

public:
    /**
      Destroys the codec.
    */
    ~IdentityCodec() {}

    using Codec::encode;
    using Codec::decode;

    /**
      @copydoc
      QByteArray Codec::encode()
    */
    QByteArray encode(const QByteArray &src, bool withCRLF = false) const;

    /**
      @copydoc
      QByteArray Codec::decode()
    */
    QByteArray decode(const QByteArray &src, bool withCRLF = false) const;

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    int maxEncodedSizeFor(int insize, bool withCRLF) const
    {
        if (withCRLF) {
            return 2 * insize;
        } else {
            return insize;
        }
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    int maxDecodedSizeFor(int insize, bool withCRLF) const
    {
        if (withCRLF) {
            return 2 * insize;
        } else {
            return insize;
        }
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
  A class representing the @ref codec for @ref seven-bit-text.
*/
class KMIME_EXPORT SevenBitCodec : public IdentityCodec
{
protected:
    friend class Codec;
    /**
      Constructs the 7-bit codec.
    */
    SevenBitCodec() : IdentityCodec() {}

public:
    /**
      Destroys the codec.
    */
    ~SevenBitCodec() {}

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const
    {
        return "7bit";
    }
};

/**
  @brief
  A class representing the @ref codec for @ref eight-bit-text.
*/
class KMIME_EXPORT EightBitCodec : public IdentityCodec
{
protected:
    friend class Codec;
    /**
      Constructs the 8-bit codec.
    */
    EightBitCodec() : IdentityCodec() {}

public:
    /**
      Destroys the codec.
    */
    ~EightBitCodec() {}

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const
    {
        return "8bit";
    }
};

/**
  @brief
  A class representing the @ref codec for @ref eight-bit-binary.
*/
class KMIME_EXPORT BinaryCodec : public IdentityCodec
{
protected:
    friend class Codec;
    /**
      Constructs the 8-bit-binary codec.
    */
    BinaryCodec() : IdentityCodec() {}

public:
    /**
      Destroys the codec.
    */
    ~BinaryCodec() {}

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const
    {
        return "binary";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    int maxEncodedSizeFor(int insize, bool withCRLF = false) const
    {
        Q_UNUSED(withCRLF);
        return insize;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    int maxDecodedSizeFor(int insize, bool withCRLF = false) const
    {
        Q_UNUSED(withCRLF);
        return insize;
    }
};

} // namespace KMime

#endif // __KMIME_CODEC_IDENTITY_H__
