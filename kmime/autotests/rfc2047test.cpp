/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qtest.h>

#include "rfc2047test.h"

#include <kmime_util.h>
using namespace KMime;

QTEST_MAIN( RFC2047Test )

void RFC2047Test::testRFC2047decode()
{
  QByteArray encCharset;
  // empty
  QCOMPARE( KMime::decodeRFC2047String( QByteArray(), encCharset, "utf-8", false ), QString() );
  // identity
  QCOMPARE( KMime::decodeRFC2047String( "bla", encCharset, "utf-8", false ), QString( "bla" ) );
  // utf-8
  QCOMPARE( KMime::decodeRFC2047String( "=?utf-8?q?Ingo=20Kl=C3=B6cker?= <kloecker@kde.org>", encCharset, "utf-8", false ),
            QString::fromUtf8( "Ingo Klöcker <kloecker@kde.org>" ) );
  QCOMPARE( KMime::decodeRFC2047String( "=?utf-8?q?Ingo=20Kl=C3=B6cker?= <kloecker@kde.org>", encCharset, "iso8859-1", false ),
            QString::fromUtf8( "Ingo Klöcker <kloecker@kde.org>" ) );
  QCOMPARE( KMime::decodeRFC2047String( "=?utf-8?q?Ingo=20Kl=C3=B6cker?=", encCharset, "utf-8", false ),
            QString::fromUtf8( "Ingo Klöcker" ) );
  QCOMPARE( encCharset, QByteArray( "UTF-8" ) );

  // whitespaces between two encoded words
  QCOMPARE( KMime::decodeRFC2047String( "=?utf-8?q?Ingo=20Kl=C3=B6cker?=       =?utf-8?q?Ingo=20Kl=C3=B6cker?=", encCharset, "utf-8", false ),
            QString::fromUtf8( "Ingo KlöckerIngo Klöcker" ) );
  QCOMPARE( decodeRFC2047String( "=?utf-8?q?Ingo=20Kl=C3=B6cker?=  foo  =?utf-8?q?Ingo=20Kl=C3=B6cker?=", encCharset ),
            QString::fromUtf8( "Ingo Klöcker  foo  Ingo Klöcker" ) );

  // iso-8859-x
  QCOMPARE( KMime::decodeRFC2047String( "=?ISO-8859-1?Q?Andr=E9s_Ot=F3n?=", encCharset, "utf-8", false ),
            QString::fromUtf8( "Andrés Otón" ) );
  QCOMPARE( encCharset, QByteArray( "ISO-8859-1" ) );
  QCOMPARE( KMime::decodeRFC2047String( "=?iso-8859-2?q?Rafa=B3_Rzepecki?=", encCharset, "utf-8", false ),
            QString::fromUtf8( "Rafał Rzepecki" ) );
  QCOMPARE( encCharset, QByteArray( "ISO-8859-2" ) );
  QCOMPARE( KMime::decodeRFC2047String( "=?iso-8859-9?Q?S=2E=C7a=F0lar?= Onur", encCharset, "utf-8", false ),
            QString::fromUtf8( "S.Çağlar Onur" ) );
  QCOMPARE( encCharset, QByteArray( "ISO-8859-9" ) );
  QCOMPARE( KMime::decodeRFC2047String( "Rafael =?iso-8859-15?q?Rodr=EDguez?=", encCharset, "utf-8", false ),
            QString::fromUtf8( "Rafael Rodríguez" ) );
  QCOMPARE( encCharset, QByteArray( "ISO-8859-15" ) );

  // wrong charset + charset overwrite
  QCOMPARE( KMime::decodeRFC2047String( "=?iso-8859-1?q?Ingo=20Kl=C3=B6cker?=", encCharset, "utf-8", true ),
            QString::fromUtf8( "Ingo Klöcker" ) );

  // language parameter according to RFC 2231, section 5
  QCOMPARE( decodeRFC2047String( "From: =?US-ASCII*EN?Q?Keith_Moore?= <moore@cs.utk.edu>", encCharset ),
            QString::fromUtf8( "From: Keith Moore <moore@cs.utk.edu>" ) );
  QCOMPARE( encCharset, QByteArray( "US-ASCII" ) );

  // broken qp endoding (using lowercase)
  QCOMPARE( decodeRFC2047String( "Subject: =?iso-8859-1?Q?Belangrijk=3a=20Verhuizing=20FTP=20server?=", encCharset ),
            QString::fromUtf8( "Subject: Belangrijk: Verhuizing FTP server" ) );
  QCOMPARE( encCharset, QByteArray( "ISO-8859-1" ) );

  // mixed charsets, based on bug 125542 but pasted from above instead since I'm unable to enter those asian symbols
  QCOMPARE( decodeRFC2047String( "Subject: =?utf-8?q?Ingo=20Kl=C3=B6cker?= unencoded words =?iso-8859-9?Q?S=2E=C7a=F0lar?=", encCharset ),
            QString::fromUtf8( "Subject: Ingo Klöcker unencoded words S.Çağlar" ) );
  QCOMPARE( encCharset, QByteArray( "ISO-8859-9" ) );

  // illegal characters which are already encoded in the given encoding but are not ASCII (bug 206417)
  QCOMPARE( decodeRFC2047String( "Subject: =?utf-8?Q?пиѿилл,=20=D0=B4=D0=BE=D0=B1=D1=80=D1=8B=D0=B9=20=D0=B4=D0=B5=D0=BD=D1=8C?=", encCharset ),
            QString::fromUtf8( "Subject: пиѿилл, добрый день" ) );
  QCOMPARE( decodeRFC2047String( "Subject: =?iso-8859-1?Q?������?=" ),
            QString::fromLatin1( "Subject: ������" ) );

  // Small data
  QCOMPARE( decodeRFC2047String( "=?iso-8859-1?Q?c?=", encCharset ), QString::fromUtf8( "c" ) );
}

