/*
    Copyright (c) 2007, 2009 Volker Krause <vkrause@kde.org>

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

#include "collectionsync_p.h"
#include "collection.h"

#include "collectioncreatejob.h"
#include "collectiondeletejob.h"
#include "collectionfetchjob.h"
#include "collectionmodifyjob.h"
#include "collectionfetchscope.h"
#include "collectionmovejob.h"

#include <kdebug.h>
#include <KLocale>
#include <QtCore/QVariant>

using namespace Akonadi;

struct RemoteNode;

/**
  LocalNode is used to build a tree structure of all our locally existing collections.
*/
struct LocalNode
{
  LocalNode( const Collection &col ) :
    collection( col ),
    processed( false )
  {}

  ~LocalNode()
  {
    qDeleteAll( childNodes );
    qDeleteAll( pendingRemoteNodes );
  }

  Collection collection;
  QList<LocalNode*> childNodes;
  QHash<QString, LocalNode*> childRidMap;
  /** When using hierarchical RIDs we attach a list of not yet processable remote nodes to
      the closest already existing local ancestor node. They will be re-evaluated once a new
      child node is added. */
  QList<RemoteNode*> pendingRemoteNodes;
  bool processed;
};

Q_DECLARE_METATYPE( LocalNode* )
static const char LOCAL_NODE[] = "LocalNode";

/**
  RemoteNode is used as a container for remote collections which typically don't have a UID set
  and thus cannot easily be compared or put into maps etc.
*/
struct RemoteNode
{
  RemoteNode( const Collection &col ) :
    collection( col )
  {}

  Collection collection;
};

Q_DECLARE_METATYPE( RemoteNode* )
static const char REMOTE_NODE[] = "RemoteNode";

/**
 * @internal
 */
class CollectionSync::Private
{
  public:
    Private( CollectionSync *parent ) :
      q( parent ),
      pendingJobs( 0 ),
      progress( 0 ),
      incremental( false ),
      streaming( false ),
      hierarchicalRIDs( false ),
      localListDone( false ),
      deliveryDone( false )
    {
      localRoot = new LocalNode( Collection::root() );
      localRoot->processed = true; // never try to delete that one
      localUidMap.insert( localRoot->collection.id(), localRoot );
      if ( !hierarchicalRIDs )
        localRidMap.insert( QString(), localRoot );
    }

    ~Private()
    {
      delete localRoot;
    }

    /** Create a local node from the given local collection and integrate it into the local tree structure. */
    LocalNode* createLocalNode( const Collection &col )
    {
      if ( col.remoteId().isEmpty() ) // no remote id here means it hasn't been added to the resource yet, so we exclude it from the sync
        return 0;
      LocalNode *node = new LocalNode( col );
      Q_ASSERT( !localUidMap.contains( col.id() ) );
      localUidMap.insert( node->collection.id(), node );
      if ( !hierarchicalRIDs )
        localRidMap.insert( node->collection.remoteId(), node );

      // add already existing children
      if ( localPendingCollections.contains( col.id() ) ) {
        QList<Collection::Id> childIds = localPendingCollections.take( col.id() );
        foreach ( Collection::Id childId, childIds ) {
          Q_ASSERT( localUidMap.contains( childId ) );
          LocalNode *childNode = localUidMap.value( childId );
          node->childNodes.append( childNode );
          node->childRidMap.insert( childNode->collection.remoteId(), childNode );
        }
      }

      // set our parent and add ourselves as child
      if ( localUidMap.contains( col.parentCollection().id() ) ) {
        LocalNode* parentNode = localUidMap.value( col.parentCollection().id() );
        parentNode->childNodes.append( node );
        parentNode->childRidMap.insert( node->collection.remoteId(), node );
      } else {
        localPendingCollections[ col.parentCollection().id() ].append( col.id() );
      }

      return node;
    }

    /** Same as createLocalNode() for remote collections. */
    void createRemoteNode( const Collection &col )
    {
      if ( col.remoteId().isEmpty() ) {
        kWarning() << "Collection '" << col.name() << "' does not have a remote identifier - skipping";
        return;
      }
      RemoteNode *node = new RemoteNode( col );
      localRoot->pendingRemoteNodes.append( node );
    }

    /** Create local nodes as we receive the local listing from the Akonadi server. */
    void localCollectionsReceived( const Akonadi::Collection::List &localCols )
    {
      foreach ( const Collection &c, localCols )
        createLocalNode( c );
    }

