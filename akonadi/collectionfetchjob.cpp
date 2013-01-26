/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#include "collectionfetchjob.h"

#include "imapparser_p.h"
#include "job_p.h"
#include "protocol_p.h"
#include "protocolhelper_p.h"
#include "entity_p.h"
#include "collectionfetchscope.h"
#include "collectionutils_p.h"

#include <kdebug.h>
#include <KLocale>

#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

using namespace Akonadi;

class Akonadi::CollectionFetchJobPrivate : public JobPrivate
{
  public:
    CollectionFetchJobPrivate( CollectionFetchJob *parent )
      : JobPrivate( parent ), mEmitTimer( 0 ), mBasePrefetch( false )
    {

    }

    void init()
    {
      mEmitTimer = new QTimer( q_ptr );
      mEmitTimer->setSingleShot( true );
      mEmitTimer->setInterval( 100 );
      q_ptr->connect( mEmitTimer, SIGNAL(timeout()), q_ptr, SLOT(timeout()) );
      q_ptr->connect( q_ptr, SIGNAL(result(KJob*)), q_ptr, SLOT(timeout()) );
    }

    Q_DECLARE_PUBLIC( CollectionFetchJob )

    CollectionFetchJob::Type mType;
    Collection mBase;
    Collection::List mBaseList;
    Collection::List mCollections;
    CollectionFetchScope mScope;
    Collection::List mPendingCollections;
    QTimer *mEmitTimer;
    bool mBasePrefetch;
    Collection::List mPrefetchList;

    void timeout()
    {
      Q_Q( CollectionFetchJob );

      mEmitTimer->stop(); // in case we are called by result()
      if ( !mPendingCollections.isEmpty() ) {
        if ( !q->error() ) {
          emit q->collectionsReceived( mPendingCollections );
        }
        mPendingCollections.clear();
      }
    }

    void subJobCollectionReceived( const Akonadi::Collection::List &collections )
    {
      mPendingCollections += collections;
      if ( !mEmitTimer->isActive() ) {
        mEmitTimer->start();
      }
    }

    void flushIterativeResult()
    {
      Q_Q( CollectionFetchJob );

      if ( mPendingCollections.isEmpty() ) {
        return;
      }

      emit q->collectionsReceived( mPendingCollections );
      mPendingCollections.clear();
    }
};

CollectionFetchJob::CollectionFetchJob( const Collection &collection, Type type, QObject *parent )
  : Job( new CollectionFetchJobPrivate( this ), parent )
{
  Q_D( CollectionFetchJob );
  d->init();

  d->mBase = collection;
  d->mType = type;
}

CollectionFetchJob::CollectionFetchJob( const Collection::List & cols, QObject * parent )
  : Job( new CollectionFetchJobPrivate( this ), parent )
{
  Q_D( CollectionFetchJob );
  d->init();

  Q_ASSERT( !cols.isEmpty() );
  if ( cols.size() == 1 ) {
    d->mBase = cols.first();
  } else {
    d->mBaseList = cols;
  }
  d->mType = CollectionFetchJob::Base;
}

CollectionFetchJob::CollectionFetchJob( const Collection::List & cols, Type type, QObject * parent )
  : Job( new CollectionFetchJobPrivate( this ), parent )
{
  Q_D( CollectionFetchJob );
  d->init();

  Q_ASSERT( !cols.isEmpty() );
  if ( cols.size() == 1 ) {
    d->mBase = cols.first();
  } else {
    d->mBaseList = cols;
  }
  d->mType = type;
}

CollectionFetchJob::CollectionFetchJob( const QList<Collection::Id> & cols, Type type, QObject * parent )
  : Job( new CollectionFetchJobPrivate( this ), parent )
{
  Q_D( CollectionFetchJob );
  d->init();

  Q_ASSERT( !cols.isEmpty() );
  if ( cols.size() == 1 ) {
    d->mBase = Collection( cols.first() );
  } else {
    foreach ( Collection::Id id, cols ) {
      d->mBaseList.append( Collection( id ) );
    }
  }
  d->mType = type;
}

CollectionFetchJob::~CollectionFetchJob()
{
}

Akonadi::Collection::List CollectionFetchJob::collections() const
{
  Q_D( const CollectionFetchJob );

  return d->mCollections;
}

