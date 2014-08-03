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
  This file is part of the API for handling MIME data and
  defines the Codec class.

  @brief
  Defines the Codec class.

  @authors Marc Mutz \<mutz@kde.org\>
*/

#include "kmime_codecs.h"
#include "kmime_util.h"
#include "kmime_codec_base64.h"
#include "kmime_codec_qp.h"
#include "kmime_codec_uuencode.h"
#include "kmime_codec_identity.h"

#include "kautodeletehash.h"

#include <qdebug.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QMutex>

#include <cassert>
#include <cstring>
#include <string.h>

using namespace KMime;

namespace KMime
{

// global list of KMime::Codec's
//@cond PRIVATE
KAutoDeleteHash<QByteArray, Codec> *Codec::all = 0;
Q_GLOBAL_STATIC(QMutex, dictLock)
//@endcond

void Codec::cleanupCodec()
{
    delete all;
    all = 0;
}

void Codec::fillDictionary()
{
    //all->insert( "7bit", new SevenBitCodec() );
    //all->insert( "8bit", new EightBitCodec() );
    all->insert("base64", new Base64Codec());
    all->insert("quoted-printable", new QuotedPrintableCodec());
    all->insert("b", new Rfc2047BEncodingCodec());
    all->insert("q", new Rfc2047QEncodingCodec());
    all->insert("x-kmime-rfc2231", new Rfc2231EncodingCodec());
    all->insert("x-uuencode", new UUCodec());
    //all->insert( "binary", new BinaryCodec() );
}

Codec *Codec::codecForName(const char *name)
{
    const QByteArray ba(name);
    return codecForName(ba);
}

Codec *Codec::codecForName(const QByteArray &name)
{
    dictLock->lock(); // protect "all"
    if (!all) {
        all = new KAutoDeleteHash<QByteArray, Codec>();
        qAddPostRoutine(cleanupCodec);
        fillDictionary();
    }
    QByteArray lowerName = name.toLower();
    Codec *codec = (*all)[ lowerName ];   // FIXME: operator[] adds an entry into the hash
    dictLock->unlock();

    if (!codec) {
        qDebug() << "Unknown codec \"" << name << "\" requested!";
    }

    return codec;
}

bool Codec::encode(const char *&scursor, const char *const send,
                   char *&dcursor, const char *const dend,
                   bool withCRLF) const
{
    // get an encoder:
    Encoder *enc = makeEncoder(withCRLF);
    assert(enc);

    // encode and check for output buffer overflow:
    while (!enc->encode(scursor, send, dcursor, dend)) {
        if (dcursor == dend) {
            delete enc;
            return false; // not enough space in output buffer
        }
    }

    // finish and check for output buffer overflow:
    while (!enc->finish(dcursor, dend)) {
        if (dcursor == dend) {
            delete enc;
            return false; // not enough space in output buffer
        }
    }

    // cleanup and return:
    delete enc;
    return true; // successfully encoded.
}

QByteArray Codec::encode(const QByteArray &src, bool withCRLF) const
{
    // allocate buffer for the worst case:
    QByteArray result;
    result.resize(maxEncodedSizeFor(src.size(), withCRLF));

    // set up iterators:
    QByteArray::ConstIterator iit = src.begin();
    QByteArray::ConstIterator iend = src.end();
    QByteArray::Iterator oit = result.begin();
    QByteArray::ConstIterator oend = result.end();

    // encode
    if (!encode(iit, iend, oit, oend, withCRLF)) {
        qCritical() << name() << "codec lies about it's mEncodedSizeFor()";
    }

    // shrink result to actual size:
    result.truncate(oit - result.begin());

    return result;
}

QByteArray Codec::decode(const QByteArray &src, bool withCRLF) const
{
    // allocate buffer for the worst case:
    QByteArray result;
    result.resize(maxDecodedSizeFor(src.size(), withCRLF));

    // set up iterators:
    QByteArray::ConstIterator iit = src.begin();
    QByteArray::ConstIterator iend = src.end();
    QByteArray::Iterator oit = result.begin();
    QByteArray::ConstIterator oend = result.end();

    // decode
    if (!decode(iit, iend, oit, oend, withCRLF)) {
        qCritical() << name() << "codec lies about it's maxDecodedSizeFor()";
    }

    // shrink result to actual size:
    result.truncate(oit - result.begin());

    return result;
}

bool Codec::decode(const char *&scursor, const char *const send,
                   char *&dcursor, const char *const dend,
                   bool withCRLF) const
{
    // get a decoder:
    Decoder *dec = makeDecoder(withCRLF);
    assert(dec);

    // decode and check for output buffer overflow:
    while (!dec->decode(scursor, send, dcursor, dend)) {
        if (dcursor == dend) {
            delete dec;
            return false; // not enough space in output buffer
        }
    }

    // finish and check for output buffer overflow:
    while (!dec->finish(dcursor, dend)) {
        if (dcursor == dend) {
            delete dec;
            return false; // not enough space in output buffer
        }
    }

    // cleanup and return:
    delete dec;
    return true; // successfully encoded.
}

// write as much as possible off the output buffer. Return true if
// flushing was complete, false if some chars could not be flushed.
bool Encoder::flushOutputBuffer(char *&dcursor, const char *const dend)
{
    int i;
    // copy output buffer to output stream:
    for (i = 0 ; dcursor != dend && i < mOutputBufferCursor ; ++i) {
        *dcursor++ = mOutputBuffer[i];
    }

    // calculate the number of missing chars:
    int numCharsLeft = mOutputBufferCursor - i;
    // push the remaining chars to the begin of the buffer:
    if (numCharsLeft) {
        ::memmove(mOutputBuffer, mOutputBuffer + i, numCharsLeft);
    }
    // adjust cursor:
    mOutputBufferCursor = numCharsLeft;

    return !numCharsLeft;
}

} // namespace KMime
