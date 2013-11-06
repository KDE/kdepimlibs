/*
    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

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

// @cond PRIVATE

#include "monitor_p.h"

#include "collectionfetchjob.h"
#include "collectionstatistics.h"
#include "dbusconnectionpool.h"
#include "itemfetchjob.h"
#include "idle_p.h"
#include "session.h"
#include "idlejob_p.h"

#include <kdebug.h>
#include <kcomponentdata.h>

#include <QCoreApplication>

using namespace Akonadi;

static int IdleSessionsCounter = 0;

MonitorPrivate::MonitorPrivate( Monitor * parent) :
  q_ptr( parent ),
  mFetchChangedOnly( false ),
  fetchCollection( false ),
  fetchCollectionStatistics( false ),
  collectionMoveTranslationEnabled( true ),
  useRefCounting( false )
{

}

MonitorPrivate::~MonitorPrivate()
{
}

void MonitorPrivate::init()
{
  statisticsCompressionTimer.setSingleShot( true );
  statisticsCompressionTimer.setInterval( 500 );
  QObject::connect( &statisticsCompressionTimer, SIGNAL(timeout()), q_ptr, SLOT(slotFlushRecentlyChangedCollections()) );

  // FIXME : This is not really a good approach to get a unique yet persistent
  //         ID for the IDLE session, but should work just fine for current
  //         purposes
  session = new Akonadi::Session( qAppName().toLatin1() + "-IDLE-" + QByteArray::number( IdleSessionsCounter ), q_ptr );
  IdleSessionsCounter++;


  idleJob = new IdleJob( session );
  QObject::connect( idleJob, SIGNAL(notify(IdleNotification)),
                    q_ptr, SLOT(slotNotify(IdleNotification)) );
  /*
   * QObject::connect( job, SIGNAL(notify(IdleNotificiation)),
                q_ptr, SLOT(emitCollectionNotification(IdleNotificiation)) );
  */
}

void MonitorPrivate::slotNotify( const IdleNotification &notification )
{
  bool needsSplit = false, batchSupported = false;

  checkBatchSupport( notification, needsSplit, batchSupported );
  kDebug() << needsSplit << batchSupported;

  QList<IdleNotification> notifications;
  if ( needsSplit ) {
    notifications = splitNotification( notification, false );
  }

  if ( !needsSplit || batchSupported ) {
    notifications << notification;
  }

  Q_FOREACH ( const IdleNotification &notification, notifications ) {
    // TODO: What about calling this via QTimer::singleShot() - would that improve
    // responsiveness of apps during high-load?

    if ( notification.type() == Idle::Item ) {
      emitItemsNotification( notification );
    } else {
      emitCollectionNotification( notification );
    }
  }
}

void MonitorPrivate::checkBatchSupport(const IdleNotification& msg, bool &needsSplit, bool &batchSupported) const
{
  const bool isBatch = (msg.items().count() > 1);

  if ( msg.type() == Idle::Item ) {
    switch ( msg.operation() ) {
      case Idle::Add:
        needsSplit = isBatch;
        batchSupported = false;
        return;
      case Idle::Modify:
        needsSplit = isBatch;
        batchSupported = false;
        return;
      case Idle::ModifyFlags:
        batchSupported = q_ptr->receivers( SIGNAL(itemsFlagsChanged(Akonadi::Item::List,QSet<QByteArray>,QSet<QByteArray>)) ) > 0;
        needsSplit = isBatch && !batchSupported && q_ptr->receivers( SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)) ) > 0;
        return;
      case Idle::Move:
        needsSplit = isBatch && q_ptr->receivers( SIGNAL(itemMoved(Akonadi::Item,Akonadi::Collection,Akonadi::Collection)) ) > 0;
        batchSupported = q_ptr->receivers( SIGNAL(itemsMoved(Akonadi::Item::List,Akonadi::Collection,Akonadi::Collection)) ) > 0;
        return;
      case Idle::Remove:
        needsSplit = isBatch && q_ptr->receivers( SIGNAL(itemRemoved(Akonadi::Item)) ) > 0;
        batchSupported = q_ptr->receivers( SIGNAL(itemsRemoved(Akonadi::Item::List)) ) > 0;
        return;
      case Idle::Link:
        needsSplit = isBatch && q_ptr->receivers( SIGNAL(itemLinked(Akonadi::Item,Akonadi::Collection)) ) > 0;
        batchSupported = q_ptr->receivers( SIGNAL(itemsLinked(Akonadi::Item::List,Akonadi::Collection)) ) > 0;
        return;
      case Idle::Unlink:
        needsSplit = isBatch && q_ptr->receivers( SIGNAL(itemUnlinked(Akonadi::Item,Akonadi::Collection)) ) > 0;
        batchSupported = q_ptr->receivers( SIGNAL(itemsUnlinked(Akonadi::Item::List,Akonadi::Collection)) ) > 0;
        return;
      default:
        needsSplit = isBatch;
        batchSupported = false;
        kDebug() << "Unknown operation type" << msg.operation() << "in item change notification";
        return;
    }
  } else if ( msg.type() == Idle::Collection ) {
    needsSplit = isBatch;
    batchSupported = false;
  }
}

