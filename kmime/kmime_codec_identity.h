/*  -*- c++ -*-
    kmime_codec_identity.h

    This file is part of KMime, the KDE internet mail/usenet news message library.
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

#ifndef __KMIME_CODEC_IDENTITY_H__
#define __KMIME_CODEC_IDENTITY_H__

#include "kmime_codecs.h"

//#include <QByteArray>

class QByteArray;

namespace KMime {

class KMIME_EXPORT IdentityCodec : public Codec {
protected:
  friend class Codec;
  IdentityCodec() : Codec() {}

public:
  ~IdentityCodec() {}

  QByteArray encode( const QByteArray & src, bool withCRLF ) const;
  QByteArray decode( const QByteArray & src, bool withCRLF ) const;

  int maxEncodedSizeFor( int insize, bool withCRLF ) const {
    if ( withCRLF )
      return 2 * insize;
    else
      return insize;
  }

  int maxDecodedSizeFor( int insize, bool withCRLF ) const {
    if ( withCRLF )
      return 2 * insize;
    else
      return insize;
  }

  Encoder * makeEncoder( bool withCRLF=false ) const;
  Decoder * makeDecoder( bool withCRLF=false ) const;
};

class KMIME_EXPORT SevenBitCodec : public IdentityCodec {
protected:
  friend class Codec;
  SevenBitCodec() : IdentityCodec() {}

public:
  ~SevenBitCodec() {}

  const char * name() const { return "7bit"; }
};

class KMIME_EXPORT EightBitCodec : public IdentityCodec {
protected:
  friend class Codec;
  EightBitCodec() : IdentityCodec() {}

public:
  ~EightBitCodec() {}

  const char * name() const { return "8bit"; }
};

class KMIME_EXPORT BinaryCodec : public IdentityCodec {
protected:
  friend class Codec;
  BinaryCodec() : IdentityCodec() {}

public:
  ~BinaryCodec() {}

  const char * name() const { return "binary"; }

  int maxEncodedSizeFor( int insize, bool ) const {
    return insize;
  }
  int maxDecodedSizeFor( int insize, bool ) const {
    return insize;
  }
};

} // namespace KMime

#endif // __KMIME_CODEC_IDENTITY_H__