    /** Once the local collection listing finished we can continue with the interesting stuff. */
    void localCollectionFetchResult( KJob *job )
    {
      if ( job->error() )
        return; // handled by the base class

      // safety check: the local tree has to be connected
      if ( !localPendingCollections.isEmpty() ) {
        q->setError( Unknown );
        q->setErrorText( i18n( "Inconsistent local collection tree detected." ) );
        q->emitResult();
        return;
      }

      localListDone = true;
      execute();
    }

    /**
      Find the local node that matches the given remote collection, returns 0
      if that doesn't exist (yet).
    */
    LocalNode* findMatchingLocalNode( const Collection &collection )
    {
      if ( !hierarchicalRIDs ) {
        if ( localRidMap.contains( collection.remoteId() ) )
          return localRidMap.value( collection.remoteId() );
        return 0;
      } else {
        if ( collection.id() == Collection::root().id() || collection.remoteId() == Collection::root().remoteId() )
          return localRoot;
        LocalNode *localParent = 0;
        if ( collection.parentCollection().id() < 0 && collection.parentCollection().remoteId().isEmpty() ) {
          kWarning() << "Remote collection without valid parent found: " << collection;
          return 0;
        }
        if ( collection.parentCollection().id() == Collection::root().id() || collection.parentCollection().remoteId() == Collection::root().remoteId() )
          localParent = localRoot;
        else
          localParent = findMatchingLocalNode( collection.parentCollection() );

        if ( localParent && localParent->childRidMap.contains( collection.remoteId() ) )
          return localParent->childRidMap.value( collection.remoteId() );
        return 0;
      }
    }

    /**
      Find the local node that is the nearest ancestor of the given remote collection
      (when using hierarchical RIDs only, otherwise it's always the local root node).
      Never returns 0.
    */
    LocalNode* findBestLocalAncestor( const Collection &collection, bool *exactMatch = 0 )
    {
      if ( !hierarchicalRIDs )
        return localRoot;
      if ( collection == Collection::root() ) {
        if ( exactMatch ) *exactMatch = true;
        return localRoot;
      }
      if ( collection.parentCollection().id() < 0 && collection.parentCollection().remoteId().isEmpty() ) {
        kWarning() << "Remote collection without valid parent found: " << collection;
        return 0;
      }
      bool parentIsExact = false;
      LocalNode *localParent = findBestLocalAncestor( collection.parentCollection(), &parentIsExact );
      if ( !parentIsExact ) {
        if ( exactMatch ) *exactMatch = false;
        return localParent;
      }
      if ( localParent->childRidMap.contains( collection.remoteId() ) ) {
        if ( exactMatch ) *exactMatch = true;
        return localParent->childRidMap.value( collection.remoteId() );
      }
      if ( exactMatch ) *exactMatch = false;
      return localParent;
    }

    /**
      Checks the pending remote nodes attached to the given local root node
      to see if any of them can be processed by now. If not, they are moved to
      the closest ancestor available.
    */
    void processPendingRemoteNodes( LocalNode *_localRoot )
    {
      QList<RemoteNode*> pendingRemoteNodes( _localRoot->pendingRemoteNodes );
      _localRoot->pendingRemoteNodes.clear();
      QHash<LocalNode*, QList<RemoteNode*> > pendingCreations;
      foreach ( RemoteNode *remoteNode, pendingRemoteNodes ) {
        // step 1: see if we have a matching local node already
        LocalNode *localNode = findMatchingLocalNode( remoteNode->collection );
        if ( localNode ) {
          Q_ASSERT( !localNode->processed );
          updateLocalCollection( localNode, remoteNode );
          continue;
        }
        // step 2: check if we have the parent at least, then we can create it
        localNode = findMatchingLocalNode( remoteNode->collection.parentCollection() );
        if ( localNode ) {
          pendingCreations[localNode].append( remoteNode );
          continue;
        }
        // step 3: find the best matching ancestor and enqueue it for later processing
        localNode = findBestLocalAncestor( remoteNode->collection );
        if ( !localNode ) {
          q->setError( Unknown );
          q->setErrorText( i18n( "Remote collection without root-terminated ancestor chain provided, resource is broken." ) );
          q->emitResult();
          return;
        }
        localNode->pendingRemoteNodes.append( remoteNode );
      }

      // process the now possible collection creations
      for ( QHash<LocalNode*, QList<RemoteNode*> >::const_iterator it = pendingCreations.constBegin();
            it != pendingCreations.constEnd(); ++it )
      {
        createLocalCollections( it.key(), it.value() );
      }
    }

