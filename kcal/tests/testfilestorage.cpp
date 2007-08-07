/*
  This file is part of the kcal library.
  Copyright (C) 2007 Allen Winter <winter@kde.org>

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
#include <unistd.h>

#include <qtest_kde.h>

#include "testfilestorage.h"
#include "testfilestorage.moc"

QTEST_KDEMAIN( FileStorageTest, NoGUI )

#include "kcal/filestorage.h"
#include "kcal/calendarlocal.h"

using namespace KCal;

void FileStorageTest::testValidity()
{
  CalendarLocal cal( KDateTime::UTC );
  FileStorage fs( &cal, QLatin1String( "fred.ics" ) );
  QCOMPARE( fs.fileName(), QLatin1String( "fred.ics" ) );
  QCOMPARE( fs.calendar(), &cal );
  cal.close();
}

void FileStorageTest::testSave()
{
  CalendarLocal cal( QLatin1String( "UTC" ) );
  FileStorage fs( &cal, QLatin1String( "fred.ics" ) );

  QDate dt = QDate::currentDate();

  Event *event1 = new Event();
  event1->setUid( "1" );
  event1->setDtStart( KDateTime( dt ) );
  event1->setDtEnd( KDateTime( dt ).addDays( 1 ) );
  event1->setSummary( "Event1 Summary" );
  event1->setDescription( "This is a description of the first event" );
  event1->setLocation( "the place" );
  cal.addEvent( event1 );

  Event *event2 = new Event();
  event2->setUid( "2" );
  event2->setDtStart( KDateTime( dt ).addDays( 1 ) );
  event2->setDtEnd( KDateTime( dt ).addDays( 2 ) );
  event2->setSummary( "Event2 Summary" );
  event2->setDescription( "This is a description of the second event" );
  event2->setLocation( "the other place" );
  cal.addEvent( event2 );

  QVERIFY( fs.open() );
  QVERIFY( fs.save() );
  QVERIFY( fs.close() );
  cal.close();
  unlink( "fred.ics" );
}

void FileStorageTest::testSaveLoadSave()
{
  CalendarLocal cal( QLatin1String( "UTC" ) );
  FileStorage fs( &cal, QLatin1String( "fred.ics" ) );

  QDate dt = QDate::currentDate();

  Event *event1 = new Event();
  event1->setUid( "1" );
  event1->setDtStart( KDateTime( dt ) );
  event1->setDtEnd( KDateTime( dt ).addDays( 1 ) );
  event1->setSummary( "Event1 Summary" );
  event1->setDescription( "This is a description of the first event" );
  event1->setLocation( "the place" );
  cal.addEvent( event1 );

  Event *event2 = new Event();
  event2->setUid( "2" );
  event2->setDtStart( KDateTime( dt ).addDays( 1 ) );
  event2->setDtEnd( KDateTime( dt ).addDays( 2 ) );
  event2->setSummary( "Event2 Summary" );
  event2->setDescription( "This is a description of the second event" );
  event2->setLocation( "the other place" );
  cal.addEvent( event2 );

  QVERIFY( fs.open() );
  QVERIFY( fs.save() );
  QVERIFY( fs.close() );
  QVERIFY( fs.open() );
  QVERIFY( fs.load() );
  Event *e = static_cast<Event *>( fs.calendar()->incidence( "1" ) );
  QVERIFY( e != 0 );
  QVERIFY( fs.close() );
  unlink( "fred.ics" );

  QVERIFY( fs.open() );
  QVERIFY( fs.save() );
  QVERIFY( fs.close() );
  unlink( "fred.ics" );
}
