/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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

#include "test_utils.h"

#include <akonadi/agentmanager.h>
#include <akonadi/agentinstance.h>
#include <akonadi/control.h>
#include <akonadi/collection.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>

#include "../akonadi/collectionsync.cpp"

#include <krandom.h>

#include <QtCore/QObject>
#include <QSignalSpy>

#include <qtest_akonadi.h>

using namespace Akonadi;

Q_DECLARE_METATYPE( KJob* )

class CollectionSyncTest : public QObject
{
  Q_OBJECT
  private:
    Collection::List fetchCollections( const QString &res )
    {
      CollectionFetchJob *fetch = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, this );
      fetch->fetchScope().setResource( res );
      fetch->fetchScope().setAncestorRetrieval( CollectionFetchScope::All );
      Q_ASSERT( fetch->exec() );
      Q_ASSERT( !fetch->collections().isEmpty() );
      return fetch->collections();
    }

    void makeTestData()
    {
      QTest::addColumn<bool>( "hierarchicalRIDs" );
      QTest::addColumn<QString>( "resource" );

      QTest::newRow( "akonadi_knut_resource_0 global RID" ) << false << "akonadi_knut_resource_0";
      QTest::newRow( "akonadi_knut_resource_1 global RID" ) << false << "akonadi_knut_resource_1";
      QTest::newRow( "akonadi_knut_resource_2 global RID" ) << false << "akonadi_knut_resource_2";

      QTest::newRow( "akonadi_knut_resource_0 hierarchical RID" ) << true << "akonadi_knut_resource_0";
      QTest::newRow( "akonadi_knut_resource_1 hierarchical RID" ) << true << "akonadi_knut_resource_1";
      QTest::newRow( "akonadi_knut_resource_2 hierarchical RID" ) << true << "akonadi_knut_resource_2";
    }

  private slots:
    void initTestCase()
    {
      AkonadiTest::checkTestIsIsolated();
      Control::start();
      AkonadiTest::setAllResourcesOffline();
      qRegisterMetaType<KJob*>();
    }

    void testFullSync_data()
    {
      makeTestData();
    }

    void testFullSync()
    {
      QFETCH( bool, hierarchicalRIDs );
      QFETCH( QString, resource );

      Collection::List origCols = fetchCollections( resource );

      CollectionSync* syncer = new CollectionSync( resource, this );
      syncer->setHierarchicalRemoteIds( hierarchicalRIDs );
      syncer->setRemoteCollections( origCols );
      AKVERIFYEXEC( syncer );

      Collection::List resultCols = fetchCollections( resource );
      QCOMPARE( resultCols.count(), origCols.count() );
    }

    void testFullStreamingSync_data()
    {
      makeTestData();
    }

    void testFullStreamingSync()
    {
      QFETCH( bool, hierarchicalRIDs );
      QFETCH( QString, resource );

      Collection::List origCols = fetchCollections( resource );

      CollectionSync* syncer = new CollectionSync( resource, this );
      syncer->setHierarchicalRemoteIds( hierarchicalRIDs );
      syncer->setAutoDelete( false );
      QSignalSpy spy( syncer, SIGNAL(result(KJob*)) );
      QVERIFY( spy.isValid() );
      syncer->setStreamingEnabled( true );
      QTest::qWait( 10 );
      QCOMPARE( spy.count(), 0 );

      for ( int i = 0; i < origCols.count(); ++i ) {
        Collection::List l;
        l << origCols[i];
        syncer->setRemoteCollections( l );
        if ( i < origCols.count() - 1 )
          QTest::qWait( 10 ); // enter the event loop so itemsync actually can do something
        QCOMPARE( spy.count(), 0 );
      }
      syncer->retrievalDone();
      QTest::qWait( 1000 ); // let it finish its job
      QCOMPARE( spy.count(), 1 );
      KJob *job = spy.at( 0 ).at( 0 ).value<KJob*>();
      QCOMPARE( job, syncer );
      QCOMPARE( job->errorText(), QString() );
      QCOMPARE( job->error(), 0 );

      Collection::List resultCols = fetchCollections( resource );
      QCOMPARE( resultCols.count(), origCols.count() );

      delete syncer;
    }

    void testIncrementalSync_data()
    {
      makeTestData();
    }

    void testIncrementalSync()
    {
      QFETCH( bool, hierarchicalRIDs );
      QFETCH( QString, resource );
      if ( resource == QLatin1String( "akonadi_knut_resource_2" ) )
        QSKIP( "test requires more than one collection", SkipSingle );

      Collection::List origCols = fetchCollections( resource );

      CollectionSync* syncer = new CollectionSync( resource, this );
      syncer->setHierarchicalRemoteIds( hierarchicalRIDs );
      syncer->setRemoteCollections( origCols, Collection::List() );
      AKVERIFYEXEC( syncer );

      Collection::List resultCols = fetchCollections( resource );
      QCOMPARE( resultCols.count(), origCols.count() );

      Collection::List delCols;
      delCols << resultCols.front();
      resultCols.pop_front();

      // ### not implemented yet I guess
#if 0
      Collection colWithOnlyRemoteId;
      colWithOnlyRemoteId.setRemoteId( resultCols.front().remoteId() );
      delCols << colWithOnlyRemoteId;
      resultCols.pop_front();
#endif

#if 0
      // ### should this work?
      Collection colWithRandomRemoteId;
      colWithRandomRemoteId.setRemoteId( KRandom::randomString( 100 ) );
      delCols << colWithRandomRemoteId;
#endif

      syncer = new CollectionSync( resource, this );
      syncer->setRemoteCollections( resultCols, delCols );
      AKVERIFYEXEC( syncer );

      Collection::List resultCols2 = fetchCollections( resource );
      QCOMPARE( resultCols2.count(), resultCols.count() );
    }

    void testIncrementalStreamingSync_data()
    {
      makeTestData();
    }

    void testIncrementalStreamingSync()
    {
      QFETCH( bool, hierarchicalRIDs );
      QFETCH( QString, resource );

      Collection::List origCols = fetchCollections( resource );

      CollectionSync* syncer = new CollectionSync( resource, this );
      syncer->setHierarchicalRemoteIds( hierarchicalRIDs );
      syncer->setAutoDelete( false );
      QSignalSpy spy( syncer, SIGNAL(result(KJob*)) );
      QVERIFY( spy.isValid() );
      syncer->setStreamingEnabled( true );
      QTest::qWait( 10 );
      QCOMPARE( spy.count(), 0 );

      for ( int i = 0; i < origCols.count(); ++i ) {
        Collection::List l;
        l << origCols[i];
        syncer->setRemoteCollections( l, Collection::List() );
        if ( i < origCols.count() - 1 )
          QTest::qWait( 10 ); // enter the event loop so itemsync actually can do something
        QCOMPARE( spy.count(), 0 );
      }
      syncer->retrievalDone();
      QTest::qWait( 1000 ); // let it finish its job
      QCOMPARE( spy.count(), 1 );
      KJob *job = spy.at( 0 ).at( 0 ).value<KJob*>();
      QCOMPARE( job, syncer );
      QCOMPARE( job->errorText(), QString() );
      QCOMPARE( job->error(), 0 );

      Collection::List resultCols = fetchCollections( resource );
      QCOMPARE( resultCols.count(), origCols.count() );

      delete syncer;
    }

    void testEmptyIncrementalSync_data()
    {
      makeTestData();
    }

    void testEmptyIncrementalSync()
    {
      QFETCH( bool, hierarchicalRIDs );
      QFETCH( QString, resource );

      Collection::List origCols = fetchCollections( resource );

      CollectionSync* syncer = new CollectionSync( resource, this );
      syncer->setHierarchicalRemoteIds( hierarchicalRIDs );
      syncer->setRemoteCollections( Collection::List(), Collection::List() );
      AKVERIFYEXEC( syncer );

      Collection::List resultCols = fetchCollections( resource );
      QCOMPARE( resultCols.count(), origCols.count() );
    }
};

QTEST_AKONADIMAIN( CollectionSyncTest, NoGUI )

#include "collectionsynctest.moc"
