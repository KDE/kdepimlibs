/*
  This file is part of the kcalcore library.

  Copyright (c) 2011 Sérgio Martins <iamsergio@gmail.com>

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
#include "testrecurtodo.h"
#include "testrecurtodo.moc"
#include "../todo.h"
//#include <kdebug.h>
#include <qtest_kde.h>
QTEST_KDEMAIN( RecurTodoTest, NoGUI )

using namespace KCalCore;

void RecurTodoTest::testAllDay()
{
  const QDate dueDate( QDate::currentDate().addDays( -3 ) );
  Todo *todo = new Todo();
  todo->setDtStart( KDateTime( dueDate.addDays( -1 ) ) );
  todo->setDtDue( KDateTime( dueDate ) );
  todo->setSummary( QLatin1String( "All day event" ) );
  todo->setAllDay( true );

  Recurrence *recurrence = todo->recurrence();
  recurrence->unsetRecurs();
  recurrence->setDaily( 1 );
  QVERIFY( todo->dtDue() == KDateTime( dueDate ) );
  todo->setCompleted( KDateTime::currentUtcDateTime() );
  QVERIFY( todo->recurs() );
  QVERIFY( todo->percentComplete() == 0 );
  QVERIFY( todo->dtDue().date() == QDate::currentDate() );

  todo->setCompleted( KDateTime::currentUtcDateTime() );
  QVERIFY( todo->dtDue().date() == QDate::currentDate().addDays( 1 ) );
  QVERIFY( todo->dtDue( true /*first ocurrence*/ ).date() == dueDate );
}

void RecurTodoTest::testNonAllDay()
{
  const QDateTime currentDateTime = QDateTime::currentDateTime();
  const QDate currentDate = currentDateTime.date();
  const QTime currentTimeWithMS = currentDateTime.time();

  const QDate twoDaysAgo( currentDate.addDays( -2 ) );
  const QDate treeDaysAgo( currentDate.addDays( -3 ) );
  const QTime currentTime( currentTimeWithMS.hour(), currentTimeWithMS.minute(), currentTimeWithMS.second() );

  Todo *todo = new Todo();
  todo->setDtStart( KDateTime( twoDaysAgo ) );
  todo->setDtDue( KDateTime( treeDaysAgo, currentTime ) );
  todo->setSummary( QLatin1String( "Not an all day event" ) );
  todo->setAllDay( false );

  Recurrence *recurrence = todo->recurrence();
  recurrence->unsetRecurs();
  recurrence->setDaily( 1 );
  QVERIFY( todo->dtDue() == KDateTime( treeDaysAgo, currentTime ) );
  todo->setCompleted( KDateTime::currentUtcDateTime() );
  QVERIFY( todo->recurs() );
  QVERIFY( todo->percentComplete() == 0 );
  //kDebug() << todo->dtDue() << KDateTime( currentDate, currentTime, todo->dtDue().timeSpec() ).addDays( 1 );
  QVERIFY( todo->dtDue() == KDateTime( currentDate, currentTime, todo->dtDue().timeSpec() ).addDays( 1 ) );

  todo->setCompleted( KDateTime::currentUtcDateTime() );
  QVERIFY( todo->dtDue() == KDateTime( currentDate, currentTime, todo->dtDue().timeSpec() ).addDays( 2 ) );
  QVERIFY( todo->dtDue( true /*first ocurrence*/ ) == KDateTime( treeDaysAgo, currentTime ) );
}