    /**
      Performs a local update for the given node pair.
    */
    void updateLocalCollection( LocalNode *localNode, RemoteNode *remoteNode )
    {
      ++pendingJobs;
      Collection upd( remoteNode->collection );
      upd.setId( localNode->collection.id() );
      CollectionModifyJob *mod = new CollectionModifyJob( upd, q );
      connect( mod, SIGNAL(result(KJob*)), q, SLOT(updateLocalCollectionResult(KJob*)) );

      // detecting moves is only possible with global RIDs
      if ( !hierarchicalRIDs ) {
        LocalNode *oldParent = localUidMap.value( localNode->collection.parentCollection().id() );
        LocalNode *newParent = findMatchingLocalNode( remoteNode->collection.parentCollection() );
        // TODO: handle the newParent == 0 case correctly, ie. defer the move until the new
        // local parent has been created
        if ( newParent && oldParent != newParent ) {
          ++pendingJobs;
          CollectionMoveJob *move = new CollectionMoveJob( upd, newParent->collection, q );
          connect( move, SIGNAL(result(KJob*)), q, SLOT(updateLocalCollectionResult(KJob*)) );
        }
      }

      localNode->processed = true;
      delete remoteNode;
    }

    void updateLocalCollectionResult( KJob* job )
    {
      --pendingJobs;
      if ( job->error() )
        return; // handled by the base class
      if ( qobject_cast<CollectionModifyJob*>( job ) )
        ++progress;
      checkDone();
    }

    /**
      Creates local folders for the given local parent and remote nodes.
      @todo group CollectionCreateJobs into a single one once it supports that
    */
    void createLocalCollections( LocalNode* localParent, QList<RemoteNode*> remoteNodes )
    {
      foreach ( RemoteNode *remoteNode, remoteNodes ) {
        ++pendingJobs;
        Collection col( remoteNode->collection );
        col.setParentCollection( localParent->collection );
        CollectionCreateJob *create = new CollectionCreateJob( col, q );
        create->setProperty( LOCAL_NODE, QVariant::fromValue( localParent ) );
        create->setProperty( REMOTE_NODE, QVariant::fromValue( remoteNode ) );
        connect( create, SIGNAL(result(KJob*)), q, SLOT(createLocalCollectionResult(KJob*)) );
      }
    }

    void createLocalCollectionResult( KJob* job )
    {
      --pendingJobs;
      if ( job->error() )
        return; // handled by the base class

      const Collection newLocal = static_cast<CollectionCreateJob*>( job )->collection();
      LocalNode* localNode = createLocalNode( newLocal );
      localNode->processed = true;

      LocalNode* localParent = job->property( LOCAL_NODE ).value<LocalNode*>();
      Q_ASSERT( localParent->childNodes.contains( localNode ) );
      RemoteNode* remoteNode = job->property( REMOTE_NODE ).value<RemoteNode*>();
      delete remoteNode;
      ++progress;

      processPendingRemoteNodes( localParent );
      if ( !hierarchicalRIDs )
        processPendingRemoteNodes( localRoot );

      checkDone();
    }

    /**
      Checks if the given local node has processed child nodes.
    */
    bool hasProcessedChildren( LocalNode *localNode ) const
    {
      if ( localNode->processed )
        return true;
      foreach ( LocalNode *child, localNode->childNodes ) {
        if ( hasProcessedChildren( child ) )
          return true;
      }
      return false;
    }

    /**
      Find all local nodes that are not marked as processed and have no children that
      are marked as processed.
    */
    Collection::List findUnprocessedLocalCollections( LocalNode *localNode ) const
    {
      Collection::List rv;
      if ( !localNode->processed && hasProcessedChildren( localNode ) ) {
        kWarning() << "Found unprocessed local node with processed children, excluding from deletion";
        kWarning() << localNode->collection;
        return rv;
      }
      if ( !localNode->processed ) {
        rv.append( localNode->collection );
        return rv;
      }
      foreach ( LocalNode *child, localNode->childNodes )
        rv.append( findUnprocessedLocalCollections( child ) );
      return rv;
    }

    /**
      Deletes unprocessed local nodes, in non-incremental mode.
    */
    void deleteUnprocessedLocalNodes()
    {
      if ( incremental )
        return;
      const Collection::List cols = findUnprocessedLocalCollections( localRoot );
      deleteLocalCollections( cols );
    }

    /**
      Deletes the given collection list.
      @todo optimite delete job to support batch operations
    */
    void deleteLocalCollections( const Collection::List &cols )
    {
      q->setTotalAmount( KJob::Bytes, q->totalAmount( KJob::Bytes ) + cols.size() );
      foreach ( const Collection &col, cols ) {
        ++pendingJobs;
        CollectionDeleteJob *job = new CollectionDeleteJob( col, q );
        connect( job, SIGNAL(result(KJob*)), q, SLOT(deleteLocalCollectionsResult(KJob*)) );
      }
    }

