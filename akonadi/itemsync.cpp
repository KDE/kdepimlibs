/*
    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>
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

#include "itemsync.h"

#include "collection.h"
#include "item.h"
#include "item_p.h"
#include "itemcreatejob.h"
#include "itemdeletejob.h"
#include "itemfetchjob.h"
#include "itemmodifyjob.h"
#include "transactionsequence.h"
#include "itemfetchscope.h"

#include <kdebug.h>

#include <QtCore/QStringList>

using namespace Akonadi;

/**
 * @internal
 */
class ItemSync::Private
{
  public:
    Private( ItemSync *parent ) :
      q( parent ),
      mTransactionMode( SingleTransaction ),
      mCurrentTransaction( 0 ),
      mTransactionJobs( 0 ),
      mPendingJobs( 0 ),
      mProgress( 0 ),
      mTotalItems( -1 ),
      mTotalItemsProcessed( 0 ),
      mStreaming( false ),
      mIncremental( false ),
      mLocalListDone( false ),
      mDeliveryDone( false ),
      mFinished( false )
    {
      // we want to fetch all data by default
      mFetchScope.fetchFullPayload();
      mFetchScope.fetchAllAttributes();
    }

    void createLocalItem( const Item &item );
    void checkDone();
    void slotLocalListDone( KJob* );
    void slotLocalDeleteDone( KJob* );
    void slotLocalChangeDone( KJob* );
    void execute();
    void processItems();
    void deleteItems( const Item::List &items );
    void slotTransactionResult( KJob *job );
    Job* subjobParent() const;

    ItemSync *q;
    Collection mSyncCollection;
    QHash<Item::Id, Akonadi::Item> mLocalItemsById;
    QHash<QString, Akonadi::Item> mLocalItemsByRemoteId;
    QSet<Akonadi::Item> mUnprocessedLocalItems;

    ItemSync::TransactionMode mTransactionMode;
    TransactionSequence *mCurrentTransaction;
    int mTransactionJobs;

    // fetch scope for initial item listing
    ItemFetchScope mFetchScope;

    // remote items
    Akonadi::Item::List mRemoteItems;

    // removed remote items
    Item::List mRemovedRemoteItems;

    // create counter
    int mPendingJobs;
    int mProgress;
    int mTotalItems;
    int mTotalItemsProcessed;

    bool mStreaming;
    bool mIncremental;
    bool mLocalListDone;
    bool mDeliveryDone;
    bool mFinished;
};

void ItemSync::Private::createLocalItem( const Item & item )
{
  // don't try to do anything in error state
  if ( q->error() )
    return;
  mPendingJobs++;
  ItemCreateJob *create = new ItemCreateJob( item, mSyncCollection, subjobParent() );
  q->connect( create, SIGNAL(result(KJob*)), q, SLOT(slotLocalChangeDone(KJob*)) );
}

void ItemSync::Private::checkDone()
{
  q->setProcessedAmount( KJob::Bytes, mProgress );
  if ( mPendingJobs > 0 || !mDeliveryDone || mTransactionJobs > 0 )
    return;

  if ( !mFinished ) { // prevent double result emission, can happen since checkDone() is called from all over the place
    mFinished = true;
    q->emitResult();
  }
}

ItemSync::ItemSync( const Collection &collection, QObject *parent ) :
    Job( parent ),
    d( new Private( this ) )
{
  d->mSyncCollection = collection;
}

ItemSync::~ItemSync()
{
  delete d;
}

void ItemSync::setFullSyncItems( const Item::List &items )
{
  Q_ASSERT( !d->mIncremental );
  if ( !d->mStreaming )
    d->mDeliveryDone = true;
  d->mRemoteItems += items;
  d->mTotalItemsProcessed += items.count();
  kDebug() << "Received: " << items.count() << "In total: " << d->mTotalItemsProcessed << " Wanted: " << d->mTotalItems;
  setTotalAmount( KJob::Bytes, d->mTotalItemsProcessed );
  if ( d->mTotalItemsProcessed == d->mTotalItems )
    d->mDeliveryDone = true;
  d->execute();
}

void ItemSync::setTotalItems( int amount )
{
  Q_ASSERT( !d->mIncremental );
  Q_ASSERT( amount >= 0 );
  setStreamingEnabled( true );
  kDebug() << amount;
  d->mTotalItems = amount;
  setTotalAmount( KJob::Bytes, amount );
  if ( d->mTotalItems == 0 ) {
    d->mDeliveryDone = true;
    d->execute();
  }
}

