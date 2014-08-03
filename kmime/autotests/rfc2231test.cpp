/*
    Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

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

#include "rfc2231test.h"

#include <kmime_util.h>
#include <QDebug>
using namespace KMime;

QTEST_MAIN(RFC2231Test)

void RFC2231Test::testRFC2231decode()
{
    QByteArray encCharset;
    qDebug() << KMime::decodeRFC2231String(QByteArray(), encCharset, "utf-8", false);

/// empty
    QCOMPARE(KMime::decodeRFC2231String(QByteArray(), encCharset, "utf-8", false), QString());
    // identity
    QCOMPARE(KMime::decodeRFC2231String("bla", encCharset, "utf-8", false), QLatin1String("bla"));
    // utf-8
    QCOMPARE(KMime::decodeRFC2231String("utf-8''Ingo%20Kl%C3%B6cker <kloecker@kde.org>", encCharset, "utf-8", false),
             QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"));
    qDebug() << "Charset:" << encCharset;
    QCOMPARE(KMime::decodeRFC2231String("iso8859-1''Ingo%20Kl%C3%B6cker <kloecker@kde.org>", encCharset, "iso8859-1", false),
             QString::fromUtf8("Ingo KlÃ¶cker <kloecker@kde.org>"));
    QCOMPARE(KMime::decodeRFC2231String("utf-8''Ingo%20Kl%C3%B6cker", encCharset, "utf-8", false),
             QString::fromUtf8("Ingo Klöcker"));
    QCOMPARE(encCharset, QByteArray("UTF-8"));

    // whitespaces between two encoded words
    QCOMPARE(KMime::decodeRFC2231String("utf-8''Ingo%20Kl%C3%B6cker       Ingo%20Kl%C3%B6cker", encCharset, "utf-8", false),
             QString::fromUtf8("Ingo Klöcker       Ingo Klöcker"));
    QCOMPARE(decodeRFC2231String("utf-8''Ingo%20Kl%C3%B6cker  foo  Ingo%20Kl%C3%B6cker", encCharset),
             QString::fromUtf8("Ingo Klöcker  foo  Ingo Klöcker"));

    // iso-8859-x
    QCOMPARE(KMime::decodeRFC2231String("ISO-8859-1'Andr%E9s Ot%F3n", encCharset, "utf-8", false),
             QString::fromUtf8("Andrés Otón"));
    QCOMPARE(encCharset, QByteArray("ISO-8859-1"));
    /*
     * QCOMPARE( KMime::decodeRFC2231String( "iso-8859-2''Rafa%B3 Rzepecki", encCharset, "utf-8", false ),
                QString::fromUtf8( "RafaÅ Rzepecki" ) );
      QCOMPARE( encCharset, QByteArray( "ISO-8859-2" ) );
      QCOMPARE( KMime::decodeRFC2231String( "iso-8859-9''S%2E%C7a%F0lar Onur", encCharset, "utf-8", false ),
                QString::fromUtf8( "S.ÃaÄlar Onur" ) );
      QCOMPARE( encCharset, QByteArray( "ISO-8859-9" ) );
      QCOMPARE( KMime::decodeRFC2231String( "Rafael =?iso-8859-15?q?Rodr=EDguez?=", encCharset, "utf-8", false ),
                QString::fromUtf8( "Rafael RodrÃ­guez" ) );
      QCOMPARE( encCharset, QByteArray( "ISO-8859-15" ) );

      // wrong charset + charset overwrite
      QCOMPARE( KMime::decodeRFC2231String( "=?iso-8859-1?q?Ingo=20Kl=C3=B6cker?=", encCharset, "utf-8", true ),
                QString::fromUtf8( "Ingo KlÃ¶cker" ) );

      // language parameter according to RFC 2231, section 5
      QCOMPARE( decodeRFC2231String( "From: =?US-ASCII*EN?Q?Keith_Moore?= <moore@cs.utk.edu>", encCharset ),
                QString::fromUtf8( "From: Keith Moore <moore@cs.utk.edu>" ) );
      QCOMPARE( encCharset, QByteArray( "US-ASCII" ) );

      // broken qp endoding (using lowercase)
      QCOMPARE( decodeRFC2231String( "Subject: =?iso-8859-1?Q?Belangrijk=3a=20Verhuizing=20FTP=20server?=", encCharset ),
                QString::fromUtf8( "Subject: Belangrijk: Verhuizing FTP server" ) );
      QCOMPARE( encCharset, QByteArray( "ISO-8859-1" ) );

      // mixed charsets, based on bug 125542 but pasted from above instead since I'm unable to enter those asian symbols
      QCOMPARE( decodeRFC2231String( "Subject: =?utf-8?q?Ingo=20Kl=C3=B6cker?= unencoded words =?iso-8859-9?Q?S=2E=C7a=F0lar?=", encCharset ),
                QString::fromUtf8( "Subject: Ingo KlÃ¶cker unencoded words S.ÃaÄlar" ) );
      QCOMPARE( encCharset, QByteArray( "ISO-8859-9" ) );

      // illegal characters which are already encoded in the given encoding but are not ASCII (bug 206417)
      QCOMPARE( decodeRFC2231String( "Subject: =?utf-8?Q?Ð¿ÐžÑ¿ÐžÐ»Ð»,=20=D0=B4=D0=BE=D0=B1=D1=80=D1=8B=D0=B9=20=D0=B4=D0=B5=D0=BD=D1=8C?=", encCharset ),
                QString::fromUtf8( "Subject: Ð¿ÐžÑ¿ÐžÐ»Ð», ÐŽÐŸÐ±ÑÑÐ¹ ÐŽÐµÐœÑ" ) );
      QCOMPARE( decodeRFC2231String( "Subject: =?iso-8859-1?Q?ÖÄÜöäü?=" ),
                QString::fromLatin1( "Subject: ÖÄÜöäü" ) );

      // Small data
      QCOMPARE( decodeRFC2231String( "=?iso-8859-1?Q?c?=", encCharset ), QString::fromUtf8("c") ); */
}

void RFC2231Test::testInvalidDecode()
{
    /* QByteArray encCharset;

     // invalid / incomplete encoded data
     QCOMPARE( decodeRFC2231String( "=", encCharset ), QString::fromUtf8("=") );
     QCOMPARE( decodeRFC2231String( "=?", encCharset ), QString::fromUtf8("=?") );
     QCOMPARE( decodeRFC2231String( "=?a?b?=", encCharset ), QString::fromUtf8("=?a?b?=") );
     QCOMPARE( decodeRFC2231String( "=?a?b?c?", encCharset ), QString::fromUtf8("=?a?b?c?") );
     QCOMPARE( decodeRFC2231String( "=?a??c?=", encCharset ), QString::fromUtf8("=?a??c?=") ); */
}

void RFC2231Test::testRFC2231encode()
{
    // empty
    QCOMPARE(KMime::encodeRFC2047String(QString(), "utf-8"), QByteArray());
    // identity
    QCOMPARE(KMime::encodeRFC2047String(QLatin1String("bla"), "utf-8"), QByteArray("bla"));
    QCOMPARE(KMime::encodeRFC2231String(QString::fromUtf8("with accents Ã²Ã³Ã¨Ã©Ã¤Ã¯Ã±"), "utf-8").constData(),
             "utf-8''with%20accents%20%C3%83%C2%B2%C3%83%C2%B3%C3%83%C2%A8%C3%83%C2%A9%C3%83%C2%A4%C3%83%C2%AF%C3%83%C2%B1");
}
