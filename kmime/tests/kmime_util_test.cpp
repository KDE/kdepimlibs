/*
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

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
}

void KMimeUtilTest::testExtractHeader()
{
  QByteArray header( "To: <foo@bla.org>\nSubject: =?UTF-8?Q?_Notification_for_appointment:?=\n =?UTF-8?Q?_Test?=\nMIME-Version: 1.0" );

  // basic tests
  QVERIFY( extractHeader( header, "Foo" ).isEmpty() );
  QCOMPARE( extractHeader( header, "To" ), QByteArray("<foo@bla.org>") );

  // case insensitive matching
  QCOMPARE( extractHeader( header, "mime-version" ), QByteArray( "1.0" ) );

  // extraction of multi-line headers
  QCOMPARE( extractHeader( header, "Subject" ),
            QByteArray("=?UTF-8?Q?_Notification_for_appointment:?= =?UTF-8?Q?_Test?=") );

  // missing space after ':'
  QCOMPARE( extractHeader( "From:<toma@kovoks.nl>", "From" ), QByteArray( "<toma@kovoks.nl>" ) );
}
