/*  -*- c++ -*-
    kmime_codec_identity.cpp

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

#include "kmime_codec_identity.h"

#include <qdebug.h>

#include <QtCore/QByteArray>

#include <cassert>
#include <cstring>

using namespace KMime;

namespace KMime {

class IdentityEnDecoder : public Encoder, public Decoder
{
  protected:
    friend class IdentityCodec;
    IdentityEnDecoder( bool withCRLF ): Encoder( false )
    {
      if (withCRLF )
         qWarning( ) << "IdentityEnDecoder: withCRLF isn't yet supported!";
    }

  public:
    ~IdentityEnDecoder() {}

    bool encode( const char* &scursor, const char *const send,
                 char* &dcursor, const char *const dend )
    { return decode( scursor, send, dcursor, dend ); }

    bool decode( const char* &scursor, const char *const send,
                 char* &dcursor, const char *const dend );

    bool finish( char* &dcursor, const char *const dend )
    { Q_UNUSED( dcursor ); Q_UNUSED( dend ); return true; }
};

Encoder *IdentityCodec::makeEncoder( bool withCRLF ) const
{
  return new IdentityEnDecoder( withCRLF );
}

Decoder *IdentityCodec::makeDecoder( bool withCRLF ) const
{
  return new IdentityEnDecoder( withCRLF );
}

/********************************************************/
/********************************************************/
/********************************************************/

bool IdentityEnDecoder::decode( const char* &scursor, const char *const send,
                                char* &dcursor, const char *const dend )
{
  const int size = qMin( send - scursor, dcursor - dend );
  if ( size > 0 ) {
    std::memmove( dcursor, scursor, size );
    dcursor += size;
    scursor += size;
  }
  return scursor == send;
}

QByteArray IdentityCodec::encode( const QByteArray &src, bool withCRLF ) const
{
  if (withCRLF)
     qWarning() << "IdentityCodec::encode(): withCRLF not yet supported!";
  return src;
}

QByteArray IdentityCodec::decode( const QByteArray &src, bool withCRLF ) const
{
  if (withCRLF)
    qWarning() << "IdentityCodec::decode(): withCRLF not yet supported!";
  return src;
}

} // namespace KMime
