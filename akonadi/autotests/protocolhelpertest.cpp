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
#include "protocolhelper.cpp"

using namespace Akonadi;

Q_DECLARE_METATYPE(Scope)
Q_DECLARE_METATYPE(QVector<Protocol::Ancestor>)
Q_DECLARE_METATYPE(Protocol::FetchCollectionsResponse)
Q_DECLARE_METATYPE(Protocol::FetchScope)

class ProtocolHelperTest : public QObject
{
  Q_OBJECT
  private Q_SLOTS:
    void testItemSetToByteArray_data()
    {
      QTest::addColumn<Item::List>( "items" );
      QTest::addColumn<Scope>( "result" );
      QTest::addColumn<bool>( "shouldThrow" );

      Item u1; u1.setId( 1 );
      Item u2; u2.setId( 2 );
      Item u3; u3.setId( 3 );
      Item r1; r1.setRemoteId( QStringLiteral("A") );
      Item r2; r2.setRemoteId( QStringLiteral("B") );
      Item h1; h1.setRemoteId( QStringLiteral("H1") ); h1.setParentCollection( Collection::root() );
      Item h2; h2.setRemoteId( QStringLiteral("H2a") ); h2.parentCollection().setRemoteId( QStringLiteral("H2b") ); h2.parentCollection().setParentCollection( Collection::root() );
      Item h3; h3.setRemoteId( QStringLiteral("H3a") ); h3.parentCollection().setRemoteId( QStringLiteral("H3b") );

      QTest::newRow( "empty" ) << Item::List() << Scope() << true;
      QTest::newRow( "single uid" ) << (Item::List() << u1) << Scope(1) << false;
      QTest::newRow( "multi uid" ) << (Item::List() << u1 << u3) << Scope(QVector<qint64>{ 1, 3 }) << false;
      QTest::newRow( "block uid" ) << (Item::List() << u1 << u2 << u3) << Scope(ImapInterval(1, 3)) << false;
      QTest::newRow( "single rid" ) << (Item::List() << r1) << Scope(Scope::Rid, { QLatin1String("A") }) << false;
      QTest::newRow( "multi rid" ) << (Item::List() << r1 << r2) << Scope(Scope::Rid, { QLatin1String("A"), QLatin1String("B") }) << false;
      QTest::newRow( "invalid" ) << (Item::List() << Item()) << Scope() << true;
      QTest::newRow( "mixed" ) << (Item::List() << u1 << r1) << Scope() << true;
      QTest::newRow( "single hrid" ) << (Item::List() << h1) << Scope({ Scope::HRID(-1, QLatin1String("H1")), Scope::HRID(0) }) << false;
      QTest::newRow( "single hrid 2" ) << (Item::List() << h2) << Scope({ Scope::HRID(-1, QLatin1String("H2a")), Scope::HRID(-2, QLatin1String("H2b")), Scope::HRID(0) })<< false;
      QTest::newRow( "mixed hrid/rid" ) << (Item::List() << h1 << r1) << Scope(Scope::Rid, { QLatin1String("H1"), QLatin1String("A") }) << false;
      QTest::newRow( "unterminated hrid" ) << (Item::List() << h3) << Scope(Scope::Rid, { QLatin1String("H3a") }) << false;
    }

    void testItemSetToByteArray()
    {
      QFETCH( Item::List, items );
      QFETCH( Scope, result );
      QFETCH( bool, shouldThrow );

      bool didThrow = false;
      try {
        const Scope scope = ProtocolHelper::entitySetToScope(items);
        QCOMPARE(scope, result);
      } catch ( const std::exception &e ) {
        qDebug() << e.what();
        didThrow = true;
      }
      QCOMPARE( didThrow, shouldThrow );
    }

    void testAncestorParsing_data()
    {
      QTest::addColumn<QVector<Protocol::Ancestor>>( "input" );
      QTest::addColumn<Collection>( "parent" );

      QTest::newRow( "top-level" ) << QVector<Protocol::Ancestor>{ Protocol::Ancestor(0) } << Collection::root();

      Protocol::Ancestor a1(42);
      a1.setRemoteId(QLatin1String("net"));

      Collection c1;
      c1.setRemoteId( QStringLiteral("net") );
      c1.setId( 42 );
      c1.setParentCollection( Collection::root() );
      QTest::newRow( "till's obscure folder" ) << QVector<Protocol::Ancestor>{ a1, Protocol::Ancestor(0) } << c1;
    }

    void testAncestorParsing()
    {
      QFETCH( QVector<Protocol::Ancestor>, input );
      QFETCH( Collection, parent );

      Item i;
      ProtocolHelper::parseAncestors( input, &i );
      QCOMPARE( i.parentCollection().id(), parent.id() );
      QCOMPARE( i.parentCollection().remoteId(), parent.remoteId() );
    }

    void testCollectionParsing_data()
    {
      QTest::addColumn<Protocol::FetchCollectionsResponse>( "input" );
      QTest::addColumn<Collection>( "collection" );

      Collection c1;
      c1.setId( 2 );
      c1.setRemoteId( QStringLiteral("r2") );
      c1.parentCollection().setId( 1 );
      c1.setName( QStringLiteral("n2") );

      {
            Protocol::FetchCollectionsResponse resp(2);
            resp.setParentId(1);
            resp.setRemoteId(QLatin1String("r2"));
            resp.setName(QLatin1String("n2"));
            QTest::newRow( "no ancestors" ) << resp << c1;
      }

      {
            Protocol::FetchCollectionsResponse resp(3);
            resp.setParentId(2);
            resp.setRemoteId(QLatin1String("r3"));
            resp.setAncestors({ Protocol::Ancestor(2, QLatin1String("r2")), Protocol::Ancestor(1, QLatin1String("r1")), Protocol::Ancestor(0) });

            Collection c2;
            c2.setId( 3 );
            c2.setRemoteId( QStringLiteral("r3") );
            c2.parentCollection().setId( 2 );
            c2.parentCollection().setRemoteId( QStringLiteral("r2") );
            c2.parentCollection().parentCollection().setId( 1 );
            c2.parentCollection().parentCollection().setRemoteId( QStringLiteral("r1") );
            c2.parentCollection().parentCollection().setParentCollection( Collection::root() );
            QTest::newRow( "ancestors" ) << resp << c2;
      }
    }

