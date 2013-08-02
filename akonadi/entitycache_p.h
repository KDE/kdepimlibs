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

#ifndef AKONADI_ENTITYCACHE_P_H
#define AKONADI_ENTITYCACHE_P_H

#include <akonadi/item.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/collection.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/session.h>

#include "akonadiprivate_export.h"

#include <qobject.h>
#include <QQueue>
#include <QVariant>
#include <QHash>
#include <QtCore/QQueue>

class KJob;

typedef QList<Akonadi::Entity::Id> EntityIdList;
Q_DECLARE_METATYPE(QList<Akonadi::Entity::Id>)

namespace Akonadi {

/**
  @internal
  QObject part of EntityCache.
*/
class AKONADI_TESTS_EXPORT EntityCacheBase : public QObject
{
  Q_OBJECT
  public:
    explicit EntityCacheBase ( Session *session, QObject * parent = 0 );

    void setSession(Session *session);

  protected:
    Session *session;

  signals:
    void dataAvailable();

  private slots:
    virtual void processResult( KJob* job ) = 0;
};

template <typename T>
struct EntityCacheNode
{
  EntityCacheNode() : pending( false ), invalid( false ) {}
  EntityCacheNode( typename T::Id id ) : entity( T( id ) ), pending( true ), invalid( false ) {}
  T entity;
  bool pending;
  bool invalid;
};

/**
 * @internal
 * A in-memory FIFO cache for a small amount of Entity objects.
 */
template<typename T, typename FetchJob, typename FetchScope_>
class EntityCache : public EntityCacheBase
{
  public:
    typedef FetchScope_ FetchScope;
    explicit EntityCache( int maxCapacity, Session *session = 0, QObject *parent = 0 ) :
      EntityCacheBase( session, parent ),
      mCapacity( maxCapacity )
    {}

    ~EntityCache()
    {
      qDeleteAll( mCache );
    }

    /** Object is available in the cache and can be retrieved. */
    bool isCached( typename T::Id id ) const
    {
      EntityCacheNode<T>* node = cacheNodeForId( id );
      return node && !node->pending;
    }

    /** Object has been requested but is not yet loaded into the cache or is already available. */
    bool isRequested( typename T::Id id ) const
    {
      return cacheNodeForId( id );
    }

    /** Returns the cached object if available, an empty instance otherwise. */
    virtual T retrieve( typename T::Id id ) const
    {
      EntityCacheNode<T>* node = cacheNodeForId( id );
      if ( node && !node->pending && !node->invalid ) {
        return node->entity;
      }
      return T();
    }

    /** Marks the cache entry as invalid, use in case the object has been deleted on the server. */
    void invalidate( typename T::Id id )
    {
      EntityCacheNode<T>* node = cacheNodeForId( id );
      if ( node ) {
        node->invalid = true;
      }
    }

    /** Triggers a re-fetching of a cache entry, use if it has changed on the server. */
    void update( typename T::Id id, const FetchScope &scope )
    {
      EntityCacheNode<T>* node = cacheNodeForId( id );
      if ( node ) {
        mCache.removeAll( node );
        if ( node->pending ) {
          request( id, scope );
        }
        delete node;
      }
    }

    /** Requests the object to be cached if it is not yet in the cache. @returns @c true if it was in the cache already. */
    virtual bool ensureCached( typename T::Id id, const FetchScope &scope )
    {
      EntityCacheNode<T>* node = cacheNodeForId( id );
      if ( !node ) {
        request( id, scope );
        return false;
      }
      return !node->pending;
    }

    /**
      Asks the cache to retrieve @p id. @p request is used as
      a token to indicate which request has been finished in the
      dataAvailable() signal.
    */
    virtual void request( typename T::Id id, const FetchScope &scope )
    {
      Q_ASSERT( !isRequested( id ) );
      shrinkCache();
      EntityCacheNode<T> *node = new EntityCacheNode<T>( id );
      FetchJob* job = createFetchJob( id );
      job->setFetchScope( scope );
      job->setProperty( "EntityCacheNode", QVariant::fromValue<typename T::Id>( id ) );
      connect( job, SIGNAL( result( KJob* )), SLOT(processResult( KJob* ) ) );
      mCache.enqueue( node );
    }

  private:
    EntityCacheNode<T>* cacheNodeForId( typename T::Id id ) const
    {
      for ( typename QQueue<EntityCacheNode<T>*>::const_iterator it = mCache.constBegin(), endIt = mCache.constEnd();
            it != endIt; ++it ) {
        if ( ( *it )->entity.id() == id ) {
          return *it;
        }
      }
      return 0;
    }