QList<IdleNotification> MonitorPrivate::splitNotification(const IdleNotification& msg, bool legacy) const
{
  QList<IdleNotification> list;

  IdleNotification baseMsg = msg;
  baseMsg.setItems( QList<Item>() );
  if ( msg.operation() == Idle::ModifyFlags ) {
    baseMsg.setOperation( Idle::Modify );
    baseMsg.setChangedParts( QSet<QByteArray>() << "FLAGS" );
  }

  Q_FOREACH( const Item &item, msg.items() ) {
    IdleNotification copy = baseMsg;
    copy.setItems(QList<Item>() << item );
    list << copy;
  }

  return list;
}

void MonitorPrivate::updatePendingStatistics( const IdleNotification& msg )
{
  if ( msg.type() == Idle::Item ) {
    notifyCollectionStatisticsWatchers( msg.sourceCollection(), msg.resource() );
    // FIXME use the proper resource of the target collection, for cross resource moves
    notifyCollectionStatisticsWatchers( msg.destinationCollection(), msg.destinationResource() );
  } else if ( msg.type() == Idle::Collection && msg.operation() == Idle::Remove ) {
    // no need for statistics updates anymore
    Q_FOREACH( const Item &item, msg.items() ) {
      recentlyChangedCollections.remove( item.id() );
    }
  }
}

void MonitorPrivate::slotStatisticsChangedFinished( KJob* job )
{
  if ( job->error() ) {
    kWarning() << "Error on fetching collection statistics: " << job->errorText();
  } else {
    CollectionStatisticsJob *statisticsJob = static_cast<CollectionStatisticsJob*>( job );
    Q_ASSERT( statisticsJob->collection().isValid() );
    emit q_ptr->collectionStatisticsChanged( statisticsJob->collection().id(),
                                             statisticsJob->statistics() );
  }
}

void MonitorPrivate::slotFlushRecentlyChangedCollections()
{
  foreach ( Collection::Id collection, recentlyChangedCollections ) {
    Q_ASSERT( collection >= 0 );
    if ( fetchCollectionStatistics ) {
      fetchStatistics( collection );
    } else {
      static const CollectionStatistics dummyStatistics;
      emit q_ptr->collectionStatisticsChanged( collection, dummyStatistics );
    }
  }
  recentlyChangedCollections.clear();
}

void MonitorPrivate::slotSessionDestroyed( QObject *object )
{
  Session *session = static_cast<Akonadi::Session*>( session );
  if ( ignoredSessions.contains( session ) ) {
    idleJob->removeIgnoredSession( ignoredSessions.value( session ) );
    ignoredSessions.remove( session );
  }
}