    void testCollectionParsing()
    {
      QFETCH( Protocol::FetchCollectionsResponse, input );
      QFETCH( Collection, collection );

      Collection parsedCollection = ProtocolHelper::parseCollection( input );

      QCOMPARE( parsedCollection.name(), collection.name() );

      while ( collection.isValid() || parsedCollection.isValid() ) {
        QCOMPARE( parsedCollection.id(), collection.id() );
        QCOMPARE( parsedCollection.remoteId(), collection.remoteId() );
        const Collection p1( parsedCollection.parentCollection() );
        const Collection p2( collection.parentCollection() );
        parsedCollection = p1;
        collection = p2;
      }
    }

    void testParentCollectionAfterCollectionParsing()
    {
      Protocol::FetchCollectionsResponse resp(111);
      resp.setParentId(222);
      resp.setRemoteId(QLatin1String("A"));
      resp.setAncestors({ Protocol::Ancestor(222), Protocol::Ancestor(333), Protocol::Ancestor(0) });

      Collection parsedCollection = ProtocolHelper::parseCollection( resp );

      QList<qint64> ids;
      ids << 111 << 222 << 333 << 0;
      int i = 0;

      Collection col = parsedCollection;
      while ( col.isValid() ) {
        QCOMPARE( col.id(), ids[i++] );
        col = col.parentCollection();
      }
      QCOMPARE( i, 4 );
    }

    void testHRidToScope_data()
    {
      QTest::addColumn<Collection>( "collection" );
      QTest::addColumn<Scope>( "result" );

      QTest::newRow( "empty" ) << Collection() << Scope();

      {
            Scope scope;
            scope.setHRidChain({ Scope::HRID(0) });
            QTest::newRow( "root" ) << Collection::root() << scope;
      }

      Collection c;
      c.setId(1);
      c.setParentCollection( Collection::root() );
      c.setRemoteId( QStringLiteral("r1") );
      {
            Scope scope;
            scope.setHRidChain({ Scope::HRID(1, QLatin1String("r1")), Scope::HRID(0) });
            QTest::newRow( "one level" ) << c << scope;
      }

      {
            Collection c2;
            c2.setId(2);
            c2.setParentCollection( c );
            c2.setRemoteId( QStringLiteral("r2") );

            Scope scope;
            scope.setHRidChain({ Scope::HRID(2, QLatin1String("r2")), Scope::HRID(1, QLatin1String("r1")), Scope::HRID(0) });
            QTest::newRow( "two level ok" ) << c2 << scope;
      }
    }

    void testHRidToScope()
    {
      QFETCH( Collection, collection );
      QFETCH( Scope, result );
      QCOMPARE( ProtocolHelper::hierarchicalRidToScope( collection ), result );
    }

    void testItemFetchScopeToProtocol_data()
    {
      QTest::addColumn<ItemFetchScope>( "scope" );
      QTest::addColumn<Protocol::FetchScope>( "result" );

      {
            Protocol::FetchScope fs;
            fs.setFetch(Protocol::FetchScope::Flags |
                        Protocol::FetchScope::Size |
                        Protocol::FetchScope::RemoteID |
                        Protocol::FetchScope::RemoteRevision |
                        Protocol::FetchScope::MTime);
            QTest::newRow( "empty" ) << ItemFetchScope() << fs;
      }

      {
            ItemFetchScope scope;
            scope.fetchAllAttributes();
            scope.fetchFullPayload();
            scope.setAncestorRetrieval( Akonadi::ItemFetchScope::All );
            scope.setIgnoreRetrievalErrors( true );

            Protocol::FetchScope fs;
            fs.setFetch(Protocol::FetchScope::FullPayload |
                        Protocol::FetchScope::AllAttributes |
                        Protocol::FetchScope::Flags |
                        Protocol::FetchScope::Size |
                        Protocol::FetchScope::RemoteID |
                        Protocol::FetchScope::RemoteRevision |
                        Protocol::FetchScope::MTime |
                        Protocol::FetchScope::IgnoreErrors);
            fs.setAncestorDepth(Akonadi::Protocol::Ancestor::AllAncestors);
            QTest::newRow( "full" ) << scope << fs;
      }

      {
            ItemFetchScope scope;
            scope.setFetchModificationTime( false );
            scope.setFetchRemoteIdentification( false );

            Protocol::FetchScope fs;
            fs.setFetch(Protocol::FetchScope::Flags |
                        Protocol::FetchScope::Size);
            QTest::newRow( "minimal" ) << scope << fs;
      }
    }

    void testItemFetchScopeToProtocol()
    {
      QFETCH( ItemFetchScope, scope );
      QFETCH( Protocol::FetchScope, result );
      QCOMPARE( ProtocolHelper::itemFetchScopeToProtocol( scope ), result );
    }
};

QTEST_MAIN( ProtocolHelperTest )

#include "protocolhelpertest.moc"