void ItemSync::setIncrementalSyncItems( const Item::List &changedItems, const Item::List &removedItems )
{
  d->mIncremental = true;
  if ( !d->mStreaming )
    d->mDeliveryDone = true;
  d->mRemoteItems += changedItems;
  d->mRemovedRemoteItems += removedItems;
  d->mTotalItemsProcessed += changedItems.count() + removedItems.count();
  setTotalAmount( KJob::Bytes, d->mTotalItemsProcessed );
  if ( d->mTotalItemsProcessed == d->mTotalItems )
    d->mDeliveryDone = true;
  d->execute();
}

void ItemSync::setFetchScope( ItemFetchScope &fetchScope )
{
  d->mFetchScope = fetchScope;
}

ItemFetchScope &ItemSync::fetchScope()
{
  return d->mFetchScope;
}

void ItemSync::doStart()
{
  ItemFetchJob* job = new ItemFetchJob( d->mSyncCollection, this );
  job->setFetchScope( d->mFetchScope );

  // we only can fetch parts already in the cache, otherwise this will deadlock
  job->fetchScope().setCacheOnly( true );

  connect( job, SIGNAL(result(KJob*)), SLOT(slotLocalListDone(KJob*)) );
}

bool ItemSync::updateItem( const Item &storedItem, Item &newItem )
{
  // we are in error state, better not change anything at all anymore
  if ( error() )
    return false;

  /*
   * We know that this item has changed (as it is part of the
   * incremental changed list), so we just put it into the
   * storage.
   */
  if ( d->mIncremental )
    return true;

  if ( newItem.d_func()->mClearPayload )
    return true;

  // Check whether the remote revisions differ
  if ( storedItem.remoteRevision() != newItem.remoteRevision() )
    return true;

  // Check whether the flags differ
  if ( storedItem.flags() != newItem.flags() ) {
    kDebug() << "Stored flags "  << storedItem.flags()
             << "new flags " << newItem.flags();
    return true;
  }

  // Check whether the new item contains unknown parts
  QSet<QByteArray> missingParts = newItem.loadedPayloadParts();
  missingParts.subtract( storedItem.loadedPayloadParts() );
  if ( !missingParts.isEmpty() )
    return true;

  // ### FIXME SLOW!!!
  // If the available part identifiers don't differ, check
  // whether the content of the payload differs
  if ( newItem.hasPayload()
    && storedItem.payloadData() != newItem.payloadData() )
    return true;

  // check if remote attributes have been changed
  foreach ( Attribute* attr, newItem.attributes() ) {
    if ( !storedItem.hasAttribute( attr->type() ) )
      return true;
    if ( attr->serialized() != storedItem.attribute( attr->type() )->serialized() )
      return true;
  }

  return false;
}

void ItemSync::Private::slotLocalListDone( KJob * job )
{
  if ( !job->error() ) {
    const Item::List list = static_cast<ItemFetchJob*>( job )->items();
    foreach ( const Item &item, list ) {
      if ( item.remoteId().isEmpty() )
        continue;
      mLocalItemsById.insert( item.id(), item );
      mLocalItemsByRemoteId.insert( item.remoteId(), item );
      mUnprocessedLocalItems.insert( item );
    }
  }

  mLocalListDone = true;
  execute();
}

void ItemSync::Private::execute()
{
  if ( !mLocalListDone )
    return;

  // early exit to avoid unnecessary TransactionSequence creation in MultipleTransactions mode
  // TODO: do the transaction handling in a nicer way instead, only creating TransactionSequences when really needed
  if ( !mDeliveryDone && mRemoteItems.isEmpty() )
    return;

  if ( (mTransactionMode == SingleTransaction && !mCurrentTransaction) || mTransactionMode == MultipleTransactions) {
    ++mTransactionJobs;
    mCurrentTransaction = new TransactionSequence( q );
    mCurrentTransaction->setAutomaticCommittingEnabled( false );
    connect( mCurrentTransaction, SIGNAL(result(KJob*)), q, SLOT(slotTransactionResult(KJob*)) );
  }

  processItems();
  if ( !mDeliveryDone ) {
    if ( mTransactionMode == MultipleTransactions && mCurrentTransaction ) {
      mCurrentTransaction->commit();
      mCurrentTransaction = 0;
    }
    return;
  }

  // removed
  if ( !mIncremental ) {
    mRemovedRemoteItems = mUnprocessedLocalItems.toList();
    mUnprocessedLocalItems.clear();
  }

  deleteItems( mRemovedRemoteItems );
  mLocalItemsById.clear();
  mLocalItemsByRemoteId.clear();
  mRemovedRemoteItems.clear();

  if ( mCurrentTransaction ) {
    mCurrentTransaction->commit();
    mCurrentTransaction = 0;
  }

  checkDone();
}