    void processResult( KJob* job )
    {
      // Error handling?
      typename T::Id id = job->property( "EntityCacheNode" ).template value<typename T::Id>();
      EntityCacheNode<T> *node = cacheNodeForId( id );
      if ( !node ) {
        return; // got replaced in the meantime
      }

      node->pending = false;
      extractResult( node, job );
      // make sure we find this node again if something went wrong here,
      // most likely the object got deleted from the server in the meantime
      if ( node->entity.id() != id ) {
        // TODO: Recursion guard? If this is called with non-existing ids, the if will never be true!
        node->entity.setId( id );
        node->invalid = true;
      }
      emit dataAvailable();
    }

    void extractResult( EntityCacheNode<T>* node, KJob* job ) const;

    inline FetchJob* createFetchJob( typename T::Id id )
    {
      return new FetchJob( T( id ), session );
    }

    /** Tries to reduce the cache size until at least one more object fits in. */
    void shrinkCache()
    {
      while ( mCache.size() >= mCapacity && !mCache.first()->pending ) {
        delete mCache.dequeue();
      }
    }

  private:
    QQueue<EntityCacheNode<T>*> mCache;
    int mCapacity;
};

template<> inline void EntityCache<Collection, CollectionFetchJob, CollectionFetchScope>::extractResult( EntityCacheNode<Collection>* node, KJob *job ) const
{
  CollectionFetchJob* fetch = qobject_cast<CollectionFetchJob*>( job );
  Q_ASSERT( fetch );
  if ( fetch->collections().isEmpty() ) {
    node->entity = Collection();
  } else {
    node->entity = fetch->collections().first();
  }
}

template<> inline void EntityCache<Item, ItemFetchJob, ItemFetchScope>::extractResult( EntityCacheNode<Item>* node, KJob *job ) const
{
  ItemFetchJob* fetch = qobject_cast<ItemFetchJob*>( job );
  Q_ASSERT( fetch );
  if ( fetch->items().isEmpty() ) {
    node->entity = Item();
  } else {
    node->entity = fetch->items().first();
  }
}

template<> inline CollectionFetchJob* EntityCache<Collection, CollectionFetchJob, CollectionFetchScope>::createFetchJob( Collection::Id id )
{
  return new CollectionFetchJob( Collection( id ), CollectionFetchJob::Base, session );
}

typedef EntityCache<Collection, CollectionFetchJob, CollectionFetchScope> CollectionCache;
typedef EntityCache<Item, ItemFetchJob, ItemFetchScope> ItemCache;

template <typename T>
struct EntityListCacheNode
{
  EntityListCacheNode() : pending( false ), invalid( false ) {}
  EntityListCacheNode( typename T::Id id ) : entity(id), pending( true ), invalid( false ) {}

  T entity;
  bool pending;
  bool invalid;
};

template<typename T, typename FetchJob, typename FetchScope_>
class EntityListCache : public EntityCacheBase
{
public:
  typedef FetchScope_ FetchScope;

  explicit EntityListCache( int maxCapacity, Session *session = 0, QObject *parent = 0 ) :
    EntityCacheBase( session, parent ),
    mCapacity( maxCapacity )
  {}

  ~EntityListCache()
  {
    qDeleteAll( mCache );
  }

  /** Returns the cached object if available, an empty instance otherwise. */
  typename T::List retrieve( const QList<Entity::Id> &ids ) const
  {
    typename T::List list;

    foreach( Entity::Id id, ids ) {
      EntityListCacheNode<T>* node = mCache.value( id );
      if ( !node || node->pending || node->invalid )
        return typename T::List();

      list << node->entity;
    }

    return list;
  }

  /** Requests the object to be cached if it is not yet in the cache. @returns @c true if it was in the cache already. */
  bool ensureCached( const QList<Entity::Id> &ids, const FetchScope &scope )
  {
    QList<Entity::Id> toRequest;
    bool result = true;

    foreach( Entity::Id id, ids ) {
      EntityListCacheNode<T>* node = mCache.value( id );
      if ( !node ) {
        toRequest << id;
        continue;
      }

      if ( node->pending )
        result = false;
    }

    if ( !toRequest.isEmpty() ) {
      request( toRequest, scope, ids );
      return false;
    }

    return result;
  }

  /** Marks the cache entry as invalid, use in case the object has been deleted on the server. */
  void invalidate( const QList<Entity::Id> &ids )
  {
    foreach( Entity::Id id, ids ) {
      EntityListCacheNode<T>* node = mCache.value( id );
      if ( node ) {
        node->invalid = true;
      }
    }
  }

