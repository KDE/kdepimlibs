/*
  This file is part of the kcal library.
  Copyright (c) 2007 Allen Winter <winter@kde.org>

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

#include "testfreebusy.h"
#include "testfreebusy.moc"

QTEST_KDEMAIN( FreeBusyTest, NoGUI )

#include "kcal/freebusy.h"

using namespace KCal;

void FreeBusyTest::testValidity()
{
  FreeBusy fb1(
    KDateTime( QDate( 2007, 7, 23 ), QTime( 7, 0, 0 ), KDateTime::UTC ),
    KDateTime( QDate( 2007, 7, 23 ), QTime( 8, 0, 0 ), KDateTime::UTC ) );

  QVERIFY( fb1.dtEnd() ==
           KDateTime( QDate( 2007, 7, 23 ), QTime( 8, 0, 0 ), KDateTime::UTC ) );
}

void FreeBusyTest::testAddSort()
{
  Period::List periods;
  Period q1(
    KDateTime( QDate( 2007, 7, 23 ), QTime( 7, 0, 0 ), KDateTime::UTC ),
    KDateTime( QDate( 2007, 7, 23 ), QTime( 8, 0, 0 ), KDateTime::UTC ) );
  periods.append( q1 );

  Period q2(
    KDateTime( QDate( 2007, 8, 23 ), QTime( 7, 0, 0 ), KDateTime::UTC ),
    KDateTime( QDate( 2007, 8, 23 ), QTime( 8, 0, 0 ), KDateTime::UTC ) );
  periods.append( q2 );

  Period q3(
    KDateTime( QDate( 2007, 9, 23 ), QTime( 7, 0, 0 ), KDateTime::UTC ),
    KDateTime( QDate( 2007, 9, 23 ), QTime( 8, 0, 0 ), KDateTime::UTC ) );
  periods.append( q3 );

  FreeBusy fb1;
  fb1.addPeriods( periods );

  fb1.addPeriod(
    KDateTime( QDate( 2007, 10, 27 ), QTime( 7, 0, 0 ), KDateTime::UTC ),
    KDateTime( QDate( 2007, 10, 27 ), QTime( 8, 0, 0 ), KDateTime::UTC ) );

  fb1.addPeriod(
    KDateTime( QDate( 2007, 8, 27 ), QTime( 7, 0, 0 ), KDateTime::UTC ),
    KDateTime( QDate( 2007, 8, 27 ), QTime( 8, 0, 0 ), KDateTime::UTC ) );

  fb1.addPeriod(
    KDateTime( QDate( 2007, 6, 27 ), QTime( 7, 0, 0 ), KDateTime::UTC ),
    KDateTime( QDate( 2007, 6, 27 ), QTime( 8, 0, 0 ), KDateTime::UTC ) );

  QVERIFY( fb1.busyPeriods().last().end() ==
           KDateTime( QDate( 2007, 10, 27 ), QTime( 8, 0, 0 ), KDateTime::UTC ) );

}