    void deleteLocalCollectionsResult( KJob *job )
    {
      --pendingJobs;
      if ( job->error() )
        return; // handled by the base class
      ++progress;
      checkDone();
    }

    /**
      Process what's currently available.
    */
    void execute()
    {
      if ( !localListDone )
        return;

      processPendingRemoteNodes( localRoot );

      if ( !incremental && deliveryDone )
        deleteUnprocessedLocalNodes();

      if ( !hierarchicalRIDs ) {
        deleteLocalCollections( removedRemoteCollections );
      } else {
        Collection::List localCols;
        foreach ( const Collection &c, removedRemoteCollections ) {
          LocalNode *node = findMatchingLocalNode( c );
          if ( node )
            localCols.append( node->collection );
        }
        deleteLocalCollections( localCols );
      }
      removedRemoteCollections.clear();

      checkDone();
    }

    /**
      Finds pending remote nodes, which at the end of the day should be an empty set.
    */
    QList<RemoteNode*> findPendingRemoteNodes( LocalNode *localNode )
    {
      QList<RemoteNode*> rv;
      rv.append( localNode->pendingRemoteNodes );
      foreach ( LocalNode *child, localNode->childNodes )
        rv.append( findPendingRemoteNodes( child ) );
      return rv;
    }

    /**
      Are we there yet??
      @todo progress reporting
    */
    void checkDone()
    {
      q->setProcessedAmount( KJob::Bytes, progress );

      // still running jobs or not fully delivered local/remote state
      if ( !deliveryDone || pendingJobs > 0 || !localListDone )
        return;

      // safety check: there must be no pending remote nodes anymore
      QList<RemoteNode*> orphans = findPendingRemoteNodes( localRoot );
      if ( !orphans.isEmpty() ) {
        q->setError( Unknown );
        q->setErrorText( i18n( "Found unresolved orphan collections" ) );
        foreach ( RemoteNode* orphan, orphans )
          kDebug() << "found orphan collection:" << orphan->collection;
        q->emitResult();
        return;
      }

      q->commit();
    }

    CollectionSync *q;

    QString resourceId;

    int pendingJobs;
    int progress;

    LocalNode* localRoot;
    QHash<Collection::Id, LocalNode*> localUidMap;
    QHash<QString, LocalNode*> localRidMap;

    // temporary during build-up of the local node tree, must be empty afterwards
    QHash<Collection::Id, QList<Collection::Id> > localPendingCollections;

    // removed remote collections in incremental mode
    Collection::List removedRemoteCollections;

    bool incremental;
    bool streaming;
    bool hierarchicalRIDs;

    bool localListDone;
    bool deliveryDone;
};

CollectionSync::CollectionSync( const QString &resourceId, QObject *parent ) :
    TransactionSequence( parent ),
    d( new Private( this ) )
{
  d->resourceId = resourceId;
  setTotalAmount( KJob::Bytes, 0 );
}

CollectionSync::~CollectionSync()
{
  delete d;
}

void CollectionSync::setRemoteCollections(const Collection::List & remoteCollections)
{
  setTotalAmount( KJob::Bytes, totalAmount( KJob::Bytes ) + remoteCollections.count() );
  foreach ( const Collection &c, remoteCollections )
    d->createRemoteNode( c );

  if ( !d->streaming )
    d->deliveryDone = true;
  d->execute();
}

void CollectionSync::setRemoteCollections(const Collection::List & changedCollections, const Collection::List & removedCollections)
{
  setTotalAmount( KJob::Bytes, totalAmount( KJob::Bytes ) + changedCollections.count() );
  d->incremental = true;
  foreach ( const Collection &c, changedCollections )
    d->createRemoteNode( c );
  d->removedRemoteCollections += removedCollections;

  if ( !d->streaming )
    d->deliveryDone = true;
  d->execute();
}

void CollectionSync::doStart()
{
  CollectionFetchJob *job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, this );
  job->fetchScope().setResource( d->resourceId );
  job->fetchScope().setAncestorRetrieval( CollectionFetchScope::Parent );
  connect( job, SIGNAL(collectionsReceived(Akonadi::Collection::List)), SLOT(localCollectionsReceived(Akonadi::Collection::List)) );
  connect( job, SIGNAL(result(KJob*)), SLOT(localCollectionFetchResult(KJob*)) );
}

void CollectionSync::setStreamingEnabled( bool streaming )
{
  d->streaming = streaming;
}

void CollectionSync::retrievalDone()
{
  d->deliveryDone = true;
  d->execute();
}

void CollectionSync::setHierarchicalRemoteIds( bool hierarchical )
{
  d->hierarchicalRIDs = hierarchical;
}

#include "collectionsync_p.moc"
