/*
    Copyright (c) 2006 Till Adam <adam@kde.org>
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

#include "resourcebase.h"
#include "agentbase_p.h"

#include "resourceadaptor.h"
#include "collectiondeletejob.h"
#include "collectionsync_p.h"
#include "itemsync.h"
#include "resourcescheduler_p.h"
#include "tracerinterface.h"
#include "xdgbasedirs_p.h"

#include "changerecorder.h"
#include "collectionfetchjob.h"
#include "collectionfetchscope.h"
#include "collectionmodifyjob.h"
#include "itemfetchjob.h"
#include "itemfetchscope.h"
#include "itemmodifyjob.h"
#include "itemmodifyjob_p.h"
#include "session.h"
#include "resourceselectjob_p.h"
#include "monitor_p.h"
#include "servermanager_p.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtDBus/QtDBus>

using namespace Akonadi;

class Akonadi::ResourceBasePrivate : public AgentBasePrivate
{
  Q_OBJECT
  Q_CLASSINFO( "D-Bus Interface", "org.kde.dfaure" )

  public:
    ResourceBasePrivate( ResourceBase *parent )
      : AgentBasePrivate( parent ),
        scheduler( 0 ),
        mItemSyncer( 0 ),
        mItemTransactionMode( ItemSync::SingleTransaction ),
        mCollectionSyncer( 0 ),
        mHierarchicalRid( false )
    {
      Internal::setClientType( Internal::Resource );
      mStatusMessage = defaultReadyMessage();
    }

    Q_DECLARE_PUBLIC( ResourceBase )

    void delayedInit()
    {
      if ( !QDBusConnection::sessionBus().registerService( QLatin1String( "org.freedesktop.Akonadi.Resource." ) + mId ) ) {
        QString reason = QDBusConnection::sessionBus().lastError().message();
        if ( reason.isEmpty() ) {
          reason = QString::fromLatin1( "this service is probably running already." );
        }
        kError() << "Unable to register service at D-Bus: " << reason;
        QCoreApplication::instance()->exit(1);
      } else {
        AgentBasePrivate::delayedInit();
      }
    }

    virtual void changeProcessed()
    {
      mChangeRecorder->changeProcessed();
      if ( !mChangeRecorder->isEmpty() )
        scheduler->scheduleChangeReplay();
      scheduler->taskDone();
    }

    void slotDeliveryDone( KJob* job );
    void slotCollectionSyncDone( KJob *job );
    void slotLocalListDone( KJob *job );
    void slotSynchronizeCollection( const Collection &col );
    void slotCollectionListDone( KJob *job );

    void slotItemSyncDone( KJob *job );

    void slotPercent( KJob* job, unsigned long percent );
    void slotDeleteResourceCollection();
    void slotDeleteResourceCollectionDone( KJob *job );
    void slotCollectionDeletionDone( KJob *job );

    void slotPrepareItemRetrieval( const Akonadi::Item &item );
    void slotPrepareItemRetrievalResult( KJob* job );

    void changeCommittedResult( KJob* job );

  public Q_SLOTS:
    Q_SCRIPTABLE void dump()
    {
      scheduler->dump();
    }

    Q_SCRIPTABLE void clear()
    {
      scheduler->clear();
    }

  public:
    // synchronize states
    Collection currentCollection;

    ResourceScheduler *scheduler;
    ItemSync *mItemSyncer;
    ItemSync::TransactionMode mItemTransactionMode;
    CollectionSync *mCollectionSyncer;
    bool mHierarchicalRid;
};

ResourceBase::ResourceBase( const QString & id )
  : AgentBase( new ResourceBasePrivate( this ), id )
{
  Q_D( ResourceBase );

  new Akonadi__ResourceAdaptor( this );

  d->scheduler = new ResourceScheduler( this );

  d->mChangeRecorder->setChangeRecordingEnabled( true );
  connect( d->mChangeRecorder, SIGNAL( changesAdded() ),
           d->scheduler, SLOT( scheduleChangeReplay() ) );

  d->mChangeRecorder->setResourceMonitored( d->mId.toLatin1() );

  connect( d->scheduler, SIGNAL( executeFullSync() ),
           SLOT( retrieveCollections() ) );
  connect( d->scheduler, SIGNAL( executeCollectionTreeSync() ),
           SLOT( retrieveCollections() ) );
  connect( d->scheduler, SIGNAL( executeCollectionSync( const Akonadi::Collection& ) ),
           SLOT( slotSynchronizeCollection( const Akonadi::Collection& ) ) );
  connect( d->scheduler, SIGNAL( executeItemFetch( const Akonadi::Item&, const QSet<QByteArray>& ) ),
           SLOT( slotPrepareItemRetrieval(Akonadi::Item)) );
  connect( d->scheduler, SIGNAL( executeResourceCollectionDeletion() ),
           SLOT( slotDeleteResourceCollection() ) );
  connect( d->scheduler, SIGNAL( status( int, const QString& ) ),
           SIGNAL( status( int, const QString& ) ) );
  connect( d->scheduler, SIGNAL( executeChangeReplay() ),
           d->mChangeRecorder, SLOT( replayNext() ) );
  connect( d->scheduler, SIGNAL( fullSyncComplete() ), SIGNAL( synchronized() ) );
  connect( d->mChangeRecorder, SIGNAL( nothingToReplay() ), d->scheduler, SLOT( taskDone() ) );
  connect( d->mChangeRecorder, SIGNAL( collectionRemoved( const Akonadi::Collection& ) ),
           d->scheduler, SLOT( collectionRemoved( const Akonadi::Collection& ) ) );
  connect( this, SIGNAL( synchronized() ), d->scheduler, SLOT( taskDone() ) );
  connect( this, SIGNAL( agentNameChanged( const QString& ) ),
           this, SIGNAL( nameChanged( const QString& ) ) );

  d->scheduler->setOnline( d->mOnline );
  if ( !d->mChangeRecorder->isEmpty() )
    d->scheduler->scheduleChangeReplay();

  QDBusConnection::sessionBus().registerObject( dbusPathPrefix() + QLatin1String( "/Debug" ), d, QDBusConnection::ExportScriptableSlots );

  new ResourceSelectJob( identifier() );
}

ResourceBase::~ResourceBase()
{
}

void ResourceBase::synchronize()
{
  d_func()->scheduler->scheduleFullSync();
}

void ResourceBase::setName( const QString &name )
{
  AgentBase::setAgentName( name );
}

QString ResourceBase::name() const
{
  return AgentBase::agentName();
}

QString ResourceBase::parseArguments( int argc, char **argv )
{
  QString identifier;
  if ( argc < 3 ) {
    kDebug() << "Not enough arguments passed...";
    exit( 1 );
  }

  for ( int i = 1; i < argc - 1; ++i ) {
    if ( QLatin1String( argv[ i ] ) == QLatin1String( "--identifier" ) )
      identifier = QLatin1String( argv[ i + 1 ] );
  }

  if ( identifier.isEmpty() ) {
    kDebug() << "Identifier argument missing";
    exit( 1 );
  }

  QByteArray catalog;
  char *p = strrchr( argv[0], '/' );
  if ( p )
    catalog = QByteArray( p + 1 );
  else
    catalog = QByteArray( argv[0] );

  KCmdLineArgs::init( argc, argv, identifier.toLatin1(), catalog,
                      ki18nc( "@title application name", "Akonadi Resource" ), "0.1",
                      ki18nc( "@title application description", "Akonadi Resource" ) );

  KCmdLineOptions options;
  options.add( "identifier <argument>",
               ki18nc( "@label commandline option", "Resource identifier" ) );
  KCmdLineArgs::addCmdLineOptions( options );

  return identifier;
}

int ResourceBase::init( ResourceBase *r )
{
  QApplication::setQuitOnLastWindowClosed( false );
  int rv = kapp->exec();
  delete r;
  return rv;
}

void ResourceBase::itemRetrieved( const Item &item )
{
  Q_D( ResourceBase );
  Q_ASSERT( d->scheduler->currentTask().type == ResourceScheduler::FetchItem );
  if ( !item.isValid() ) {
    d->scheduler->currentTask().sendDBusReplies( false );
    d->scheduler->taskDone();
    return;
  }

  Item i( item );
  QSet<QByteArray> requestedParts = d->scheduler->currentTask().itemParts;
  foreach ( const QByteArray &part, requestedParts ) {
    if ( !item.loadedPayloadParts().contains( part ) ) {
      kWarning() << "Item does not provide part" << part;
    }
  }

  ItemModifyJob *job = new ItemModifyJob( i );
  // FIXME: remove once the item with which we call retrieveItem() has a revision number
  job->disableRevisionCheck();
  connect( job, SIGNAL( result( KJob* ) ), SLOT( slotDeliveryDone( KJob* ) ) );
}

void ResourceBasePrivate::slotDeliveryDone(KJob * job)
{
  Q_Q( ResourceBase );
  Q_ASSERT( scheduler->currentTask().type == ResourceScheduler::FetchItem );
  if ( job->error() ) {
    emit q->error( QLatin1String( "Error while creating item: " ) + job->errorString() );
  }
  scheduler->currentTask().sendDBusReplies( !job->error() );
  scheduler->taskDone();
}

void ResourceBasePrivate::slotDeleteResourceCollection()
{
  Q_Q( ResourceBase );

  CollectionFetchJob *job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::FirstLevel );
  job->fetchScope().setResource( q->identifier() );
  connect( job, SIGNAL( result( KJob* ) ), q, SLOT( slotDeleteResourceCollectionDone( KJob* ) ) );
}

void ResourceBasePrivate::slotDeleteResourceCollectionDone( KJob *job )
{
  Q_Q( ResourceBase );
  if ( job->error() ) {
    emit q->error( job->errorString() );
    scheduler->taskDone();
  } else {
    const CollectionFetchJob *fetchJob = static_cast<const CollectionFetchJob*>( job );

    if ( !fetchJob->collections().isEmpty() ) {
      CollectionDeleteJob *job = new CollectionDeleteJob( fetchJob->collections().first() );
      connect( job, SIGNAL( result( KJob* ) ), q, SLOT( slotCollectionDeletionDone( KJob* ) ) );
    } else {
      // there is no resource collection, so just ignore the request
      scheduler->taskDone();
    }
  }
}

void ResourceBasePrivate::slotCollectionDeletionDone( KJob *job )
{
  Q_Q( ResourceBase );
  if ( job->error() ) {
    emit q->error( job->errorString() );
  }

  scheduler->taskDone();
}

void ResourceBase::changeCommitted( const Item& item )
{
  Q_D( ResourceBase );
  ItemModifyJob *job = new ItemModifyJob( item );
  job->d_func()->setClean();
  job->disableRevisionCheck(); // TODO: remove, but where/how do we handle the error?
  job->ignorePayload(); // we only want to reset the dirty flag and update the remote id
  d->changeProcessed();
}

void ResourceBase::changeCommitted( const Collection &collection )
{
  CollectionModifyJob *job = new CollectionModifyJob( collection );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( changeCommittedResult( KJob* ) ) );
}

void ResourceBasePrivate::changeCommittedResult( KJob *job )
{
  Q_Q( ResourceBase );
  if ( job->error() )
    emit q->error( i18nc( "@info", "Updating local collection failed: %1.", job->errorText() ) );
  mChangeRecorder->d_ptr->invalidateCache( static_cast<CollectionModifyJob*>( job )->collection() );
  changeProcessed();
}

bool ResourceBase::requestItemDelivery( qint64 uid, const QString & remoteId,
                                        const QString &mimeType, const QStringList &_parts )
{
  Q_D( ResourceBase );
  if ( !isOnline() ) {
    emit error( i18nc( "@info", "Cannot fetch item in offline mode." ) );
    return false;
  }

  setDelayedReply( true );
  // FIXME: we need at least the revision number too
  Item item( uid );
  item.setMimeType( mimeType );
  item.setRemoteId( remoteId );

  QSet<QByteArray> parts;
  Q_FOREACH( const QString &str, _parts )
    parts.insert( str.toLatin1() );

  d->scheduler->scheduleItemFetch( item, parts, message().createReply() );

  return true;
}

void ResourceBase::collectionsRetrieved( const Collection::List & collections )
{
  Q_D( ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollectionTree ||
              d->scheduler->currentTask().type == ResourceScheduler::SyncAll,
              "ResourceBase::collectionsRetrieved()",
              "Calling collectionsRetrieved() although no collection retrieval is in progress" );
  if ( !d->mCollectionSyncer ) {
    d->mCollectionSyncer = new CollectionSync( identifier() );
    d->mCollectionSyncer->setHierarchicalRemoteIds( d->mHierarchicalRid );
    connect( d->mCollectionSyncer, SIGNAL( percent( KJob*, unsigned long ) ), SLOT( slotPercent( KJob*, unsigned long ) ) );
    connect( d->mCollectionSyncer, SIGNAL( result( KJob* ) ), SLOT( slotCollectionSyncDone( KJob* ) ) );
  }
  d->mCollectionSyncer->setRemoteCollections( collections );
}

void ResourceBase::collectionsRetrievedIncremental( const Collection::List & changedCollections,
                                                    const Collection::List & removedCollections )
{
  Q_D( ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollectionTree ||
              d->scheduler->currentTask().type == ResourceScheduler::SyncAll,
              "ResourceBase::collectionsRetrievedIncremental()",
              "Calling collectionsRetrievedIncremental() although no collection retrieval is in progress" );
  if ( !d->mCollectionSyncer ) {
    d->mCollectionSyncer = new CollectionSync( identifier() );
    d->mCollectionSyncer->setHierarchicalRemoteIds( d->mHierarchicalRid );
    connect( d->mCollectionSyncer, SIGNAL( percent( KJob*, unsigned long ) ), SLOT( slotPercent( KJob*, unsigned long ) ) );
    connect( d->mCollectionSyncer, SIGNAL( result( KJob* ) ), SLOT( slotCollectionSyncDone( KJob* ) ) );
  }
  d->mCollectionSyncer->setRemoteCollections( changedCollections, removedCollections );
}

void ResourceBase::setCollectionStreamingEnabled( bool enable )
{
  Q_D( ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollectionTree ||
              d->scheduler->currentTask().type == ResourceScheduler::SyncAll,
              "ResourceBase::setCollectionStreamingEnabled()",
              "Calling setCollectionStreamingEnabled() although no collection retrieval is in progress" );
  if ( !d->mCollectionSyncer ) {
    d->mCollectionSyncer = new CollectionSync( identifier() );
    d->mCollectionSyncer->setHierarchicalRemoteIds( d->mHierarchicalRid );
    connect( d->mCollectionSyncer, SIGNAL( percent( KJob*, unsigned long ) ), SLOT( slotPercent( KJob*, unsigned long ) ) );
    connect( d->mCollectionSyncer, SIGNAL( result( KJob* ) ), SLOT( slotCollectionSyncDone( KJob* ) ) );
  }
  d->mCollectionSyncer->setStreamingEnabled( enable );
}

void ResourceBase::collectionsRetrievalDone()
{
  Q_D( ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollectionTree ||
              d->scheduler->currentTask().type == ResourceScheduler::SyncAll,
              "ResourceBase::collectionsRetrievalDone()",
              "Calling collectionsRetrievalDone() although no collection retrieval is in progress" );
  // streaming enabled, so finalize the sync
  if ( d->mCollectionSyncer ) {
    d->mCollectionSyncer->retrievalDone();
  }
  // user did the sync himself, we are done now
  else {
    // FIXME: we need the same special case for SyncAll as in slotCollectionSyncDone here!
    d->scheduler->taskDone();
  }
}

void ResourceBasePrivate::slotCollectionSyncDone( KJob * job )
{
  Q_Q( ResourceBase );
  mCollectionSyncer = 0;
  if ( job->error() ) {
    if ( job->error() != Job::UserCanceled )
      emit q->error( job->errorString() );
  } else {
    if ( scheduler->currentTask().type == ResourceScheduler::SyncAll ) {
      CollectionFetchJob *list = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive );
      list->setFetchScope( q->changeRecorder()->collectionFetchScope() );
      list->fetchScope().setResource( mId );
      q->connect( list, SIGNAL( result( KJob* ) ), q, SLOT( slotLocalListDone( KJob* ) ) );
      return;
    }
  }
  scheduler->taskDone();
}

void ResourceBasePrivate::slotLocalListDone( KJob * job )
{
  Q_Q( ResourceBase );
  if ( job->error() ) {
    emit q->error( job->errorString() );
  } else {
    Collection::List cols = static_cast<CollectionFetchJob*>( job )->collections();
    foreach ( const Collection &col, cols ) {
      scheduler->scheduleSync( col );
    }
    scheduler->scheduleFullSyncCompletion();
  }
  scheduler->taskDone();
}

void ResourceBasePrivate::slotSynchronizeCollection( const Collection &col )
{
  Q_Q( ResourceBase );
  currentCollection = col;
  // check if this collection actually can contain anything
  QStringList contentTypes = currentCollection.contentMimeTypes();
  contentTypes.removeAll( Collection::mimeType() );
  if ( !contentTypes.isEmpty() || (col.rights() & (Collection::CanLinkItem)) ) { // HACK to check for virtual collections
    emit q->status( AgentBase::Running, i18nc( "@info:status", "Syncing collection '%1'", currentCollection.name() ) );
    q->retrieveItems( currentCollection );
    return;
  }
  scheduler->taskDone();
}

void ResourceBasePrivate::slotPrepareItemRetrieval( const Akonadi::Item &item )
{
  Q_Q( ResourceBase );
  ItemFetchJob *fetch = new ItemFetchJob( item, this );
  fetch->fetchScope().setAncestorRetrieval( q->changeRecorder()->itemFetchScope().ancestorRetrieval() );
  fetch->fetchScope().setCacheOnly( true );

  // copy list of attributes to fetch
  const QSet<QByteArray> attributes = q->changeRecorder()->itemFetchScope().attributes();
  foreach ( const QByteArray &attribute, attributes )
    fetch->fetchScope().fetchAttribute( attribute );

  q->connect( fetch, SIGNAL( result( KJob* ) ), SLOT( slotPrepareItemRetrievalResult( KJob* ) ) );
}

void ResourceBasePrivate::slotPrepareItemRetrievalResult( KJob* job )
{
  Q_Q( ResourceBase );
  Q_ASSERT_X( scheduler->currentTask().type == ResourceScheduler::FetchItem,
            "ResourceBasePrivate::slotPrepareItemRetrievalResult()",
            "Preparing item retrieval although no item retrieval is in progress" );
  if ( job->error() ) {
    q->cancelTask( job->errorText() );
    return;
  }
  ItemFetchJob *fetch = qobject_cast<ItemFetchJob*>( job );
  if ( fetch->items().count() != 1 ) {
    q->cancelTask( i18n( "The requested item no longer exists" ) );
    return;
  }
  const Item item = fetch->items().first();
  const QSet<QByteArray> parts = scheduler->currentTask().itemParts;
  if ( !q->retrieveItem( item, parts ) )
    q->cancelTask();
}

void ResourceBase::itemsRetrievalDone()
{
  Q_D( ResourceBase );
  // streaming enabled, so finalize the sync
  if ( d->mItemSyncer ) {
    d->mItemSyncer->deliveryDone();
  }
  // user did the sync himself, we are done now
  else {
    d->scheduler->taskDone();
  }
}

void ResourceBase::clearCache()
{
  Q_D( ResourceBase );
  d->scheduler->scheduleResourceCollectionDeletion();
}

Collection ResourceBase::currentCollection() const
{
  Q_D( const ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollection ,
              "ResourceBase::currentCollection()",
              "Trying to access current collection although no item retrieval is in progress" );
  return d->currentCollection;
}

Item ResourceBase::currentItem() const
{
  Q_D( const ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::FetchItem ,
              "ResourceBase::currentItem()",
              "Trying to access current item although no item retrieval is in progress" );
  return d->scheduler->currentTask().item;
}

void ResourceBase::synchronizeCollectionTree()
{
  d_func()->scheduler->scheduleCollectionTreeSync();
}

void ResourceBase::cancelTask()
{
  Q_D( ResourceBase );
  switch ( d->scheduler->currentTask().type ) {
    case ResourceScheduler::FetchItem:
      itemRetrieved( Item() ); // sends the error reply and
      break;
    case ResourceScheduler::ChangeReplay:
      d->changeProcessed();
      break;
    case ResourceScheduler::SyncCollectionTree:
    case ResourceScheduler::SyncAll:
      if ( d->mCollectionSyncer )
        d->mCollectionSyncer->rollback();
      else
        d->scheduler->taskDone();
      break;
    case ResourceScheduler::SyncCollection:
      if ( d->mItemSyncer )
        d->mItemSyncer->rollback();
      else
        d->scheduler->taskDone();
      break;
    default:
      d->scheduler->taskDone();
  }
}

void ResourceBase::cancelTask( const QString &msg )
{
  cancelTask();

  emit error( msg );
}

void ResourceBase::deferTask()
{
  Q_D( ResourceBase );
  d->scheduler->deferTask();
}

void ResourceBase::doSetOnline( bool state )
{
  d_func()->scheduler->setOnline( state );
}

void ResourceBase::synchronizeCollection( qint64 collectionId )
{
  CollectionFetchJob* job = new CollectionFetchJob( Collection( collectionId ), CollectionFetchJob::Base );
  job->setFetchScope( changeRecorder()->collectionFetchScope() );
  job->fetchScope().setResource( identifier() );
  connect( job, SIGNAL( result( KJob* ) ), SLOT( slotCollectionListDone( KJob* ) ) );
}

void ResourceBasePrivate::slotCollectionListDone( KJob *job )
{
  if ( !job->error() ) {
    Collection::List list = static_cast<CollectionFetchJob*>( job )->collections();
    if ( !list.isEmpty() ) {
      Collection col = list.first();
      scheduler->scheduleSync( col );
    }
  }
  // TODO: error handling
}

void ResourceBase::setTotalItems( int amount )
{
  kDebug() << amount;
  Q_D( ResourceBase );
  setItemStreamingEnabled( true );
  d->mItemSyncer->setTotalItems( amount );
}

void ResourceBase::setItemStreamingEnabled( bool enable )
{
  Q_D( ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollection,
              "ResourceBase::setItemStreamingEnabled()",
              "Calling setItemStreamingEnabled() although no item retrieval is in progress" );
  if ( !d->mItemSyncer ) {
    d->mItemSyncer = new ItemSync( currentCollection() );
    d->mItemSyncer->setTransactionMode( d->mItemTransactionMode );
    connect( d->mItemSyncer, SIGNAL( percent( KJob*, unsigned long ) ), SLOT( slotPercent( KJob*, unsigned long ) ) );
    connect( d->mItemSyncer, SIGNAL( result( KJob* ) ), SLOT( slotItemSyncDone( KJob* ) ) );
  }
  d->mItemSyncer->setStreamingEnabled( enable );
}

void ResourceBase::itemsRetrieved( const Item::List &items )
{
  Q_D( ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollection,
              "ResourceBase::itemsRetrieved()",
              "Calling itemsRetrieved() although no item retrieval is in progress" );
  if ( !d->mItemSyncer ) {
    d->mItemSyncer = new ItemSync( currentCollection() );
    d->mItemSyncer->setTransactionMode( d->mItemTransactionMode );
    connect( d->mItemSyncer, SIGNAL( percent( KJob*, unsigned long ) ), SLOT( slotPercent( KJob*, unsigned long ) ) );
    connect( d->mItemSyncer, SIGNAL( result( KJob* ) ), SLOT( slotItemSyncDone( KJob* ) ) );
  }
  d->mItemSyncer->setFullSyncItems( items );
}

void ResourceBase::itemsRetrievedIncremental( const Item::List &changedItems, const Item::List &removedItems )
{
  Q_D( ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollection,
              "ResourceBase::itemsRetrievedIncremental()",
              "Calling itemsRetrievedIncremental() although no item retrieval is in progress" );
  if ( !d->mItemSyncer ) {
    d->mItemSyncer = new ItemSync( currentCollection() );
    d->mItemSyncer->setTransactionMode( d->mItemTransactionMode );
    connect( d->mItemSyncer, SIGNAL( percent( KJob*, unsigned long ) ), SLOT( slotPercent( KJob*, unsigned long ) ) );
    connect( d->mItemSyncer, SIGNAL( result( KJob* ) ), SLOT( slotItemSyncDone( KJob* ) ) );
  }
  d->mItemSyncer->setIncrementalSyncItems( changedItems, removedItems );
}

void ResourceBasePrivate::slotItemSyncDone( KJob *job )
{
  mItemSyncer = 0;
  Q_Q( ResourceBase );
  if ( job->error() && job->error() != Job::UserCanceled ) {
    emit q->error( job->errorString() );
  }
  scheduler->taskDone();
}

void ResourceBasePrivate::slotPercent( KJob *job, unsigned long percent )
{
  Q_Q( ResourceBase );
  Q_UNUSED( job );
  emit q->percent( percent );
}

void ResourceBase::setHierarchicalRemoteIdentifiersEnabled( bool enable )
{
  Q_D( ResourceBase );
  d->mHierarchicalRid = enable;
}

void ResourceBase::scheduleCustomTask( QObject *receiver, const char *method, const QVariant &argument, SchedulePriority priority )
{
  Q_D( ResourceBase );
  d->scheduler->scheduleCustomTask( receiver, method, argument, priority );
}

void ResourceBase::taskDone()
{
  Q_D( ResourceBase );
  d->scheduler->taskDone();
}

void ResourceBase::setItemTransactionMode(ItemSync::TransactionMode mode)
{
  Q_D( ResourceBase );
  d->mItemTransactionMode = mode;
}


#include "resourcebase.moc"
#include "moc_resourcebase.cpp"
