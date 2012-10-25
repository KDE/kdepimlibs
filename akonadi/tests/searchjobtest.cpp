/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "searchjobtest.h"

#include <akonadi/collection.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/searchcreatejob.h>

#include "collectionutils_p.h"

#include <qtest_akonadi.h>

QTEST_AKONADIMAIN( SearchJobTest, NoGUI )

using namespace Akonadi;

void SearchJobTest::initTestCase()
{
  AkonadiTest::checkTestIsIsolated();
}

void SearchJobTest::testCreateDeleteSearch()
{
  SearchCreateJob *create = new SearchCreateJob( "search123456", "<request><userQuery>Akonadi</userQuery></request>", this );
  AKVERIFYEXEC( create );
  Collection created = create->createdCollection();
  QVERIFY( created.isValid() );
  CollectionFetchJob *list = new CollectionFetchJob( Collection( 1 ), CollectionFetchJob::Recursive, this );
  AKVERIFYEXEC( list );
  Collection::List cols = list->collections();
  Collection col;
  foreach ( const Collection &c, cols ) {
    if ( c.name() == "search123456" )
      col = c;
  }
  QVERIFY( col == created );
  QCOMPARE( col.parentCollection().id(), 1LL );
  QVERIFY( col.isVirtual() );

  CollectionDeleteJob *delJob = new CollectionDeleteJob( col, this );
  AKVERIFYEXEC( delJob );
}

void SearchJobTest::testModifySearch()
{
  // make sure there is a virtual collection
  SearchCreateJob *create = new SearchCreateJob( "search123456", "<request><userQuery>Akonadi</userQuery></request>", this );
  AKVERIFYEXEC( create );
  Collection created = create->createdCollection();
  QVERIFY( created.isValid() );

}
