/*
    Copyright (c) 20089 Volker Krause <vkrause@kde.org>

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

#include <qtest_akonadi.h>
#include <collection.h>
#include <collectionpathresolver.h>
#include <collectionselectjob_p.h>
#include <control.h>
#include <itemdeletejob.h>
#include <itemfetchjob.h>
#include <transactionjobs.h>
#include <tagcreatejob.h>
#include <itemmodifyjob.h>
#include <resourceselectjob_p.h>
#include "test_utils.h"

#include <QtCore/QObject>

#include <sys/types.h>

using namespace Akonadi;

class ItemDeleteTest : public QObject
{
  Q_OBJECT
  private Q_SLOTS:
    void initTestCase()
    {
      AkonadiTest::checkTestIsIsolated();
      Control::start();
    }

    void testIllegalDelete()
    {
      ItemDeleteJob *djob = new ItemDeleteJob( Item( INT_MAX ), this );
      QVERIFY( !djob->exec() );

      // make sure a failed delete doesn't leave a transaction open (the kpilot bug)
      TransactionRollbackJob *tjob = new TransactionRollbackJob( this );
      QVERIFY( !tjob->exec() );
    }

    void testDelete()
    {
      ItemFetchJob *fjob = new ItemFetchJob( Item( 1 ), this );
      AKVERIFYEXEC( fjob );
      QCOMPARE( fjob->items().count(), 1 );

      ItemDeleteJob *djob = new ItemDeleteJob( Item( 1 ), this );
      AKVERIFYEXEC( djob );

      fjob = new ItemFetchJob( Item( 1 ), this );
      QVERIFY( !fjob->exec() );
    }

    void testDeleteFromUnselectedCollection()
    {
      const QString path = QLatin1String( "res1" ) +
                           CollectionPathResolver::pathDelimiter() +
                           QLatin1String( "foo" );
      CollectionPathResolver *rjob = new CollectionPathResolver( path, this );
      AKVERIFYEXEC( rjob );

      ItemFetchJob *fjob = new ItemFetchJob( Collection( rjob->collection() ), this );
      AKVERIFYEXEC( fjob );

      const Item::List items = fjob->items();
      QVERIFY( items.count() > 0 );

      CollectionSelectJob *sjob = new CollectionSelectJob( Collection( 2 ), this );
      AKVERIFYEXEC( sjob );

      fjob = new ItemFetchJob( items[ 0 ], this );
      AKVERIFYEXEC( fjob );
      QCOMPARE( fjob->items().count(), 1 );

      ItemDeleteJob *djob = new ItemDeleteJob( items[ 0 ], this );
      AKVERIFYEXEC( djob );

      fjob = new ItemFetchJob( items[ 0 ], this );
      QVERIFY( !fjob->exec() );
    }

    void testRidDelete()
    {
      {
          ResourceSelectJob *select = new ResourceSelectJob(QStringLiteral("akonadi_knut_resource_0"));
          AKVERIFYEXEC(select);
      }
      const Collection col (collectionIdFromPath(QStringLiteral("res1/foo")));
      QVERIFY(col.isValid());

      CollectionSelectJob *sel = new CollectionSelectJob( col );
      AKVERIFYEXEC(sel);

      Item i;
      i.setRemoteId( QLatin1String("C") );

      ItemFetchJob *fjob = new ItemFetchJob( i, this );
      fjob->setCollection( col );
      AKVERIFYEXEC( fjob );
      QCOMPARE( fjob->items().count(), 1 );

      ItemDeleteJob *djob = new ItemDeleteJob( i, this );
      AKVERIFYEXEC( djob );

      fjob = new ItemFetchJob( i, this );
      fjob->setCollection( col );
      QVERIFY( !fjob->exec() );
      {
          ResourceSelectJob *select = new ResourceSelectJob(QStringLiteral(""));
          AKVERIFYEXEC(select);
      }
    }

    void testTagDelete()
    {
      // Create tag
      Tag tag;
      tag.setName( QLatin1String( "Tag1" ) );
      tag.setGid( "Tag1" );
      TagCreateJob *tjob = new TagCreateJob( tag, this );
      AKVERIFYEXEC( tjob );
      tag = tjob->tag();

      const Collection col( collectionIdFromPath( QLatin1String("res1/foo") ) );
      QVERIFY( col.isValid() );

      Item i;
      i.setRemoteId( QLatin1String("D") );

      ItemFetchJob *fjob = new ItemFetchJob( i, this );
      fjob->setCollection( col );
      AKVERIFYEXEC( fjob );
      QCOMPARE( fjob->items().count(), 1 );

      i = fjob->items().first();
      i.setTag(tag);
      ItemModifyJob *mjob = new ItemModifyJob( i, this );
      AKVERIFYEXEC( mjob );

      // Delete the tagged item
      ItemDeleteJob *djob = new ItemDeleteJob( tag, this );
      AKVERIFYEXEC( djob );

      // Try to fetch the item again, there should be none
      fjob = new ItemFetchJob( i, this );
      QVERIFY( !fjob->exec() );
    }

    void testCollectionDelete()
    {
      const Collection col( collectionIdFromPath( QLatin1String("res1/foo") ) );
      ItemFetchJob *fjob = new ItemFetchJob( col, this );
      AKVERIFYEXEC( fjob );
      QVERIFY( fjob->items().count() > 0 );

      // delete from non-empty collection
      ItemDeleteJob *djob = new ItemDeleteJob( col, this );
      AKVERIFYEXEC( djob );

      fjob = new ItemFetchJob( col, this );
      AKVERIFYEXEC( fjob );
      QCOMPARE( fjob->items().count(), 0 );

      // delete from empty collection
      djob = new ItemDeleteJob( col, this );
      QVERIFY( !djob->exec() ); // error: no items found

      fjob = new ItemFetchJob( col, this );
      AKVERIFYEXEC( fjob );
      QCOMPARE( fjob->items().count(), 0 );
    }

};

QTEST_AKONADIMAIN( ItemDeleteTest )

#include "itemdeletetest.moc"
