/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef AKONADI_FILTERACTIONJOB_H
#define AKONADI_FILTERACTIONJOB_H

#include <mailtransport/mailtransport_export.h>

#include <akonadi/item.h>
#include <akonadi/transactionsequence.h>

namespace Akonadi {

class Collection;
class ItemFetchScope;
class Job;

class FilterActionJob;

/**
 * @short Base class for a filter/action for FilterActionJob.
 *
 * Abstract class defining an interface for a filter and an action for
 * FilterActionJob.  The virtual methods must be implemented in subclasses.
 *
 * @code
 * class ClearErrorAction : public Akonadi::FilterAction
 * {
 *   public:
 *     // reimpl
 *     virtual Akonadi::ItemFetchScope fetchScope() const
 *     {
 *       ItemFetchScope scope;
 *       scope.fetchFullPayload( false );
 *       scope.fetchAttribute<ErrorAttribute>();
 *       return scope;
 *     }
 *
 *     virtual bool itemAccepted( const Akonadi::Item &item ) const
 *     {
 *       return item.hasAttribute<ErrorAttribute>();
 *     }
 *
 *     virtual Akonadi::Job *itemAction( const Akonadi::Item &item, Akonadi::FilterActionJob *parent ) const
 *     {
 *       Item cp = item;
 *       cp.removeAttribute<ErrorAttribute>();
 *       return new ItemModifyJob( cp, parent );
 *     }
 * };
 * @endcode
 *
 * @see FilterActionJob
 *
 * @author Constantin Berzan <exit3219@gmail.com>
 * @since 4.4
 */
class MAILTRANSPORT_EXPORT FilterAction
{
  public:
    /**
     * Destroys this filter action.
     *
     * A FilterActionJob will delete its FilterAction automatically.
     */
    virtual ~FilterAction();

    /**
     * Returns an ItemFetchScope to use if the FilterActionJob needs
     * to fetch the items from a collection.
     *
     * @note The items are not fetched unless FilterActionJob is
     *       constructed with a Collection parameter.
     */
    virtual Akonadi::ItemFetchScope fetchScope() const = 0;

    /**
     * Returns @c true if the @p item is accepted by the filter and should be
     * acted upon by the FilterActionJob.
     */
    virtual bool itemAccepted( const Akonadi::Item &item ) const = 0;

    /**
     * Returns a job to act on the @p item.
     * The FilterActionJob will finish when all such jobs are finished.
     */
    virtual Akonadi::Job *itemAction( const Akonadi::Item &item, Akonadi::FilterActionJob *parent ) const = 0;
};

/**
 * @short Job to filter and apply an action on a set of items.
 *
 * This jobs filters through a set of items, and applies an action to the
 * items which are accepted by the filter.  The filter and action
 * are provided by a functor class derived from FilterAction.
 *
 * For example, a MarkAsRead action/filter may be used to mark all messages
 * in a folder as read.
 *
 * @code
 * FilterActionJob *mjob = new FilterActionJob( LocalFolders::self()->outbox(),
 *                                              new ClearErrorAction, this );
 * connect( mjob, SIGNAL( result( KJob* ) ), this, SLOT( massModifyResult( KJob* ) ) );
 * @endcode
 *
 * @see FilterAction
 *
 * @author Constantin Berzan <exit3219@gmail.com>
 * @since 4.4
 */
class MAILTRANSPORT_EXPORT FilterActionJob : public TransactionSequence
{
  Q_OBJECT

  public:
    /**
     * Creates a filter action job to act on a single item.
     *
     * @param item The item to act on. The item is not re-fetched.
     * @param functor The FilterAction to use.
     * @param parent The parent object.
     */
    FilterActionJob( const Item &item, FilterAction *functor, QObject *parent = 0 );

    /**
     * Creates a filter action job to act on a set of items.
     *
     * @param items The items to act on. The items are not re-fetched.
     * @param functor The FilterAction to use.
     * @param parent The parent object.
     */
    FilterActionJob( const Item::List &items, FilterAction *functor, QObject *parent = 0 );

    /**
     * Creates a filter action job to act on items in a collection.
     *
     * @param collection The collection to act on.
     *                   The items of the collection are fetched using functor->fetchScope().
     * @param functor The FilterAction to use.
     * @param parent The parent object.
     */
    FilterActionJob( const Collection &collection, FilterAction *functor, QObject *parent = 0 );

    /**
     * Destroys the filter action job.
     */
    ~FilterActionJob();

  protected:
    /* reimpl */
    virtual void doStart();

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void fetchResult( KJob* ) )
    //@endcond
};

} // namespace Akonadi

#endif // AKONADI_FILTERACTIONJOB_H
