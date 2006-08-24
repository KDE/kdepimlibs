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
#include <unistd.h>

#include <qtest_kde.h>

#include "testcalendar.h"
#include "testcalendar.moc"

QTEST_KDEMAIN( CalendarTest, NoGUI )

#include "kcal/calendarlocal.h"
using namespace KCal;

void CalendarTest::testValidity() {
  CalendarLocal cal( QLatin1String("UTC") );
  cal.setProductId( QLatin1String( "fredware calendar" ) );
  QVERIFY( cal.productId() == QLatin1String( "fredware calendar" ) );
  QVERIFY( cal.timeZoneId() == QLatin1String( "UTC" ) );
}

void CalendarTest::testSave() {
  CalendarLocal cal( QLatin1String("UTC") );
  cal.setProductId( QLatin1String( "fredware calendar" ) );
  QVERIFY( cal.save( "foo.ics" ) );
  unlink( "foo.ics" );
}