  /** Triggers a re-fetching of a cache entry, use if it has changed on the server. */
  void update( const QList<Entity::Id> &ids, const FetchScope &scope )
  {
    QList<Entity::Id> toRequest;

    foreach( Entity::Id id, ids ) {
      EntityListCacheNode<T>* node = mCache.value( id );
      if ( node ) {
        mCache.remove( id );
        if ( node->pending ) {
          toRequest << id;
        }
        delete node;
      }
    }

    if ( !toRequest.isEmpty() ) {
      request( toRequest, scope );
    }
  }

  /**
    Asks the cache to retrieve @p id. @p request is used as
    a token to indicate which request has been finished in the
    dataAvailable() signal.
  */
  void request( const QList<Entity::Id> &ids, const FetchScope &scope, const QList<Entity::Id> &preserveIds = QList<Entity::Id>() )
  {
    Q_ASSERT( isNotRequested( ids ) );
    shrinkCache( preserveIds );
    foreach( Entity::Id id, ids ) {
      EntityListCacheNode<T> *node = new EntityListCacheNode<T>( id );
      mCache.insert( id, node );
    }
    FetchJob* job = createFetchJob( ids );
    job->setFetchScope( scope );
    job->setProperty( "EntityListCacheIds", QVariant::fromValue< QList<Entity::Id> >( ids ) );
    connect( job, SIGNAL(result(KJob*)), SLOT(processResult(KJob*)) );
  }

  bool isNotRequested( const QList<Entity::Id> &ids ) const
  {
    foreach( Entity::Id id, ids ) {
      if ( mCache.contains( id ) )
        return false;
    }

    return true;
  }

  /** Object is available in the cache and can be retrieved. */
  bool isCached( const QList<Entity::Id> &ids ) const
  {
    foreach( Entity::Id id, ids ) {
      EntityListCacheNode<T>* node = mCache.value( id );
      if ( !node || node->pending ) {
        return false;
      }
    }
    return true;
  }

private:
  /** Tries to reduce the cache size until at least one more object fits in. */
  void shrinkCache( const QList<Entity::Id> &preserveIds )
  {
    typename
    QHash< Entity::Id, EntityListCacheNode<T>* >::Iterator iter = mCache.begin();
    while ( iter != mCache.end() && mCache.size() >= mCapacity ) {
      if ( iter.value()->pending || preserveIds.contains( iter.key() ) ) {
        ++iter;
        continue;
      }

      delete iter.value();
      iter = mCache.erase( iter );
    }
  }

  inline FetchJob* createFetchJob( const QList<Entity::Id> &ids )
  {
    return new FetchJob( ids, session );
  }

  void processResult( KJob* job )
  {
    const QList<Entity::Id> ids = job->property( "EntityListCacheIds" ).value< QList<Entity::Id> >();

    typename T::List entities;
    extractResults( job, entities );

    foreach( Entity::Id id, ids ) {
      EntityListCacheNode<T> *node = mCache.value( id );
      if ( !node ) {
        continue; // got replaced in the meantime
      }

      node->pending = false;

      T result;
      typename T::List::Iterator iter = entities.begin();
      for ( ; iter != entities.end(); ++iter ) {
        if ( (*iter).id() == id ) {
          result = *iter;
          entities.erase( iter );
          break;
        }
      }

      // make sure we find this node again if something went wrong here,
      // most likely the object got deleted from the server in the meantime
      if ( !result.isValid() ) {
        node->entity = Item( id );
        node->invalid = true;
      } else {
        node->entity = result;
      }
    }

    emit dataAvailable();
  }

  void extractResults( KJob* job, typename T::List &entities ) const;

private:
  QHash< Entity::Id, EntityListCacheNode<T>* > mCache;
  int mCapacity;
};

template<> inline void EntityListCache<Collection, CollectionFetchJob, CollectionFetchScope>::extractResults( KJob *job, Collection::List &collections ) const
{
  CollectionFetchJob* fetch = qobject_cast<CollectionFetchJob*>( job );
  Q_ASSERT( fetch );
  collections = fetch->collections();
}

template<> inline void EntityListCache<Item, ItemFetchJob, ItemFetchScope>::extractResults( KJob *job, Item::List &items ) const
{
  ItemFetchJob* fetch = qobject_cast<ItemFetchJob*>( job );
  Q_ASSERT( fetch );
  items = fetch->items();
}

template<>
inline CollectionFetchJob* EntityListCache<Collection, CollectionFetchJob, CollectionFetchScope>::createFetchJob( const QList<Entity::Id> &ids )
{
  return new CollectionFetchJob( ids, CollectionFetchJob::Base, session );
}

typedef EntityListCache<Collection, CollectionFetchJob, CollectionFetchScope> CollectionListCache;
typedef EntityListCache<Item, ItemFetchJob, ItemFetchScope> ItemListCache;

}

#endif