//
// TODO: Move this to server
//
/*
int MonitorPrivate::translateAndCompress( QQueue<NotificationMessageV2> &notificationQueue, const NotificationMessageV2 &msg  )
{
  // We have to split moves into insert or remove if the source or destination
  // is not monitored.
  if ( msg.operation() != NotificationMessageV2::Move ) {
    return NotificationMessageV2::appendAndCompress( notificationQueue, msg ) ? 1 : 0;
  }

  bool sourceWatched = false;
  bool destWatched = false;

  if ( useRefCounting && msg.type() == NotificationMessageV2::Items ) {
    sourceWatched = refCountMap.contains( msg.parentCollection() ) || m_buffer.isBuffered( msg.parentCollection() );
    destWatched = refCountMap.contains( msg.parentDestCollection() ) || m_buffer.isBuffered( msg.parentDestCollection() );
  } else {
    if ( !resources.isEmpty() ) {
      sourceWatched = resources.contains( msg.resource() );
      destWatched = isMoveDestinationResourceMonitored( msg );
    }
    if ( !sourceWatched )
      sourceWatched = isCollectionMonitored( msg.parentCollection() );
    if ( !destWatched )
      destWatched = isCollectionMonitored( msg.parentDestCollection() );
  }

  if ( !sourceWatched && !destWatched ) {
    return 0;
  }

  if ( ( sourceWatched && destWatched ) || ( !collectionMoveTranslationEnabled && msg.type() == NotificationMessageV2::Collections ) ) {
    return NotificationMessageV2::appendAndCompress( notificationQueue, msg ) ? 1 : 0;
  }

  if ( sourceWatched )
  {
    // Transform into a removal
    NotificationMessageV2 removalMessage = msg;
    removalMessage.setOperation( NotificationMessageV2::Remove );
    removalMessage.setParentDestCollection( -1 );
    return NotificationMessageV2::appendAndCompress( notificationQueue, removalMessage ) ? 1 : 0;
  }

  // Transform into an insertion
  NotificationMessageV2 insertionMessage = msg;
  insertionMessage.setOperation( NotificationMessageV2::Add );
  insertionMessage.setParentCollection( msg.parentDestCollection() );
  insertionMessage.setParentDestCollection( -1 );
  // We don't support batch insertion, so we have to do it one by one
  const NotificationMessageV2::List split = splitMessage( insertionMessage, false );
  int appended = 0;
  Q_FOREACH (const NotificationMessageV2 &insertion, split ) {
    if ( NotificationMessageV2::appendAndCompress( notificationQueue, insertion ) ) {
      ++appended;
    }
  }
  return appended;
}
*/

/*

  server notification --> ?accepted --> pendingNotifications --> ?dataAvailable --> emit
                                  |                                           |
                                  x --> discard                               x --> pipeline

  fetchJobDone --> pipeline ?dataAvailable --> emit
 */

/*
void MonitorPrivate::slotNotify( const NotificationMessageV2::List &msgs )
{
  int appendedMessages = 0;
  int modifiedMessages = 0;
  int erasedMessages = 0;
  foreach ( const NotificationMessageV2 &msg, msgs ) {
    invalidateCaches( msg );
    updatePendingStatistics( msg );
    bool needsSplit = true;
    bool supportsBatch = false;

    if ( isLazilyIgnored( msg, true ) ) {
      continue;
    }

    checkBatchSupport( msg, needsSplit, supportsBatch );

    if ( supportsBatch
        || ( !needsSplit && !supportsBatch && msg.operation() != NotificationMessageV2::ModifyFlags )
        || msg.type() == NotificationMessageV2::Collections ) {
      // Make sure the batch msg is always queued before the split notifications
      const int oldSize = pendingNotifications.size();
      const int appended = translateAndCompress( pendingNotifications, msg );
      if ( appended > 0 ) {
        appendedMessages += appended;
      } else {
        ++modifiedMessages;
      }
      // translateAndCompress can remove an existing "modify" when msg is a "delete".
      // Or it can merge two ModifyFlags and return false.
      // We need to detect such removals, for ChangeRecorder.
      if ( pendingNotifications.count() != oldSize + appended ) {
        ++erasedMessages; // this count isn't exact, but it doesn't matter
      }
    } else if ( needsSplit ) {
      // If it's not queued at least make sure we fetch all the items from split
      // notifications in one go.
      itemCache->ensureCached( msg.uids(), mItemFetchScope );
    }

    // if the message contains more items, but we need to emit single-item notification,
    // split the message into one message per item and queue them
    // if the message contains only one item, but batches are not supported
    // (and thus neither is flagsModified), splitMessage() will convert the
    // notification to regular Modify with "FLAGS" part changed
    if ( needsSplit || ( !needsSplit && !supportsBatch && msg.operation() == Akonadi::NotificationMessageV2::ModifyFlags ) ) {
      const NotificationMessageV2::List split = splitMessage( msg, !supportsBatch );
      pendingNotifications << split.toList();
      appendedMessages += split.count();
    }
  }

  // tell ChangeRecorder (even if 0 appended, the compression could have made changes to existing messages)
  if ( appendedMessages > 0 || modifiedMessages > 0 || erasedMessages > 0 ) {
    if ( erasedMessages > 0 )
      notificationsErased();
    else
      notificationsEnqueued( appendedMessages );
  }

  dispatchNotifications();
}
*/

