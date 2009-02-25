/*
  This file is part of the kcal library.
  Copyright (c) 2009 Thomas McGuire <mcguire@kde.org>

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
#include "testincidencerelation.h"
#include "testincidencerelation.moc"
#include <qtest_kde.h>

QTEST_KDEMAIN( IncidenceRelationTest, NoGUI )

#include "kcal/todo.h"
using namespace KCal;

void IncidenceRelationTest::testRelations()
{
  // Build the following tree:
  // todo1
  // \- todo2
  //    \- todo3

  // Then make todo3 independent:
  // todo3
  // todo1
  // \- todo2

  Todo *todo1 = new Todo();
  todo1->setSummary( "todo" );

  Todo *todo2 = new Todo();
  todo2->setSummary( "sub-todo" );

  Todo *todo3 = new Todo();
  todo3->setSummary( "sub-sub-todo" );

  todo3->setRelatedTo( todo2 );
  todo2->setRelatedTo( todo1 );

  QCOMPARE( todo3->relatedToUid(), todo2->uid() );
  QCOMPARE( todo2->relatedToUid(), todo1->uid() );
  QCOMPARE( todo1->relatedToUid(), QString() );

  todo3->setRelatedTo( 0 );

  QCOMPARE( todo3->relatedToUid(), QString() );
  QCOMPARE( todo2->relatedToUid(), todo1->uid() );
  QCOMPARE( todo1->relatedToUid(), QString() );
}
