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

#ifndef AKONADI_ITEMSYNC_H
#define AKONADI_ITEMSYNC_H

#include "akonadi_export.h"

#include <akonadi/item.h>
#include <akonadi/job.h>

namespace Akonadi {

class Collection;

/**
 * @short Syncs between items known to a client (usually a resource) and the Akonadi storage.
 *
 * Remote Id must only be set by the resource storing the item, other clients
 * should leave it empty, since the resource responsible for the target collection
 * will be notified about the addition and then create a suitable remote Id.
 *
 * There are two different forms of ItemSync usage:
 * - Full-Sync: meaning the client provides all valid items, i.e. any item not
 *   part of the list but currently stored in Akonadi will be removed
 * - Incremental-Sync: meaning the client provides two lists, one for items which
 *   are new or modified and one for items which should be removed. Any item not
 *   part of either list but currently stored in Akonadi will not be changed.
 *
 * @note This is provided for convenience to implement "save all" like behavior,
 *       however it is strongly recommended to use single item jobs whenever
 *       possible, e.g. ItemCreateJob, ItemModifyJob and ItemDeleteJob
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class AKONADI_EXPORT ItemSync : public Job
{
  Q_OBJECT

  public:
    /**
     * Creates a new item synchronizer.
     *
     * @param collection The collection we are syncing.
     * @param parent The parent object.
     */
    explicit ItemSync( const Collection &collection, QObject *parent = 0 );

    /**
     * Destroys the item synchronizer.
     */
    ~ItemSync();

    /**
     * Sets the full item list for the collection.
     *
     * Usually the result of a full item listing.
     *
     * @warning If the client using this is a resource, all items must have
     *          a valid remote identifier.
     *
     * @param items A list of items.
     */
    void setFullSyncItems( const Item::List &items );

    /**
     * Set the amount of items which you are going to return in total
     * by using the setFullSyncItems() method.
     *
     * @param amount The amount of items in total.
     */
    void setTotalItems( int amount );

    /**
      Enable item streaming. Item streaming means that the items delivered by setXItems() calls
      are delivered in chunks and you manually indicate when all items have been delivered
      by calling deliveryDone().
      @param enable @c true to enable item streaming
    */
    void setStreamingEnabled( bool enable );

    /**
      Notify ItemSync that all remote items have been delivered.
      Only call this in streaming mode.
    */
    void deliveryDone();

    /**
     * Sets the item lists for incrementally syncing the collection.
     *
     * Usually the result of an incremental remote item listing.
     *
     * @warning If the client using this is a resource, all items must have
     *          a valid remote identifier.
     *
     * @param changedItems A list of items added or changed by the client.
     * @param removedItems A list of items deleted by the client.
     */
    void setIncrementalSyncItems( const Item::List &changedItems,
                                  const Item::List &removedItems );

  protected:
    void doStart();

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotLocalListDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotLocalChangeDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotTransactionResult( KJob* ) )
    //@endcond
};

}

#endif
