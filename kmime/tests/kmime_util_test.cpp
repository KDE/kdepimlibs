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

#include <qtest_kde.h>

#include "kmime_util_test.h"
#include "kmime_util_test.moc"

#include <kmime_util.h>

using namespace KMime;

QTEST_KDEMAIN( KMimeUtilTest, NoGUI )

void KMimeUtilTest::testUnfoldHeader()
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

void KMimeUtilTest::testExtractHeader()
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
  QCOMPARE( extractHeader( header, "To" ), QByteArray("<foo@bla.org>") );

  // case insensitive matching
  QCOMPARE( extractHeader( header, "mime-version" ), QByteArray( "1.0" ) );

  // extraction of multi-line headers
  QCOMPARE( extractHeader( header, "Subject" ),
            QByteArray("=?UTF-8?Q?_Notification_for_appointment:?= =?UTF-8?Q?_Test?=") );

  // bug #86302 - malformed header continuation
  QCOMPARE( extractHeader( header, "Continuation" ),
            QByteArray("=?UTF-8?Q?_TEST CONT1?= =?UTF-8?Q?_TEST CONT2?=") );

  // missing space after ':'
  QCOMPARE( extractHeader( "From:<toma@kovoks.nl>", "From" ), QByteArray( "<toma@kovoks.nl>" ) );
}

void KMimeUtilTest::testBalanceBidiState()
{
  QFETCH( QString, input );
  QFETCH( QString, expResult );

  QCOMPARE( balanceBidiState( input ), expResult );
}

void KMimeUtilTest::testBalanceBidiState_data()
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