void ItemSync::Private::processItems()
{
  // added / updated
  foreach ( Item remoteItem, mRemoteItems ) { //krazy:exclude=foreach non-const is needed here
#ifndef NDEBUG
    if ( remoteItem.remoteId().isEmpty() ) {
      kWarning() << "Item " << remoteItem.id() << " does not have a remote identifier";
    }
#endif

    Item localItem = mLocalItemsById.value( remoteItem.id() );
    if ( !localItem.isValid() )
      localItem = mLocalItemsByRemoteId.value( remoteItem.remoteId() );
    mUnprocessedLocalItems.remove( localItem );
    // missing locally
    if ( !localItem.isValid() ) {
      createLocalItem( remoteItem );
      continue;
    }

    if ( q->updateItem( localItem, remoteItem ) ) {
      mPendingJobs++;

      remoteItem.setId( localItem.id() );
      remoteItem.setRevision( localItem.revision() );
      remoteItem.setSize( localItem.size() );
      remoteItem.setRemoteId( localItem.remoteId() );  // in case someone clears remoteId by accident
      ItemModifyJob *mod = new ItemModifyJob( remoteItem, subjobParent() );
      mod->disableRevisionCheck();
      q->connect( mod, SIGNAL(result(KJob*)), q, SLOT(slotLocalChangeDone(KJob*)) );
    } else {
      mProgress++;
    }
  }
  mRemoteItems.clear();
}

void ItemSync::Private::deleteItems( const Item::List &items )
{
  // if in error state, better not change anything anymore
  if ( q->error() )
    return;

  Item::List itemsToDelete;
  foreach ( const Item &item, items ) {
    Item delItem( item );
    if ( !item.isValid() ) {
      delItem = mLocalItemsByRemoteId.value( item.remoteId() );
    }

    if ( !delItem.isValid() ) {
#ifndef NDEBUG
      kWarning() << "Delete item (remoteeId=" << item.remoteId()
                 << "mimeType=" << item.mimeType()
                 << ") does not have a valid UID and no item with that remote ID exists either";
#endif
      continue;
    }

    if ( delItem.remoteId().isEmpty() ) {
      // don't attempt to remove items that never were written to the backend
      continue;
    }

    itemsToDelete.append ( delItem );
  }

  if ( !itemsToDelete.isEmpty() ) {
    mPendingJobs++;
    ItemDeleteJob *job = new ItemDeleteJob( itemsToDelete, subjobParent() );
    q->connect( job, SIGNAL(result(KJob*)), q, SLOT(slotLocalDeleteDone(KJob*)) );

    // It can happen that the groupware servers report us deleted items
    // twice, in this case this item delete job will fail on the second try.
    // To avoid a rollback of the complete transaction we gracefully allow the job
    // to fail :)
    TransactionSequence *transaction = qobject_cast<TransactionSequence*>( subjobParent() );
    if ( transaction )
      transaction->setIgnoreJobFailure( job );
  }
}

void ItemSync::Private::slotLocalDeleteDone( KJob* )
{
  mPendingJobs--;
  mProgress++;

  checkDone();
}

void ItemSync::Private::slotLocalChangeDone( KJob * job )
{
  Q_UNUSED( job );
  mPendingJobs--;
  mProgress++;

  checkDone();
}

void ItemSync::Private::slotTransactionResult( KJob *job )
{
  --mTransactionJobs;
  if ( mCurrentTransaction == job )
    mCurrentTransaction = 0;

  checkDone();
}

Job * ItemSync::Private::subjobParent() const
{
  if ( mCurrentTransaction && mTransactionMode != NoTransaction )
    return mCurrentTransaction;
  return q;
}

void ItemSync::setStreamingEnabled(bool enable)
{
  d->mStreaming = enable;
}

void ItemSync::deliveryDone()
{
  Q_ASSERT( d->mStreaming );
  d->mDeliveryDone = true;
  d->execute();
}

void ItemSync::slotResult(KJob* job)
{
  if ( job->error() ) {
    // pretent there were no errors
    Akonadi::Job::removeSubjob( job );
    // propagate the first error we got but continue, we might still be fed with stuff from a resource
    if ( !error() ) {
      setError( job->error() );
      setErrorText( job->errorText() );
    }
  } else {
    Akonadi::Job::slotResult( job );
  }
}

void ItemSync::rollback()
{
  setError( UserCanceled );
  if ( d->mCurrentTransaction )
    d->mCurrentTransaction->rollback();
  d->mDeliveryDone = true; // user wont deliver more data
  d->execute(); // end this in an ordered way, since we have an error set no real change will be done
}

void ItemSync::setTransactionMode(ItemSync::TransactionMode mode)
{
  d->mTransactionMode = mode;
}


#include "moc_itemsync.cpp"
