/*
  This file is part of the kcal library.

  Copyright (c) 2006 David Jarvie <djarvie@kde.org>
  Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_RESOURCECACHED_H
#define KCAL_RESOURCECACHED_H

#include "resourcecalendar.h"
#include "incidence.h"

#include <kdatetime.h>

#include <QtCore/QString>

class KConfigGroup;
namespace KRES { class IdMapper; }

namespace KCal {

class CalendarLocal;

/**
  This class provides a calendar resource using a local CalendarLocal object to
  cache the calendar data.
*/
class KCAL_EXPORT ResourceCached : public ResourceCalendar,
                                   public KCal::Calendar::CalendarObserver
{
  Q_OBJECT
  public:
    /**
      Reload policy. Whether and when to automatically reload the resource.
      @see setReloadPolicy(), reloadPolicy()
    */
    enum {
      ReloadNever,     /**< never reload the resource automatically */
      ReloadOnStartup, /**< reload when the resource is opened */
      ReloadInterval   /**< reload at regular intervals set by setReloadInterval() */
    };

    /**
      Save policy. Whether and when to automatically save the resource.
      @see setSavePolicy(), savePolicy()
    */
    enum {
      SaveNever,    /**< never save the resource automatically */
      SaveOnExit,   /**< save when the resource is closed */
      SaveInterval, /**< save at regular intervals set by setSaveInterval() */
      SaveDelayed,  /**< save after every change, after a 15 second delay */
      SaveAlways    /**< save after every change, after a 1 second delay */
    };

    /**
      Whether to update the cache file when loading a resource, or whether to
      upload the cache file after saving the resource.
      Only applicable to genuinely cached resources.
     */
    enum CacheAction {
      DefaultCache,/**< use the default action set by setReloadPolicy() or setSavePolicy() */
      NoSyncCache, /**< perform a cache-only operation, without downloading or uploading */
      SyncCache    /**< update the cache file before loading, or upload cache after saving */
    };

    ResourceCached();
    explicit ResourceCached( const KConfigGroup &group );
    virtual ~ResourceCached();

    void readConfig( const KConfigGroup &group );
    void writeConfig( KConfigGroup &group );

    /**
      Set reload policy. This controls when the cache is refreshed.

      ReloadNever     never reload
      ReloadOnStartup reload when resource is started
      ReloadInterval  reload regularly after given interval
    */
    void setReloadPolicy( int policy );
    /**
      Return reload policy.

      @see setReloadPolicy()
    */
    int reloadPolicy() const;

    /**
      Set reload interval in minutes which is used when reload policy is
      ReloadInterval.
    */
    void setReloadInterval( int minutes );

    /**
      Return reload interval in minutes.
    */
    int reloadInterval() const;

    /**
      Inhibit or allow cache reloads when using load(DefaultCache). If inhibited,
      this overrides the policy set by setReloadPolicy(), preventing any non-explicit
      reloads from being performed. If not inhibited, reloads take place according
      to the policy set by setReloadPolicy().

      @param inhibit  true to inhibit reloads, false to allow them
    */
    bool inhibitDefaultReload( bool inhibit );
    bool defaultReloadInhibited() const;

    /**
      Return whether the resource cache has been reloaded since startup.
     */
    bool reloaded() const;

    /**
      Set save policy. This controls when the cache is refreshed.

      SaveNever     never save
      SaveOnExit    save when resource is exited
      SaveInterval  save regularly after given interval
      SaveDelayed   save on every change, after small delay
      SaveAlways    save on every change
    */
    void setSavePolicy( int policy );
    /**
      Return save policy.

      @see setsavePolicy()
    */
    int savePolicy() const;

    /**
      Set save interval in minutes which is used when save policy is
      SaveInterval.
    */
    void setSaveInterval( int minutes );

    /**
      Return save interval in minutes.
    */
    int saveInterval() const;

    /**
      Return time of last load.
    */
    KDateTime lastLoad() const;

    /**
      Return time of last save.
    */
    KDateTime lastSave() const;

    /**
      Load resource data, specifying whether to refresh the cache file first.
      For a non-cached resource, this method has the same effect as load().

      @param action is the type of #CacheAction for this data loading.
     */
    bool load( CacheAction action );

    /**
      Load resource data.
     */
    virtual bool load();

    /**
      Save the resource data to cache, and optionally upload the cache file
      afterwards. For a non-cached resource, this method has the same effect
      as save().

      @param action is the type of #CacheAction for this data saving.
      @param incidence if given as 0, doSave(bool) is called to save all
      incidences, else doSave(bool, incidence) is called to save only the
      given one.
    */
    bool save( CacheAction action, Incidence *incidence = 0 );

    /**
      Save resource data.
     */
    virtual bool save( Incidence *incidence = 0 );

    /**
      Add event to calendar.
    */
    bool addEvent( Event *event );

    /**
      Deletes an event from this calendar.
    */
    bool deleteEvent( Event *event );

    /**
      Removes all Events from this calendar.
    */
    void deleteAllEvents();

    /**
      Retrieves an event on the basis of the unique string ID.
    */
    Event *event( const QString &UniqueStr );

    /**
      Return filtered list of all events in calendar.
    */
    Event::List events();

    /**
      Return unfiltered list of all events in calendar.
    */
    Event::List rawEvents( EventSortField sortField = EventSortUnsorted,
                           SortDirection sortDirection = SortDirectionAscending );

    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.

      @param date date for which to get the events
      @param timeSpec the time specification of the date
      @param sortField field used as the sort key for the result list
      @param sortDirection direction of sorting according to @p sortField
    */
    Event::List rawEventsForDate( const QDate &date,
                                  const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                                  EventSortField sortField = EventSortUnsorted,
                                  SortDirection sortDirection = SortDirectionAscending );

    /**
      Get unfiltered events for date \a dt.
    */
    Event::List rawEventsForDate( const KDateTime &dt );

    /**
      Get unfiltered events in a range of dates. If inclusive is set to true,
      only events are returned, which are completely included in the range.

      @param start date at the begin of the searching range
      @param end date at the end of the searching range
      @param timeSpec timeSpec of the searching range
      @param inclusive if @c true, only match events which are completely within
             the specified range
    */
    Event::List rawEvents( const QDate &start, const QDate &end,
                           const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                           bool inclusive = false );

    /**
      Add a todo to the todolist.
    */
    bool addTodo( Todo *todo );
    /**
      Remove a todo from the todolist.
    */
    bool deleteTodo( Todo * );

    /**
      Removes all todos from this calendar.
    */
    void deleteAllTodos();

    /**
      Searches todolist for an event with this unique string identifier,
      returns a pointer or null.
    */
    Todo *todo( const QString &uid );

    /**
      Return list of all todos.
    */
    Todo::List rawTodos( TodoSortField sortField = TodoSortUnsorted,
                         SortDirection sortDirection = SortDirectionAscending );

    /**
      Returns list of todos due on the specified date.
    */
    Todo::List rawTodosForDate( const QDate &date );

    /**
      Add a Journal entry to calendar
    */
    virtual bool addJournal( Journal * );

    /**
      Remove a Journal from the calendar
    */
    virtual bool deleteJournal( Journal * );

    /**
      Removes all Journals from this calendar.
    */
    virtual void deleteAllJournals();

    /**
      Return Journal with given unique id.
    */
    virtual Journal *journal( const QString &uid );

    /**
      Return list of all journals.
    */
    Journal::List rawJournals( JournalSortField sortField = JournalSortUnsorted,
                               SortDirection sortDirection = SortDirectionAscending );

    /**
      Return list of journals for the given date.
    */
    Journal::List rawJournalsForDate( const QDate &date );

    /**
      Return all alarms, which occur in the given time interval.
    */
    Alarm::List alarms( const KDateTime &from, const KDateTime &to );

    /**
      Return all alarms, which occur before given date.
    */
    Alarm::List alarmsTo( const KDateTime &to );

    /**
      Set the time specification (time zone, etc.).

      @param timeSpec the time specification to set
      @see timeSpec()
    */
    void setTimeSpec( const KDateTime::Spec &timeSpec );

    /**
       Get the viewing time specification (time zone etc.) for the calendar.

       @return time specification
    */
    KDateTime::Spec timeSpec() const;

    /**
      Set id of timezone, e.g. "Europe/Berlin"

      @param timeZoneId the identifier for the timezone
      @see timeZoneId()
    */
    void setTimeZoneId( const QString &timeZoneId );

    /**
      Returns the viewing time zone ID for the resource.

      @return the string containing the time zone ID, or empty string if the
              viewing time specification is not a time zone.
    */
    QString timeZoneId() const;

    /**
      @copydoc
      ResourceCalendar::shiftTimes()
    */
    virtual void shiftTimes( const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec );

    /**
      Return the owner of the calendar's full name.
    */
    Person owner() const;

    /**
      Set the owner of the calendar. Should be owner's full name.
      @param owner the person who owns this calendar resource
    */
    void setOwner( const Person &owner );

    void enableChangeNotification();
    void disableChangeNotification();

    void clearChange( Incidence *incidence );
    void clearChange( const QString &uid );

    void clearChanges();

    bool hasChanges() const;

    Incidence::List allChanges() const;

    Incidence::List addedIncidences() const;
    Incidence::List changedIncidences() const;
    Incidence::List deletedIncidences() const;

    /**
      Load the resource from the cache.
      @return true if the cache file exists, false if not
     */
    bool loadFromCache();

    /**
      Save the resource back to the cache.
     */
    void saveToCache();

    /**
      Clear cache.
    */
    void clearCache();

    void cleanUpEventCache( const KCal::Event::List &eventList );
    void cleanUpTodoCache( const KCal::Todo::List &todoList );

    /**
      Returns a reference to the id mapper.
     */
    KRES::IdMapper &idMapper();

    using QObject::event;   // prevent warning about hidden virtual method

  protected:
    CalendarLocal *calendar() const;

    // From Calendar::CalendarObserver
    void calendarIncidenceAdded( KCal::Incidence *incidence );
    void calendarIncidenceChanged( KCal::Incidence *incidence );
    void calendarIncidenceDeleted( KCal::Incidence *incidence );

    /**
      Virtual method from KRES::Resource, called when the last instace of the
      resource is closed
     */
    virtual void doClose();

    /**
      Opens the resource. Dummy implementation, so child classes don't have to
      reimplement this method. By default, this does not do anything, but can be
      reimplemented in child classes
     */
    virtual bool doOpen();

    /**
      Do the actual loading of the resource data. Called by load(CacheAction).
    */
    virtual bool doLoad( bool syncCache ) = 0;

    /**
      Set the cache-reloaded status. Non-local resources must set this true once
      the cache has been downloaded successfully.

      @param done the new cache-reloaded status
     */
    void setReloaded( bool done );

    /**
      Do the actual saving of the resource data. Called by save(CacheAction).
      Saves the resource data to the cache and optionally uploads (if a remote
      resource).

      @param syncCache if true, the cache will be uploaded to the remote
      resource. If false, only the cache will be updated.
    */
    virtual bool doSave( bool syncCache ) = 0;

    /**
      Do the actual saving of the resource data. Called by save(CacheAction).
      Save one Incidence. The default implementation calls doSave(bool) to
      save everything.

      @param syncCache if @c true, the cache will be uploaded to the remote
             resource. If @c false, only the cache will be updated
      @param incidence The incidence to be saved.
    */
    virtual bool doSave( bool syncCache, Incidence *incidence );

    /**
      Check if reload required according to reload policy.
    */
    bool checkForReload();

    /**
      Check if save required according to save policy.
    */
    bool checkForSave();

    void checkForAutomaticSave();

    void addInfoText( QString & ) const;

    void setupSaveTimer();
    void setupReloadTimer();

    /**
      This method is used by loadFromCache() and saveToCache(), reimplement
      it to change the location of the cache.
     */
    virtual QString cacheFile() const;

    /**
      Functions for keeping the changes persistent.
     */
    virtual QString changesCacheFile( const QString &type ) const;
    void loadChangesCache( QMap<Incidence *, bool> &map, const QString &type );
    void loadChangesCache();
    void saveChangesCache( const QMap<Incidence *, bool> &map, const QString &type );
    void saveChangesCache();

  protected Q_SLOTS:
    void slotReload();
    void slotSave();

    void setIdMapperIdentifier();

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( ResourceCached )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