void CollectionFetchJob::doStart()
{
  Q_D( CollectionFetchJob );

  if ( !d->mBaseList.isEmpty() ) {
    if ( d->mType == Recursive ) {
      // Because doStart starts several subjobs and @p cols could contain descendants of
      // other elements in the list, if type is Recusrive, we could end up with duplicates in the result.
      // To fix this we require an initial fetch of @p cols with Base and RetrieveAncestors,
      // Iterate over that result removing intersections and then perform the Recursive fetch on
      // the remainder.
      d->mBasePrefetch = true;
      // No need to connect to the collectionsReceived signal here. This job is internal. The
      // result needs to be filtered through filterDescendants before it is useful.
      new CollectionFetchJob( d->mBaseList, NonOverlappingRoots, this );
    } else if ( d->mType == NonOverlappingRoots ) {
      foreach ( const Collection &col, d->mBaseList ) {
        // No need to connect to the collectionsReceived signal here. This job is internal. The (aggregated)
        // result needs to be filtered through filterDescendants before it is useful.
        CollectionFetchJob *subJob = new CollectionFetchJob( col, Base, this );
        subJob->fetchScope().setAncestorRetrieval( Akonadi::CollectionFetchScope::All );
      }
    } else {
      foreach ( const Collection &col, d->mBaseList ) {
        CollectionFetchJob *subJob = new CollectionFetchJob( col, d->mType, this );
        connect( subJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), SLOT(subJobCollectionReceived(Akonadi::Collection::List)));
        subJob->setFetchScope( fetchScope() );
      }
    }
    return;
  }

  if ( !d->mBase.isValid() && d->mBase.remoteId().isEmpty() ) {
    setError( Unknown );
    setErrorText( i18n( "Invalid collection given." ) );
    emitResult();
    return;
  }

  QByteArray command = d->newTag();
  if ( !d->mBase.isValid() ) {
    if ( CollectionUtils::hasValidHierarchicalRID( d->mBase ) ) {
      command += " HRID";
    } else {
      command += " " AKONADI_CMD_RID;
    }
  }
  if ( d->mScope.includeUnsubscribed() ) {
    command += " LIST ";
  } else {
    command += " LSUB ";
  }

  if ( d->mBase.isValid() ) {
    command += QByteArray::number( d->mBase.id() );
  } else if ( CollectionUtils::hasValidHierarchicalRID( d->mBase ) ) {
    command += '(' + ProtocolHelper::hierarchicalRidToByteArray( d->mBase ) + ')';
  } else {
    command += ImapParser::quote( d->mBase.remoteId().toUtf8() );
  }

  command += ' ';
  switch ( d->mType ) {
    case Base:
      command += "0 (";
      break;
    case FirstLevel:
      command += "1 (";
      break;
    case Recursive:
      command += "INF (";
      break;
    default:
      Q_ASSERT( false );
  }

  QList<QByteArray> filter;
  if ( !d->mScope.resource().isEmpty() ) {
    filter.append( "RESOURCE" );
    // FIXME: Does this need to be quoted??
    filter.append( d->mScope.resource().toUtf8() );
  }

  if ( !d->mScope.contentMimeTypes().isEmpty() ) {
    filter.append( "MIMETYPE" );
    QList<QByteArray> mts;
    foreach ( const QString &mt, d->mScope.contentMimeTypes() ) {
      // FIXME: Does this need to be quoted??
      mts.append( mt.toUtf8() );
    }
    filter.append( '(' + ImapParser::join( mts, " " ) + ')' );
  }

  QList<QByteArray> options;
  if ( d->mScope.includeStatistics() ) {
    options.append( "STATISTICS" );
    options.append( "true" );
  }
  if ( d->mScope.ancestorRetrieval() != CollectionFetchScope::None ) {
    options.append( "ANCESTORS" );
    switch ( d->mScope.ancestorRetrieval() ) {
      case CollectionFetchScope::None:
        options.append( "0" );
        break;
      case CollectionFetchScope::Parent:
        options.append( "1" );
        break;
      case CollectionFetchScope::All:
        options.append( "INF" );
        break;
      default:
        Q_ASSERT( false );
    }
  }

  command += ImapParser::join( filter, " " ) + ") (" + ImapParser::join( options, " " ) + ")\n";
  d->writeData( command );
}

