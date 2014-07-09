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

#undef QT_USE_FAST_CONCATENATION
#undef QT_USE_FAST_OPERATOR_PLUS


#include <qtest.h>

#include "utiltest.h"

#include <kmime_util.h>
#include <kmime_message.h>

using namespace KMime;

QTEST_MAIN( UtilTest )

void UtilTest::testUnfoldHeader()
{
  // empty header
  QCOMPARE( KMime::unfoldHeader( "" ), QByteArray() );
  // identity
  QCOMPARE( KMime::unfoldHeader( "bla" ), QByteArray( "bla" ) );
  // single folding
  QCOMPARE( KMime::unfoldHeader( "bla\nblub" ), QByteArray( "bla blub" ) );
  QCOMPARE( KMime::unfoldHeader( "bla\n \t blub" ), QByteArray( "bla blub" ) );
  QCOMPARE( KMime::unfoldHeader( "bla   \r\nblub" ), QByteArray( "bla blub" ) );
  // multiple folding
  QCOMPARE( KMime::unfoldHeader( "bla\nbla\nblub" ), QByteArray( "bla bla blub" ) );
  QCOMPARE( KMime::unfoldHeader( "bla  \r\n   bla  \r\n  blub" ), QByteArray( "bla bla blub" ) );
  QCOMPARE( KMime::unfoldHeader( "bla\n" ), QByteArray( "bla" ) );
  // bug #86302 - malformed header continuation
  QCOMPARE( KMime::unfoldHeader( "bla\n=20bla" ), QByteArray( "bla bla" ) );
  QCOMPARE( KMime::unfoldHeader( "bla\n=09bla" ), QByteArray( "bla bla" ) );
  QCOMPARE( KMime::unfoldHeader( "bla\r\n=20bla" ), QByteArray( "bla bla" ) );
  QCOMPARE( KMime::unfoldHeader( "bla\r\n=09bla" ), QByteArray( "bla bla" ) );
  QCOMPARE( KMime::unfoldHeader( "bla \n=20 bla" ), QByteArray( "bla bla" ) );
  QCOMPARE( KMime::unfoldHeader( "bla \n=09 bla" ), QByteArray( "bla bla" ) );
  QCOMPARE( KMime::unfoldHeader( "bla \n =20 bla" ), QByteArray( "bla =20 bla" ) );
  QCOMPARE( KMime::unfoldHeader( "bla \n =09 bla" ), QByteArray( "bla =09 bla" ) );
}

void UtilTest::testExtractHeader()
{
  QByteArray header( "To: <foo@bla.org>\n"
                     "Subject: =?UTF-8?Q?_Notification_for_appointment:?=\n"
                     " =?UTF-8?Q?_Test?=\n"
                     "Continuation: =?UTF-8?Q?_TEST\n"
                     "=20CONT1?= =?UTF-8?Q?_TEST\n"
                     "=09CONT2?=\n"
                     "MIME-Version: 1.0" );

  // basic tests
  QVERIFY( extractHeader( header, "Foo" ).isEmpty() );
  QCOMPARE( extractHeader( header, "To" ), QByteArray( "<foo@bla.org>" ) );

  // case insensitive matching
  QCOMPARE( extractHeader( header, "mime-version" ), QByteArray( "1.0" ) );

  // extraction of multi-line headers
  QCOMPARE( extractHeader( header, "Subject" ),
            QByteArray( "=?UTF-8?Q?_Notification_for_appointment:?= =?UTF-8?Q?_Test?=" ) );

  // bug #86302 - malformed header continuation
  QCOMPARE( extractHeader( header, "Continuation" ),
            QByteArray( "=?UTF-8?Q?_TEST CONT1?= =?UTF-8?Q?_TEST CONT2?=" ) );

  // missing space after ':'
  QCOMPARE( extractHeader( "From:<toma@kovoks.nl>", "From" ), QByteArray( "<toma@kovoks.nl>" ) );
}

void UtilTest::testBalanceBidiState()
{
  QFETCH( QString, input );
  QFETCH( QString, expResult );

  QCOMPARE( balanceBidiState( input ), expResult );
}