bool MonitorPrivate::emitItemsNotification( const IdleNotification &msg )
{
  kDebug() << msg.type() << msg.operation();
  bool handled = false;
  switch ( msg.operation() ) {
    case Idle::Add:
      if ( q_ptr->receivers( SIGNAL(itemAdded(Akonadi::Item,Akonadi::Collection)) ) > 0 ) {
        Q_ASSERT( msg.items().count() == 1 );
        emit q_ptr->itemAdded( msg.items().first(), Collection( msg.destinationCollection() ) );
        return true;
      }
      return false;
    case Idle::Modify:
      if ( q_ptr->receivers( SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)) ) > 0 ) {
        Q_ASSERT( msg.items().count() == 1 );
        const Item item = msg.items().first();
        emit q_ptr->itemChanged( item, msg.changedParts() );
        return true;
      }
      return false;
    case Idle::ModifyFlags: {
      int rc = q_ptr->receivers( SIGNAL(itemsFlagsChanged(Akonadi::Item::List,QSet<QByteArray>,QSet<QByteArray>)) );
      if ( q_ptr->receivers( SIGNAL(itemsFlagsChanged(Akonadi::Item::List,QSet<QByteArray>,QSet<QByteArray>)) ) > 0 ) {
        emit q_ptr->itemsFlagsChanged( msg.items(), msg.addedFlags(), msg.removedFlags() );
        handled = true;
      }
      return handled;
    }
    case Idle::Move:
      if ( q_ptr->receivers( SIGNAL(itemMoved(Akonadi::Item,Akonadi::Collection,Akonadi::Collection)) ) > 0 ) {
        Q_ASSERT( msg.items().count() == 1 );
        emit q_ptr->itemMoved( msg.items().first(), Collection( msg.sourceCollection() ), Collection( msg.destinationCollection() ) );
        handled = true;
      }
      if ( q_ptr->receivers( SIGNAL(itemsMoved(Akonadi::Item::List,Akonadi::Collection,Akonadi::Collection)) ) > 0 ) {
        emit q_ptr->itemsMoved( msg.items(), Collection( msg.sourceCollection() ), Collection( msg.destinationCollection() ) );
        handled = true;
      }
      return handled;
    case Idle::Remove:
      if ( q_ptr->receivers( SIGNAL(itemRemoved(Akonadi::Item)) ) > 0 ) {
        Q_ASSERT( msg.items().count() == 1 );
        emit q_ptr->itemRemoved( msg.items().first() );
        handled = true;
      }
      if ( q_ptr->receivers( SIGNAL(itemsRemoved(Akonadi::Item::List)) ) > 0 ) {
        emit q_ptr->itemsRemoved( msg.items() );
        handled = true;
      }
      return handled;
    case Idle::Link:
      if ( q_ptr->receivers( SIGNAL(itemLinked(Akonadi::Item,Akonadi::Collection)) ) > 0 ) {
        Q_ASSERT( msg.items().count() == 1 );
        emit q_ptr->itemLinked( msg.items().first(), Collection( msg.destinationCollection() ) );
        handled = true;
      }
      if ( q_ptr->receivers( SIGNAL(itemsLinked(Akonadi::Item::List,Akonadi::Collection)) ) > 0 ) {
        emit q_ptr->itemsLinked( msg.items(), Collection( msg.destinationCollection() ) );
        handled = true;
      }
      return handled;
    case Idle::Unlink:
      if ( q_ptr->receivers( SIGNAL(itemUnlinked(Akonadi::Item,Akonadi::Collection)) ) > 0 ) {
        Q_ASSERT( msg.items().count() == 1 );
        emit q_ptr->itemUnlinked( msg.items().first(), Collection( msg.destinationCollection() ) );
        handled = true;
      }
      if ( q_ptr->receivers( SIGNAL(itemsUnlinked(Akonadi::Item::List,Akonadi::Collection)) ) > 0 ) {
        emit q_ptr->itemsUnlinked( msg.items(), Collection( msg.destinationCollection() ) );
        handled = true;
      }
      return handled;
    default:
      kDebug() << "Unknown operation type" << msg.operation() << "in item change notification";
  }

  return false;
}

