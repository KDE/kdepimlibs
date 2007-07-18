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

#include "testcalendarnull.h"
#include "testcalendarnull.moc"

QTEST_KDEMAIN( CalendarNullTest, NoGUI )

#include "kcal/calendarnull.h"
using namespace KCal;

void CalendarNullTest::testValidity()
{
  CalendarNull cal( KDateTime::UTC );
  CalendarNull *s1 = cal.self();
  QVERIFY( cal.reload() );
  cal.setTimeZoneId( QLatin1String( "America/New_York" ) );
  CalendarNull *s2 = cal.self();
  QVERIFY( !( s1 == 0 ) );
  QCOMPARE( s1, s2 );
  cal.close();
}

