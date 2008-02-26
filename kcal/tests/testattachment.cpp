/*
  This file is part of the kcal library.
  Copyright (C) 2006 Allen Winter <winter@kde.org>

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
#include <qtest_kde.h>

#include "testattachment.h"
#include "testattachment.moc"

QTEST_KDEMAIN( AttachmentTest, NoGUI )

#include "kcal/event.h"
#include "kcal/attachment.h"
using namespace KCal;

void AttachmentTest::testValidity()
{
  Attachment attachment( QString( "http://www.kde.org" ) );
  QCOMPARE( attachment.uri(), QString::fromLatin1("http://www.kde.org") );
  QCOMPARE( attachment.data(), (char*)0 );
  QVERIFY( attachment.decodedData().isEmpty() );
  QVERIFY( !attachment.isBinary() );

  attachment.setDecodedData( "foo" );
  QVERIFY( attachment.isBinary() );
  QCOMPARE( attachment.decodedData(), QByteArray("foo") );
  QCOMPARE( attachment.data(), "Zm9v" );
  QCOMPARE( attachment.size(), 3U );

  Attachment attachment2 = Attachment( "Zm9v" );
  QCOMPARE( attachment2.size(), 3U );
  QCOMPARE( attachment2.decodedData(), QByteArray("foo") );
  attachment2.setDecodedData( "123456" );
  QCOMPARE( attachment2.size(), 6U );

  Attachment attachment3( attachment2 );
  QCOMPARE( attachment3.size(), attachment2.size() );
}
