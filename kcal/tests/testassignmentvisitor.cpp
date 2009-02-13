/*
  This file is part of libkcal.
  Copyright (c) 2009 Kevin Krammer <kevin.krammer@gmx.at>

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

#include "kcal/assignmentvisitor.h"

#include "kcal/event.h"
#include "kcal/freebusy.h"
#include "kcal/journal.h"
#include "kcal/todo.h"

#include <QList>

#include <qtest_kde.h>

using namespace KCal;

class AssignmentVisitorTest : public QObject
{
  Q_OBJECT

  private:
    AssignmentVisitor mAssigner;

  private Q_SLOTS:
    void testEventAssignment();
    void testFreeBusyAssignment();
    void testJournalAssignment();
    void testTodoAssignment();
    void testTypeMismatches();
};

QTEST_KDEMAIN( AssignmentVisitorTest, NoGUI )

void AssignmentVisitorTest::testEventAssignment()
{
  const QString summary = QLatin1String( "Testing assignment" );
  const QString desc    = QLatin1String( "Testing AssignmentVisitor" );
  const KDateTime now   = KDateTime::currentUtcDateTime();
  const KDateTime later = now.addSecs( 3600 );

  Event source;
  source.setSummary( summary );
  source.setDescription( desc );
  source.setDtStart( now );
  source.setDtEnd( later );

  // check source
  {
    QCOMPARE( source.summary(), summary );
    QCOMPARE( source.description(), desc );
    QCOMPARE( source.dtStart(), now );
    QCOMPARE( source.dtEnd(), later );
  }

  Event target1;
  target1 = source;

  // check direct assignment
  {
    QCOMPARE( source, target1 );

    QCOMPARE( target1.summary(), summary );
    QCOMPARE( target1.description(), desc );
    QCOMPARE( target1.dtStart(), now );
    QCOMPARE( target1.dtEnd(), later );
  }

  Event target2;

  IncidenceBase *baseSource = &source;
  IncidenceBase *baseTarget = &target2;

  bool result = mAssigner.assign( baseTarget, baseSource );
  QVERIFY( result );

  // check indirect assignment
  {
    QCOMPARE( source, target2 );

    QCOMPARE( target2.summary(), summary );
    QCOMPARE( target2.description(), desc );
    QCOMPARE( target2.dtStart(), now );
    QCOMPARE( target2.dtEnd(), later );
  }
}

void AssignmentVisitorTest::testFreeBusyAssignment()
{
  const KDateTime now   = KDateTime::currentUtcDateTime();
  const KDateTime later = now.addSecs( 3600 );

  FreeBusy source;
  source.setDtStart( now );
  source.setDtEnd( later );

  // check source
  {
    QCOMPARE( source.dtStart(), now );
    QCOMPARE( source.dtEnd(), later );
  }

  FreeBusy target1;
  target1 = source;

  // check direct assignment
  {
    QCOMPARE( source, target1 );

    QCOMPARE( target1.dtStart(), now );
    QCOMPARE( target1.dtEnd(), later );
  }

  FreeBusy target2;

  IncidenceBase *baseSource = &source;
  IncidenceBase *baseTarget = &target2;

  bool result = mAssigner.assign( baseTarget, baseSource );
  QVERIFY( result );

  // check indirect assignment
  {
    QCOMPARE( source, target2 );

    QCOMPARE( target2.dtStart(), now );
    QCOMPARE( target2.dtEnd(), later );
  }
}

void AssignmentVisitorTest::testJournalAssignment()
{
  const QString summary = QLatin1String( "Testing assignment" );
  const QString desc    = QLatin1String( "Testing AssignmentVisitor" );
  const KDateTime now   = KDateTime::currentUtcDateTime();

  Journal source;
  source.setSummary( summary );
  source.setDescription( desc );
  source.setDtStart( now );

  // check source
  {
    QCOMPARE( source.summary(), summary );
    QCOMPARE( source.description(), desc );
    QCOMPARE( source.dtStart(), now );
  }

  Journal target1;
  target1 = source;

  // check direct assignment
  {
    QCOMPARE( source, target1 );

    QCOMPARE( target1.summary(), summary );
    QCOMPARE( target1.description(), desc );
    QCOMPARE( target1.dtStart(), now );
  }

  Journal target2;

  IncidenceBase *baseSource = &source;
  IncidenceBase *baseTarget = &target2;

  bool result = mAssigner.assign( baseTarget, baseSource );
  QVERIFY( result );

  // check indirect assignment
  {
    QCOMPARE( source, target2 );

    QCOMPARE( target2.summary(), summary );
    QCOMPARE( target2.description(), desc );
    QCOMPARE( target2.dtStart(), now );
  }
}

void AssignmentVisitorTest::testTodoAssignment()
{
  const QString summary = QLatin1String( "Testing assignment" );
  const QString desc    = QLatin1String( "Testing AssignmentVisitor" );

  Todo source;
  source.setSummary( summary );
  source.setDescription( desc );
  source.setPercentComplete( 50 );

  // check source
  {
    QCOMPARE( source.summary(), summary );
    QCOMPARE( source.description(), desc );
    QCOMPARE( source.percentComplete(), 50 );
  }

  Todo target1;
  target1 = source;

  // check direct assignment
  {
    QCOMPARE( source, target1 );

    QCOMPARE( target1.summary(), summary );
    QCOMPARE( target1.description(), desc );
    QCOMPARE( target1.percentComplete(), 50 );
  }

  Todo target2;

  IncidenceBase *baseSource = &source;
  IncidenceBase *baseTarget = &target2;

  bool result = mAssigner.assign( baseTarget, baseSource );
  QVERIFY( result );

  // check indirect assignment
  {
    QCOMPARE( source, target2 );

    QCOMPARE( target2.summary(), summary );
    QCOMPARE( target2.description(), desc );
    QCOMPARE( target2.percentComplete(), 50 );
  }
}

void AssignmentVisitorTest::testTypeMismatches()
{
  Event event;
  FreeBusy freeBusy;
  Journal journal;
  Todo todo;

  QList<IncidenceBase*> list;
  list << &event << &freeBusy << &journal << &todo;

  for ( int i = 0; i < list.size(); ++i ) {
    for ( int j = 0; j < list.size(); ++j ) {
      bool result = mAssigner.assign( list[ i ], list[ j ] );
      if ( i == j )
        QVERIFY( result );
      else
        QVERIFY( !result );
    }
  }
}

#include "kcalassignmenttest.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;