void UtilTest::testBalanceBidiState_data()
{
  QTest::addColumn<QString>( "input" );
  QTest::addColumn<QString>( "expResult" );

  const QString LRO( QChar( 0x202D ) );
  const QString RLO( QChar( 0x202E ) );
  const QString LRE( QChar( 0x202A ) );
  const QString RLE( QChar( 0x202B ) );
  const QString PDF( QChar( 0x202C ) );

  QTest::newRow( "" ) << "Normal" << "Normal";
  QTest::newRow( "" ) << RLO + "Balanced" + PDF << RLO + "Balanced" + PDF;
  QTest::newRow( "" ) << RLO + "MissingPDF1" << RLO + "MissingPDF1" + PDF;
  QTest::newRow( "" ) << "\"" + RLO + "Quote\"" << "\"" + RLO + "Quote" + PDF + "\"";
  QTest::newRow( "" ) << "MissingPDF2" + RLO << "MissingPDF2" + RLO + PDF;
  QTest::newRow( "" ) << RLO + "MultipleRLO" + RLO << RLO + "MultipleRLO" + RLO + PDF + PDF;
  QTest::newRow( "" ) << LRO + "Mixed" + LRE + RLE + RLO + "Bla"
                      << LRO + "Mixed" + LRE + RLE + RLO + "Bla" + PDF.repeated( 4 );
  QTest::newRow( "" ) << RLO + "TooManyPDF" + PDF + RLO + PDF + PDF
                      << RLO + "TooManyPDF" + PDF + RLO + PDF;
  QTest::newRow( "" ) << PDF + "WrongOrder" + RLO
                      << "WrongOrder" + RLO + PDF;
  QTest::newRow( "" ) << "ComplexOrder" + RLO + PDF + PDF + RLO
                      << "ComplexOrder" + RLO + PDF + RLO + PDF;
  QTest::newRow( "" ) << "ComplexOrder2" + RLO + PDF + PDF + PDF + RLO + PDF + PDF + PDF
                      << "ComplexOrder2" + RLO + PDF + RLO + PDF;
  QTest::newRow( "" ) << PDF + PDF + PDF + "ComplexOrder3" + PDF + PDF + RLO + PDF + PDF + PDF
                      << "ComplexOrder3" + RLO + PDF;
}

void UtilTest::testAddQuotes()
{
  QFETCH( QByteArray, input );
  QFETCH( QByteArray, expResult );
  QFETCH( bool, forceQuotes );

  addQuotes( input, forceQuotes );
  QCOMPARE( input.data(), expResult.data() );
}

void UtilTest::testAddQuotes_data()
{
  QTest::addColumn<QByteArray>( "input" );
  QTest::addColumn<QByteArray>( "expResult" );
  QTest::addColumn<bool>( "forceQuotes" );

  QTest::newRow( "" ) << QByteArray( "Test" ) << QByteArray( "Test" ) << false;
  QTest::newRow( "" ) << QByteArray( "Test" ) << QByteArray( "\"Test\"" ) << true;
  QTest::newRow( "" ) << QByteArray( "Lastname, Firstname" )
                      << QByteArray( "\"Lastname, Firstname\"" ) << false;
  QTest::newRow( "" ) << QByteArray( "John \"the hacker\" Smith" )
                      << QByteArray( "\"John \\\"the hacker\\\" Smith\"" ) << false;

  // Test the whole thing on strings as well, for one example
  QString string( QLatin1String( "John \"the hacker\" Smith" ) );
  addQuotes( string, false );
  QCOMPARE( string, QString::fromLatin1( "\"John \\\"the hacker\\\" Smith\"" ) );
}

void UtilTest::testIsSigned_data()
{
  QTest::addColumn<QByteArray>( "input" );
  QTest::addColumn<bool>( "hasSignature" );

  QTest::newRow( "pgp" ) << QByteArray(
    "From: xxx xxx <xxx@xxx.xxx>\n"
    "To: xxx xxx <xxx@xxx.xxx>\n"
    "Subject: Re: xxx\n"
    "Date: Mon, 13 Dec 2010 12:22:03 +0100\n"
    "MIME-Version: 1.0\n"
    "Content-Type: multipart/signed;\n"
    "  boundary=\"nextPart1571960.gHxU0aGA9V\";\n"
    "  protocol=\"application/pgp-signature\";\n"
    "  micalg=pgp-sha1\n"
    "Content-Transfer-Encoding: 7bit\n\n"
    "--nextPart1571960.gHxU0aGA9V\n"
    "Content-Type: text/plain;\n"
    "  charset=\"iso-8859-15\"\n"
    "Content-Transfer-Encoding: quoted-printable\n"
    "Content-Disposition: inline\n\n"
    "Hi there...\n\n"
    "--nextPart1571960.gHxU0aGA9V\n"
    "Content-Type: application/pgp-signature; name=signature.asc\n"
    "Content-Description: This is a digitally signed message part.\n\n"
    "-----BEGIN PGP SIGNATURE-----\n"
    "Version: GnuPG v2.0.15 (GNU/Linux)\n"
    "...\n"
    "-----END PGP SIGNATURE-----\n\n"
    "--nextPart1571960.gHxU0aGA9V--\n"
  ) << true;
}

void UtilTest::testIsSigned()
{
  QFETCH( QByteArray, input );
  QFETCH( bool, hasSignature );

  KMime::Message::Ptr msg( new KMime::Message );
  msg->setContent( input );
  msg->parse();
  QCOMPARE( isSigned( msg.get() ), hasSignature );
}
