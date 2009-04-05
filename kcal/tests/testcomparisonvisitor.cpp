/*
  This file is part of libkcal.

  Copyright 2009 Ingo Klöcker <kloecker@kde.org>

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

#include "kcal/comparisonvisitor.h"

#include "kcal/event.h"
#include "kcal/freebusy.h"
#include "kcal/journal.h"
#include "kcal/todo.h"

#include <QList>

#include <qtest_kde.h>

using namespace KCal;

class ComparisonVisitorTest : public QObject
{
  Q_OBJECT

  private:
    ComparisonVisitor mComparator;

  private Q_SLOTS:
    void testEventComparison();
    void testFreeBusyComparison();
    void testJournalComparison();
    void testTodoComparison();
    void testTypeMismatches();
};

QTEST_KDEMAIN( ComparisonVisitorTest, NoGUI )

void ComparisonVisitorTest::testEventComparison()
{
  const QString summary = QLatin1String( "Testing comparison" );
  const QString desc    = QLatin1String( "Testing ComparisonVisitor" );
  const KDateTime now   = KDateTime::currentUtcDateTime();
  const KDateTime later = now.addSecs( 3600 );

  Event reference;
  reference.setSummary( summary );
  reference.setDescription( desc );
  reference.setDtStart( now );
  reference.setDtEnd( later );

  // create a copy of the reference incidence
  Event event( reference );
  
  IncidenceBase *baseReference = &reference;
  IncidenceBase *baseIncidence = &event;

  QVERIFY( mComparator.compare( baseIncidence, baseReference ) );

  // change a property of Event (but not of IncidenceBase)
  event.setHasEndDate( !event.hasEndDate() );
  QVERIFY( !mComparator.compare( baseIncidence, baseReference ) );
}

void ComparisonVisitorTest::testFreeBusyComparison()
{
  const KDateTime now   = KDateTime::currentUtcDateTime();
  const KDateTime later = now.addSecs( 3600 );

  FreeBusy reference;
  reference.setDtStart( now );
  reference.setDtEnd( later );

  // create a copy of the reference incidence
  FreeBusy freebusy( reference );

  IncidenceBase *baseReference = &reference;
  IncidenceBase *baseIncidence = &freebusy;

  QVERIFY( mComparator.compare( baseIncidence, baseReference ) );

  // change a property of FreeBusy (but not of IncidenceBase)
  freebusy.setDtEnd( freebusy.dtEnd().addSecs( 3600 ) );
  QVERIFY( !mComparator.compare( baseIncidence, baseReference ) );
}

void ComparisonVisitorTest::testJournalComparison()
{
  const QString summary = QLatin1String( "Testing comparison" );
  const QString desc    = QLatin1String( "Testing ComparisonVisitor" );
  const KDateTime now   = KDateTime::currentUtcDateTime();

  Journal reference;
  reference.setSummary( summary );
  reference.setDescription( desc );
  reference.setDtStart( now );

  // create a copy of the reference incidence
  Journal journal( reference );

  IncidenceBase *baseReference = &reference;
  IncidenceBase *baseIncidence = &journal;

  QVERIFY( mComparator.compare( baseIncidence, baseReference ) );

  // change a property of Incidence (Journal has no new properties) (but not of IncidenceBase)
  journal.setDescription( summary );
  QVERIFY( !mComparator.compare( baseIncidence, baseReference ) );
}

void ComparisonVisitorTest::testTodoComparison()
{
  const QString summary = QLatin1String( "Testing comparison" );
  const QString desc    = QLatin1String( "Testing ComparisonVisitor" );

  Todo reference;
  reference.setSummary( summary );
  reference.setDescription( desc );
  reference.setPercentComplete( 50 );

  // create a copy of the reference incidence
  Todo todo( reference );

  IncidenceBase *baseReference = &reference;
  IncidenceBase *baseIncidence = &todo;

  QVERIFY( mComparator.compare( baseIncidence, baseReference ) );

  // change a property of Todo (but not of IncidenceBase)
  todo.setPercentComplete( 100 );
  QVERIFY( !mComparator.compare( baseIncidence, baseReference ) );
}

void ComparisonVisitorTest::testTypeMismatches()
{
  Event event;
  FreeBusy freeBusy;
  Journal journal;
  Todo todo;

  QList<IncidenceBase*> list;
  list << &event << &freeBusy << &journal << &todo << 0;

  for ( int i = 0; i < list.size(); ++i ) {
    for ( int j = 0; j < list.size(); ++j ) {
      if ( i == j )
        QVERIFY( mComparator.compare( list[ i ], list[ j ] ) );
      else
        QVERIFY( !mComparator.compare( list[ i ], list[ j ] ) );
    }
  }
}

#include "kcalcomparisontest.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;
