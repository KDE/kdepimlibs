/*
  This file is part of the kcal library.
  Copyright (c) 2009 Allen Winter <winter@kde.org>

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
#include "testcustomproperties.h"
#include "testcustomproperties.moc"
#include <qtest_kde.h>

QTEST_KDEMAIN( CustomPropertiesTest, NoGUI )

#include "kcal/customproperties.h"
using namespace KCal;

void CustomPropertiesTest::testValidity()
{
  CustomProperties cp;

  QByteArray app( "KORG" );
  QByteArray key( "TEXT" );

  cp.setCustomProperty( app, key, "rich" );
  QCOMPARE( cp.customProperty( app, key ), QString( "rich" ) );

  cp.removeCustomProperty( app, key );
  cp.setCustomProperty( app, key, "foo" );
  cp.setCustomProperty( app, key, "rich" );
  QCOMPARE( cp.customProperty( app, key ), QString( "rich" ) );

  key = "X-TEXT";
  cp.setNonKDECustomProperty( key, "rich" );
  QCOMPARE( cp.nonKDECustomProperty( key ), QString( "rich" ) );

  cp.removeNonKDECustomProperty( key );
  cp.setNonKDECustomProperty( key, "foo" );
  cp.setNonKDECustomProperty( key, "rich" );
  QCOMPARE( cp.nonKDECustomProperty( key ), QString( "rich" ) );
}

void CustomPropertiesTest::testCompare()
{
  CustomProperties cp1, cp2;

  QByteArray app( "KORG" );
  QByteArray key( "TEXT" );

  cp1.setCustomProperty( app, key, "rich" );
  cp2 = cp1;
  QVERIFY( cp1 == cp2 );

  CustomProperties cp3;
  cp3.setCustomProperty( app, key, cp1.customProperty( app, key ) );
  QVERIFY( cp1 == cp3 );

  key = "X-TEXT";
  cp1.setNonKDECustomProperty( key, "rich" );
  cp2 = cp1;
  QVERIFY( cp1 == cp2 );

  cp3.setNonKDECustomProperty( key, cp1.nonKDECustomProperty( key ) );
  QVERIFY( cp1 == cp3 );
}

void CustomPropertiesTest::testMapValidity()
{
  QMap<QByteArray, QString> cpmap;
  cpmap.insert( "X-key1", QString( "val1" ) );
  cpmap.insert( "X-key2", QString( "val2" ) );
  cpmap.insert( "X-key3", QString( "val3" ) );
  cpmap.insert( "X-key4", QString( "val4" ) );
  cpmap.insert( "X-key5", QString( "val5" ) );

  CustomProperties cp;
  cp.setCustomProperties( cpmap );

  QVERIFY( cp.customProperties().value( "X-key3" ) == QString( "val3" ) );
}

void CustomPropertiesTest::testMapCompare()
{
  QMap<QByteArray, QString> cpmap;
  cpmap.insert( "X-key1", QString( "val1" ) );
  cpmap.insert( "X-key2", QString( "val2" ) );
  cpmap.insert( "X-key3", QString( "val3" ) );
  cpmap.insert( "X-key4", QString( "val4" ) );
  cpmap.insert( "X-key5", QString( "val5" ) );

  CustomProperties cp1, cp2;
  cp1.setCustomProperties( cpmap );
  cp1 = cp2;
  QVERIFY( cp1 == cp2 );

  CustomProperties cp3;
  cp3.setCustomProperties( cp1.customProperties() );
  QVERIFY( cp1 == cp3 );
}