bool MonitorPrivate::emitCollectionNotification( const IdleNotification &msg )
{
  return false;
  /*
  const Collection collection = msg.entities().first();
  switch ( msg.operation() ) {
    case Idle::Add:
      if ( q_ptr->receivers( SIGNAL(collectionAdded(Akonadi::Collection,Akonadi::Collection)) ) == 0 )
        return false;
      emit q_ptr->collectionAdded( collection, parent );
      return true;
    case Idle::Modify:
      if ( q_ptr->receivers( SIGNAL(collectionChanged(Akonadi::Collection)) ) == 0
        && q_ptr->receivers( SIGNAL(collectionChanged(Akonadi::Collection,QSet<QByteArray>)) ) == 0 )
        return false;
      emit q_ptr->collectionChanged( collection );
      emit q_ptr->collectionChanged( collection, msg.itemParts() );
      return true;
    case Idle::Move:
      if ( q_ptr->receivers( SIGNAL(collectionMoved(Akonadi::Collection,Akonadi::Collection,Akonadi::Collection)) ) == 0 )
        return false;
      emit q_ptr->collectionMoved( collection, parent, destination );
      return true;
    case Idle::Remove:
      if ( q_ptr->receivers( SIGNAL(collectionRemoved(Akonadi::Collection)) ) == 0 )
        return false;
      emit q_ptr->collectionRemoved( collection );
      return true;
    case Idle::Subscribe:
      if ( q_ptr->receivers( SIGNAL(collectionSubscribed(Akonadi::Collection,Akonadi::Collection)) ) == 0 )
        return false;
      if ( !monitorAll ) // ### why??
        emit q_ptr->collectionSubscribed( collection, parent );
      return true;
    case Idle::Unsubscribe:
      if ( q_ptr->receivers( SIGNAL(collectionUnsubscribed(Akonadi::Collection)) ) == 0 )
        return false;
      if ( !monitorAll ) // ### why??
        emit q_ptr->collectionUnsubscribed( collection );
      return true;
    default:
      kDebug() << "Unknown operation type" << msg.operation() << "in collection change notification";
  }

  return false;
  */
}

void MonitorPrivate::ref( Collection::Id id )
{
  if ( !refCountMap.contains( id ) )
  {
    refCountMap.insert( id, 0 );
  }
  ++refCountMap[ id ];

  if ( m_buffer.isBuffered( id ) )
    m_buffer.purge( id );
}

Akonadi::Collection::Id MonitorPrivate::deref( Collection::Id id )
{
  Q_ASSERT( refCountMap.contains( id ) );
  if ( --refCountMap[ id ] == 0 )
  {
    refCountMap.remove( id );
  }
  return m_buffer.buffer( id );
}

void MonitorPrivate::PurgeBuffer::purge( Collection::Id id )
{
  m_buffer.removeOne(id);
}

Akonadi::Collection::Id MonitorPrivate::PurgeBuffer::buffer( Collection::Id id )
{
  // Ensure that we don't put a duplicate @p id into the buffer.
  purge( id );

  Collection::Id bumpedId = -1;
  if ( m_buffer.size() == MAXBUFFERSIZE )
  {
    bumpedId = m_buffer.dequeue();
    purge( bumpedId );
  }

  m_buffer.enqueue( id );

  return bumpedId;
}

void MonitorPrivate::notifyCollectionStatisticsWatchers( Entity::Id collection, const QByteArray &resource ) {
  if ( collection > 0 && (monitorAll || monitoredCollections.contains( Collection( collection ) ) || monitoredResources.contains( resource ) ) ) {
    recentlyChangedCollections.insert( collection );
    if ( !statisticsCompressionTimer.isActive() ) {
      statisticsCompressionTimer.start();
    }
  }
}

// @endcond
