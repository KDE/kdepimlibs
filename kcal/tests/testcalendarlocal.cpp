/*
  This file is part of the kcal library.
  Copyright (C) 2006-2007 Allen Winter <winter@kde.org>

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

#include "testcalendarlocal.h"
#include "testcalendarlocal.moc"

QTEST_KDEMAIN( CalendarLocalTest, NoGUI )

#include "kcal/calendarlocal.h"
using namespace KCal;

void CalendarLocalTest::testValidity()
{
  CalendarLocal cal( KDateTime::UTC );
  cal.setProductId( QLatin1String( "fredware calendar" ) );
  QVERIFY( cal.productId() == QLatin1String( "fredware calendar" ) );
  QVERIFY( cal.timeZoneId() == QLatin1String( "UTC" ) );
  QVERIFY( cal.timeSpec() == KDateTime::UTC );
  cal.close();
}

void CalendarLocalTest::testSave()
{
  CalendarLocal cal( QLatin1String( "UTC" ) );
  cal.setProductId( QLatin1String( "fredware calendar" ) );
  QVERIFY( cal.timeZoneId() == QLatin1String( "UTC" ) );
  QVERIFY( cal.timeSpec() == KDateTime::UTC );
  QVERIFY( cal.save( "foo.ics" ) );
  cal.close();
  unlink( "foo.ics" );
}

void CalendarLocalTest::testSaveLoadSave()
{
  CalendarLocal cal( QLatin1String( "UTC" ) );
  cal.setProductId( QLatin1String( "fredware calendar" ) );
  QVERIFY( cal.timeZoneId() == QLatin1String( "UTC" ) );
  QVERIFY( cal.timeSpec() == KDateTime::UTC );
  QVERIFY( cal.save( "foo.ics" ) );
  cal.close();
  QVERIFY( cal.load( "foo.ics" ) );
  QVERIFY( cal.save() );
  unlink( "foo.ics" );
}

void CalendarLocalTest::testEvents()
{
  CalendarLocal cal( KDateTime::UTC );
  cal.setProductId( QLatin1String( "fredware calendar" ) );
  QDate dt = QDate::currentDate();

  Event *event1 = new Event();
  event1->setUid( "1" );
  event1->setDtStart( KDateTime( dt ) );
  event1->setDtEnd( KDateTime( dt ).addDays( 1 ) );
  event1->setSummary( "Event1 Summary" );
  event1->setDescription( "This is a description of the first event" );
  event1->setLocation( "the place" );

  Event *event2 = new Event();
  event2->setUid( "2" );
  event2->setDtStart( KDateTime( dt ).addDays( 1 ) );
  event2->setDtEnd( KDateTime( dt ).addDays( 2 ) );
  event2->setSummary( "Event2 Summary" );
  event2->setDescription( "This is a description of the second event" );
  event2->setLocation( "the other place" );

  QVERIFY( cal.addEvent( event1 ) );
  QVERIFY( cal.addEvent( event2 ) );

  QVERIFY( cal.save( "foo.ics" ) );
  cal.close();
  unlink( "foo.ics" );
}

void CalendarLocalTest::testIncidences()
{
  CalendarLocal cal( KDateTime::UTC );
  cal.setProductId( QLatin1String( "fredware calendar" ) );
  QDate dt = QDate::currentDate();

  Event *event1 = new Event();
  event1->setUid( "1" );
  event1->setDtStart( KDateTime( dt ) );
  event1->setDtEnd( KDateTime( dt ).addDays( 1 ) );
  event1->setSummary( "Event1 Summary" );
  event1->setDescription( "This is a description of the first event" );
  event1->setLocation( "the place" );

  Event *event2 = new Event();
  event2->setUid( "2" );
  event2->setDtStart( KDateTime( dt ).addDays( 1 ) );
  event2->setDtEnd( KDateTime( dt ).addDays( 2 ) );
  event2->setSummary( "Event2 Summary" );
  event2->setDescription( "This is a description of the second event" );
  event2->setLocation( "the other place" );

  QVERIFY( cal.addEvent( event1 ) );
  QVERIFY( cal.addEvent( event2 ) );

  Todo *todo1 = new Todo();
  todo1->setUid( "3" );
  todo1->setDtStart( KDateTime( dt ).addDays( 1 ) );
  todo1->setDtDue( KDateTime( dt ).addDays( 2 ) );
  todo1->setSummary( "Todo1 Summary" );
  todo1->setDescription( "This is a description of a todo" );
  todo1->setLocation( "this place" );

  Todo *todo2 = new Todo();
  todo2->setUid( "4" );
  todo2->setDtStart( KDateTime( dt ).addDays( 1 ) );
  todo2->setFloats( true );
  todo2->setSummary( "<qt><h1>Todo2 Summary</h1></qt>", true );
  todo2->setDescription( "This is a description of a todo" );
  todo2->setLocation( "<html><a href=\"http://www.fred.com\">this place</a></html>", true );

  QVERIFY( cal.addTodo( todo1 ) );
  QVERIFY( cal.addTodo( todo2 ) );

  QVERIFY( cal.save( "foo.ics" ) );
  cal.close();

  QVERIFY( cal.load( "foo.ics" ) );
  Todo *todo = static_cast<Todo *>( cal.incidence( "4" ) );
  QVERIFY( todo->uid() == "4" );
  QVERIFY( todo->summaryIsRich() );
  QVERIFY( todo->locationIsRich() );
  cal.close();
  unlink( "foo.ics" );
}
