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

#ifndef AKONADI_MONITOR_P_H
#define AKONADI_MONITOR_P_H

#include "akonadiprivate_export.h"
#include "monitor.h"
#include "collection.h"
#include "collectionstatisticsjob.h"
#include "collectionfetchscope.h"
#include "item.h"
#include "itemfetchscope.h"
#include "job.h"
#include "servermanager.h"
#include "session.h"

#include <akonadi/private/idle_p.h>

#include <kmimetype.h>

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QQueue>

namespace Akonadi {

class IdleJob;

class Monitor;

/**
 * @internal
 */
class AKONADI_TESTS_EXPORT MonitorPrivate{
  public:
    MonitorPrivate( Monitor *parent );
    virtual ~MonitorPrivate();

    void init();

    Monitor *q_ptr;
    Q_DECLARE_PUBLIC( Monitor )

    IdleJob *idleJob;
    bool monitorAll;
    Collection::List monitoredCollections;
    QSet<Item::Id> monitoredItems;
    QSet<QByteArray> monitoredResources;
    QSet<QString> monitoredMimetypes;
    QMap<Session*, QByteArray> ignoredSessions;

    QQueue<IdleNotification> pendingNotifications;

    ItemFetchScope mItemFetchScope;
    CollectionFetchScope mCollectionFetchScope;
    bool mFetchChangedOnly;
    Session *session;

    bool fetchCollection;
    bool fetchCollectionStatistics;
    bool collectionMoveTranslationEnabled;

    // Virtual methods for ChangeRecorder
    virtual void notificationsEnqueued( int ) {}
    virtual void notificationsErased() {}


    virtual void slotNotify( const Akonadi::IdleNotification &notification );

    /**
     * Sends out the change notification @p msg.
     * @param msg the change notification to send
     * @return @c true if the notification was actually send to someone, @c false if no one was listening.
     */
    //virtual bool emitNotification( const NotificationMessageV2 &msg );
    void updatePendingStatistics( const IdleNotification &msg );
    void invalidateCaches( const IdleNotification &msg );

    /** Used by ResourceBase to inform us about collection changes before the notifications are emitted,
        needed to avoid the missing RID race on change replay.
    */
    //void invalidateCache( const Collection &col );


    // private slots
    //void dataAvailable();
    void slotStatisticsChangedFinished( KJob* );
    void slotFlushRecentlyChangedCollections();
    void slotSessionDestroyed( QObject *session );

    /**
     * Sends out a change notification for an item.
     * @return @c true if the notification was actually send to someone, @c false if no one was listening.
     */
    bool emitItemsNotification( const IdleNotification& msg );
    /**
     * Sends out a change notification for a collection.
     * @return @c true if the notification was actually send to someone, @c false if no one was listening.
     */
    bool emitCollectionNotification( const IdleNotification &msg );

    /**
      @brief Class used to determine when to purge items in a Collection

      The buffer method can be used to buffer a Collection. This may cause another Collection
      to be purged if it is removed from the buffer.

      The purge method is used to purge a Collection from the buffer, but not the model.
      This is used for example, to not buffer Collections anymore if they get referenced,
      and to ensure that one Collection does not appear twice in the buffer.

      Check whether a Collection is buffered using the isBuffered method.

      TODO: Maybe move the PurgeBuffer out of Monitor?
    */
    class PurgeBuffer
    {
      // Buffer the most recent 10 unreferenced Collections
      static const int MAXBUFFERSIZE = 10;
    public:
      explicit PurgeBuffer()
      {
      }

      /**
        Adds @p id to the Collections to be buffered

        @returns The collection id which was removed form the buffer or -1 if none.
      */
      Collection::Id buffer( Collection::Id id );

      /**
      Removes @p id from the Collections being buffered
      */
      void purge( Collection::Id id );

      bool isBuffered( Collection::Id id ) const
      {
        return m_buffer.contains( id );
      }

    private:
      QQueue<Collection::Id> m_buffer;
    } m_buffer;

    QHash<Collection::Id, int> refCountMap;
    bool useRefCounting;
    void ref( Collection::Id id );
    Collection::Id deref( Collection::Id id );

  private:
    // collections that need a statistics update
    QSet<Collection::Id> recentlyChangedCollections;
    QTimer statisticsCompressionTimer;

    /**
      @returns True if @p msg should be ignored. Otherwise appropriate signals are emitted for it.
    */
    /*
    bool isLazilyIgnored( Idle::Type type, Idle::Operation operation,
                          const Collection &parentCollection,
                          const Collection &parentDestCollection,
                          bool allowModifyFlagsConversion = false ) const;
    */

    /**
      Sets @p needsSplit to True when @p msg contains more than one item and there's at least one
      listener that does not support batch operations. Sets @p batchSupported to True when
      there's at least one listener that supports batch operations.
    */
    void checkBatchSupport( const IdleNotification &msg, bool &needsSplit, bool &batchSupported ) const;

    QList<IdleNotification> splitNotification( const IdleNotification &msg, bool legacy ) const;

    void fetchStatistics( Collection::Id colId )
    {
      CollectionStatisticsJob *job = new CollectionStatisticsJob( Collection( colId ), session );
      QObject::connect( job, SIGNAL( result( KJob* ) ), q_ptr, SLOT( slotStatisticsChangedFinished( KJob* ) ) );
    }

    void notifyCollectionStatisticsWatchers( Collection::Id collection, const QByteArray &resource );
};

}

#endif