void RFC2047Test::testInvalidDecode()
{
  QByteArray encCharset;

  // invalid / incomplete encoded data
  QCOMPARE( decodeRFC2047String( "=", encCharset ), QString::fromUtf8( "=" ) );
  QCOMPARE( decodeRFC2047String( "=?", encCharset ), QString::fromUtf8( "=?" ) );
  QCOMPARE( decodeRFC2047String( "=?a?b?=", encCharset ), QString::fromUtf8( "=?a?b?=" ) );
  QCOMPARE( decodeRFC2047String( "=?a?b?c?", encCharset ), QString::fromUtf8( "=?a?b?c?" ) );
  QCOMPARE( decodeRFC2047String( "=?a??c?=", encCharset ), QString::fromUtf8( "=?a??c?=" ) );
}

void RFC2047Test::testRFC2047encode()
{
  // empty
  QCOMPARE( KMime::encodeRFC2047String( QString(), "utf-8" ), QByteArray() );
  // identity
  QCOMPARE( KMime::encodeRFC2047String( "bla", "utf-8" ), QByteArray( "bla" ) );
  // utf-8
  // expected value is probably wrong, libkmime will chose 'B' instead of 'Q' encoding
  QEXPECT_FAIL( "", "libkmime will chose 'B' instead of 'Q' encoding", Continue );
  QCOMPARE( KMime::encodeRFC2047String( QString::fromUtf8( "Ingo Klöcker <kloecker@kde.org>" ), "utf-8" ).constData(),
            "=?utf-8?q?Ingo=20Kl=C3=B6cker?= <kloecker@kde.org>" );

  // Fallback to UTF-8 for encoding since the given charset can't encode the string
  const QString input = QString::fromUtf8( "æſðđŋħł" );
  const QByteArray result = KMime::encodeRFC2047String( input, "latin1" );
  QCOMPARE( decodeRFC2047String( result ), input );
  QVERIFY( result.contains( "utf-8" ) );
}