void CollectionFetchJob::doHandleResponse( const QByteArray & tag, const QByteArray & data )
{
  Q_D( CollectionFetchJob );

  if ( d->mBasePrefetch || d->mType == NonOverlappingRoots ) {
    return;
  }

  if ( tag == "*" ) {
    Collection collection;
    ProtocolHelper::parseCollection( data, collection );
    if ( !collection.isValid() ) {
      return;
    }

    collection.d_ptr->resetChangeLog();
    d->mCollections.append( collection );
    d->mPendingCollections.append( collection );
    if ( !d->mEmitTimer->isActive() ) {
      d->mEmitTimer->start();
    }
    return;
  }
  kDebug() << "Unhandled server response" << tag << data;
}

void CollectionFetchJob::setResource(const QString & resource)
{
  Q_D( CollectionFetchJob );

  d->mScope.setResource( resource );
}

static Collection::List filterDescendants( const Collection::List &list )
{
  Collection::List result;

  QVector<QList<Collection::Id> > ids;
  foreach ( const Collection &collection, list ) {
    QList<Collection::Id> ancestors;
    Collection parent = collection.parentCollection();
    ancestors << parent.id();
    if ( parent != Collection::root() ) {
      while ( parent.parentCollection() != Collection::root() ) {
        parent = parent.parentCollection();
        QList<Collection::Id>::iterator i = qLowerBound( ancestors.begin(), ancestors.end(), parent.id() );
        ancestors.insert( i, parent.id() );
      }
    }
    ids << ancestors;
  }

  QSet<Collection::Id> excludeList;
  foreach ( const Collection &collection, list ) {
    int i = 0;
    foreach ( const QList<Collection::Id> &ancestors, ids ) {
      if ( qBinaryFind( ancestors, collection.id() ) != ancestors.end() ) {
        excludeList.insert( list.at( i ).id() );
      }
      ++i;
    }
  }

  foreach ( const Collection &collection, list ) {
    if ( !excludeList.contains( collection.id() ) ) {
      result.append( collection );
    }
  }

  return result;
}

void CollectionFetchJob::slotResult(KJob * job)
{
  Q_D( CollectionFetchJob );

  CollectionFetchJob *list = qobject_cast<CollectionFetchJob*>( job );
  Q_ASSERT( job );
  if ( d->mBasePrefetch ) {
    d->mBasePrefetch = false;
    const Collection::List roots = list->collections();
    Job::slotResult( job );
    Q_ASSERT( !hasSubjobs() );
    if ( !job->error() ) {
      foreach ( const Collection &col, roots ) {
        CollectionFetchJob *subJob = new CollectionFetchJob( col, d->mType, this );
        connect( subJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), SLOT(subJobCollectionReceived(Akonadi::Collection::List)) );
        subJob->setFetchScope( fetchScope() );
      }
    }
    // No result yet.
  } else if ( d->mType == NonOverlappingRoots ) {
    d->mPrefetchList += list->collections();
    Job::slotResult( job );
    if ( !job->error() && !hasSubjobs() ) {
      const Collection::List result = filterDescendants( d->mPrefetchList );
      d->mPendingCollections += result;
      d->mCollections = result;
      d->flushIterativeResult();
      emitResult();
    }
  } else {
    // We need to tell the subjob to emit its collectionsReceived signal before
    // the result signal is emitted. That will populate my mPendingCollections
    // which will be flushed by calling emitResult which will cause
    // CollectionFetchJobPrivate::timeout to be called.
    list->d_func()->flushIterativeResult();
    d->mCollections += list->collections();
    // Pending collections should have already been emitted by listening to (and flushing) the job.
    Job::slotResult( job );
    if ( !job->error() && !hasSubjobs() ) {
      emitResult();
    }
  }
}

void CollectionFetchJob::includeUnsubscribed(bool include)
{
  Q_D( CollectionFetchJob );

  d->mScope.setIncludeUnsubscribed( include );
}

void CollectionFetchJob::includeStatistics(bool include)
{
  Q_D( CollectionFetchJob );

  d->mScope.setIncludeStatistics( include );
}

void CollectionFetchJob::setFetchScope( const CollectionFetchScope &scope )
{
  Q_D( CollectionFetchJob );
  d->mScope = scope;
}

CollectionFetchScope& CollectionFetchJob::fetchScope()
{
  Q_D( CollectionFetchJob );
  return d->mScope;
}

#include "moc_collectionfetchjob.cpp"
