/*
 * Copyright (C) 2012  Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "testoccurrenceiterator.h"
#include "occurrenceiterator.h"
#include "memorycalendar.h"

#include <qtest_kde.h>
QTEST_KDEMAIN( TestOccurrenceIterator, NoGUI )

void TestOccurrenceIterator::testIterationWithExceptions()
{
  KCalCore::MemoryCalendar calendar(KDateTime::UTC);

  KDateTime start(QDate(2013, 03, 10), QTime(10, 0, 0), KDateTime::UTC);
  KDateTime end(QDate(2013, 03, 10), QTime(11, 0, 0), KDateTime::UTC);

  KDateTime recurrenceId(QDate(2013, 03, 11), QTime(10, 0, 0), KDateTime::UTC);
  KDateTime exceptionStart(QDate(2013, 03, 11), QTime(12, 0, 0), KDateTime::UTC);
  KDateTime exceptionEnd(QDate(2013, 03, 11), QTime(13, 0, 0), KDateTime::UTC);

  KDateTime actualEnd(QDate(2013, 03, 12), QTime(11, 0, 0), KDateTime::UTC);

  KCalCore::Event::Ptr event1(new KCalCore::Event());
  event1->setUid("event1");
  event1->setSummary("event1");
  event1->setDtStart(start);
  event1->setDtEnd(end);
  event1->recurrence()->setDaily(1);
  calendar.addEvent(event1);

  KCalCore::Event::Ptr exception(new KCalCore::Event());
  exception->setUid(event1->uid());
  exception->setSummary("exception");
  exception->setRecurrenceId(recurrenceId);
  exception->setDtStart(exceptionStart);
  exception->setDtEnd(exceptionEnd);
  calendar.addEvent(exception);

  int occurrence = 0;
  KCalCore::OccurrenceIterator rIt( calendar, start, actualEnd );
  while ( rIt.hasNext() ) {
    rIt.next();
    occurrence++;
    if (occurrence == 1) {
      QCOMPARE(rIt.occurrenceStartDate(), start);
      QCOMPARE(rIt.incidence()->summary(), event1->summary());
    }
    if (occurrence == 2) {
      QCOMPARE(rIt.occurrenceStartDate(), exceptionStart);
      QCOMPARE(rIt.incidence()->summary(), exception->summary());
    }
    if (occurrence == 3) {
      QCOMPARE(rIt.occurrenceStartDate(), start.addDays(2));
      QCOMPARE(rIt.incidence()->summary(), event1->summary());
    }
//     qDebug() << occurrence;
//     qDebug() << "occurrence: " << rIt.occurrenceStartDate().toString();
//     qDebug() << "uid: " << rIt.incidence()->uid();
//     qDebug() << "summary: " << rIt.incidence()->summary();
//     qDebug() << "start: " << rIt.incidence()->dtStart().toString();
//     qDebug();
  }
  QCOMPARE(occurrence, 3);
}
