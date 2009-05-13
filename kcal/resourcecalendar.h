/*
  This file is part of the kcal library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#ifndef KCAL_RESOURCECALENDAR_H
#define KCAL_RESOURCECALENDAR_H

#include "alarm.h"
#include "todo.h"
#include "event.h"
#include "journal.h"
#include "calendar.h"

#include "kresources/resource.h"
#include "kresources/manager.h"
#include "kabc/lock.h"

#include <kdatetime.h>
#include <kconfig.h>

#include <QtCore/QString>

namespace KCal {

/**
  This class provides the interfaces for a calendar resource. It makes use of
  the kresources framework.
*/
class KCAL_EXPORT ResourceCalendar : public KRES::Resource
{
  Q_OBJECT
  public:
    ResourceCalendar();
    explicit ResourceCalendar( const KConfigGroup &group );
    virtual ~ResourceCalendar();

    bool isResolveConflictSet() const;
    void setResolveConflict( bool b );

    virtual void writeConfig( KConfigGroup &group );

    /**
      Return rich text with info about the resource. Adds standard info and
      then calls addInfoText() to add info about concrete resources.
    */
    virtual QString infoText() const;

    /**
      Load resource data. After calling this function all data is accessible by
      calling the incidence/event/todo/etc. accessor functions.

      Whether data is actually loaded within this function or the loading is
      delayed until it is accessed by another function depends on the
      implementation of the resource.

      If loading the data takes significant time, the resource should return
      cached values if available, and return the results via the resourceChanged
      signal. When the resource has finished loading, the resourceLoaded()
      signal is emitted.

      Calling this function multiple times should have the same effect as
      calling it once, given that the data isn't changed between calls.

      This function calls doLoad() which has to be reimplented by the resource
      to do the actual loading.
    */
    virtual bool load();

    /**
      Save resource data. After calling this function it is safe to close the
      resource without losing data.

      Whether data is actually saved within this function or saving is delayed
      depends on the implementation of the resource.

      If saving the data takes significant time, the resource should return from
      the function, do the saving in the background and notify the end of the
      save by emitting the signal resourceSaved().

      This function calls doSave() which has to be reimplented by the resource
      to do the actual saving.

      @param incidence if given as 0, doSave() is called to save all incidences,
             else doSave(incidence) is called to save only the given one.
    */
    bool save( Incidence *incidence = 0 );

    /**
      Save resource data and deliver error message.

      This function calls save() and delivers the latest error msg.

      @param err reference QString to hand over the error msg to the caller.
      @param incidence if given as 0, doSave() is called to save all incidences,
             else doSave(incidence) is called to save only the given one.
    */
    bool save( QString &err, Incidence *incidence = 0 );

    /**
      Return true if a save operation is still in progress, otherwise return
      false.
    */
    virtual bool isSaving();

    /**
      Inhibit or allow saves, overriding the save policy set by setSavePolicy().
      Inhibiting saves has the same effect as making all resources read-only,
      except that the inhibit status is not stored in the resource configuration.

      @param inhibit true to inhibit saves, false to allow them
    */
    void setInhibitSave( bool inhibit );

    /**
      Return whether saves have been inhibited by setInhibitSave().
     */
    bool saveInhibited() const;

    /**
      Return object for locking the resource.
    */
    virtual KABC::Lock *lock() = 0;

    /**
      Add incidence to resource.
    */
    virtual bool addIncidence( Incidence * );

    /**
      Delete incidence from resource.
    */
    virtual bool deleteIncidence( Incidence * );

    /**
      Return incidence with given unique id. If there is no incidence with that
      uid, return 0.

      @param uid the identifier of the Incidence to look for
    */
    Incidence *incidence( const QString &uid );

    /**
      Add event to resource.
    */
    virtual bool addEvent( Event *event ) = 0;

    /**
      Delete event from this resource.
    */
    virtual bool deleteEvent( Event * ) = 0;

    /**
      Removes all Events from the calendar.
    */
    virtual void deleteAllEvents() = 0;

    /**
      Retrieves an event on the basis of the unique string ID.
    */
    virtual Event *event( const QString &uid ) = 0;

    /**
      Return unfiltered list of all events in calendar. Use with care,
      this can be a bad idea for server-based calendars.

      @param sortField field used as the sort key for the result list
      @param sortDirection direction of sorting according to @p sortField
    */
    virtual Event::List rawEvents(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Builds and then returns a list of all events that match the
      date specified. Useful for dayView, etc. etc.

      @param date date for which to get the events
      @param timeSpec the time specification of the date
      @param sortField field used as the sort key for the result list
      @param sortDirection direction of sorting according to @p sortField
    */
    virtual Event::List rawEventsForDate(
      const QDate &date,
      const KDateTime::Spec &timeSpec = KDateTime::Spec(),
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Get unfiltered events for date \a dt.
    */
    virtual Event::List rawEventsForDate( const KDateTime &dt ) = 0;

    /**
      Get unfiltered events in a range of dates. If inclusive is set to true,
      only events which are completely included in the range are returned.

      @param start date at the begin of the searching range
      @param end date at the end of the searching range
      @param timeSpec timeSpec of the searching range
      @param inclusive if @c true, only match events which are completely within
             the specified range
    */
    virtual Event::List rawEvents(
      const QDate &start, const QDate &end,
      const KDateTime::Spec &timeSpec = KDateTime::Spec(),
      bool inclusive = false ) = 0;

    /**
      Sets a particular value of the resource's configuration. The possible
      keys are resource specific.

      This method is provided to make it possible
      to set resource-type specific settings without actually linking to
      the resource's library. Its use is discouraged, but in
      some situations the only possibility to avoid unwanted compiling and
      linking dependencies. E.g. if you don't want to link to the remote
      resource, but need to create a remote resource at the URL given in
      yourURL, you can use code like the following:
        KCal::ResourceCalendar *res = manager->createResource( "remote" );
        if ( res ) {
          res->setTimeZoneId( timezone );
          res->setResourceName( i18n("Test resource") );
          res->setValue( "DownloadURL", yourURL );
          manager->add( res );
        }

      @param key the key of the resource configuration option
      @param value the value to set for the given option
    */
    virtual bool setValue( const QString &key, const QString &value );

  Q_SIGNALS:
    /**
      This signal is emitted when the data in the resource has changed. The
      resource has to make sure that this signal is emitted whenever any
      pointers to incidences which the resource has previously given to the
      calling code, become invalid.
    */
    void resourceChanged( ResourceCalendar * );

    /**
      This signal is emitted when loading data into the resource has been
      finished.
    */
    void resourceLoaded( ResourceCalendar * );

    /**
      This signal is emitted when saving the data of the resource has been
      finished.
    */
    void resourceSaved( ResourceCalendar * );

    /**
      This signal is emitted when an error occurs during loading.
    */
    void resourceLoadError( ResourceCalendar *, const QString &error );

    /**
      This signal is emitted when an error occurs during saving.
    */
    void resourceSaveError( ResourceCalendar *, const QString &error );

    /**
      This signal is emitted when a subresource is added.
    */
    void signalSubresourceAdded( ResourceCalendar *, const QString &type,
                                 const QString &subresource, const QString &label );

    /**
      This signal is emitted when a subresource is removed.
    */
    void signalSubresourceRemoved( ResourceCalendar *, const QString &,
                                   const QString & );

  public:
    /**
      Add a todo to the todolist.
    */
    virtual bool addTodo( Todo *todo ) = 0;

    /**
      Remove a todo from the todolist.
    */
    virtual bool deleteTodo( Todo * ) = 0;

    /**
      Removes all To-dos from the calendar.
    */
    virtual void deleteAllTodos() = 0;

    /**
      Searches todolist for an event with this unique id.
      @param uid the identifier of the todo to look for

      @return pointer to todo or 0 if todo wasn't found
    */
    virtual Todo *todo( const QString &uid ) = 0;

    /**
      Return list of all todos.
    */
    virtual Todo::List rawTodos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Returns list of todos due on the specified date.
    */
    virtual Todo::List rawTodosForDate( const QDate &date ) = 0;

    /**
      Add a Journal entry to the resource.
    */
    virtual bool addJournal( Journal * ) = 0;

    /**
      Remove a Journal entry from calendar.
    */
    virtual bool deleteJournal( Journal * ) = 0;

    /**
      Removes all Journals from the calendar.
    */
    virtual void deleteAllJournals() = 0;

    /**
      Return Journal with given unique id.
    */
    virtual Journal *journal( const QString &uid ) = 0;

    /**
      Return list of all journals.
    */
    virtual Journal::List rawJournals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Returns list of journals for the given date.
    */
    virtual Journal::List rawJournalsForDate( const QDate &date ) = 0;

    /**
      Return all alarms which occur in the given time interval.
    */
    virtual Alarm::List alarms( const KDateTime &from,
                                const KDateTime &to ) = 0;

    /**
      Return all alarms which occur before given date.
    */
    virtual Alarm::List alarmsTo( const KDateTime &to ) = 0;

    /** Returns a list of all incideces */
    Incidence::List rawIncidences();

    /**
      Sets the default and viewing time specification for the calendar.

      @param timeSpec is the time specification (time zone, etc.) for
                      viewing Incidence dates.\n
    */
    virtual void setTimeSpec( const KDateTime::Spec &timeSpec ) = 0;

    /**
       Get the viewing time specification (time zone etc.) for the calendar.

       @return time specification
    */
    virtual KDateTime::Spec timeSpec() const = 0;

    /**
      Sets the time zone ID for the Calendar.

      @param timeZoneId is a string containing a time zone ID, which is
      assumed to be valid. The time zone ID is used to set the time zone
      for viewing Incidence date/times. If no time zone is found, the
      viewing time specification is set to local clock time.
      @e Example: "Europe/Berlin"
    */
    virtual void setTimeZoneId( const QString &timeZoneId ) = 0;

    /**
      Returns the time zone ID used for creating or modifying incidences in
      the calendar.

      @return the string containing the time zone ID, or empty string if the
              creation/modification time specification is not a time zone.
    */
    virtual QString timeZoneId() const = 0;

    /**
      Shifts the times of all incidences so that they appear at the same clock
      time as before but in a new time zone. The shift is done from a viewing
      time zone rather than from the actual incidence time zone.

      For example, shifting an incidence whose start time is 09:00 America/New York,
      using an old viewing time zone (@p oldSpec) of Europe/London, to a new time
      zone (@p newSpec) of Europe/Paris, will result in the time being shifted
      from 14:00 (which is the London time of the incidence start) to 14:00 Paris
      time.

      @param oldSpec the time specification which provides the clock times
      @param newSpec the new time specification
    */
    virtual void shiftTimes( const KDateTime::Spec &oldSpec,
                             const KDateTime::Spec &newSpec ) = 0;

    /**
      If this resource has subresources, return a QStringList of them.
      In most cases, resources do not have subresources, so this is
      by default just empty.
    */
    virtual QStringList subresources() const;

    /**
     * Is this resource capable of having subresources or not?
     */
    virtual bool canHaveSubresources() const;

    /**
      Is this subresource active or not?
    */
    virtual bool subresourceActive( const QString &resource ) const;

    /**
      What is the label for this subresource?
     */
    virtual QString labelForSubresource( const QString &resource ) const;

    /**
      Get the identifier of the subresource associated with a specified
      incidence.

      @param incidence the incidence to get the identifier for

      @return the identifier of the subresource or an empty string.
    */
    virtual QString subresourceIdentifier( Incidence *incidence );

  public Q_SLOTS:
    /**
      (De-)activate a subresource.
    */
    virtual void setSubresourceActive( const QString &resource, bool active );

    /**
     * Remove a subresource with the id @param resource
     */
    virtual bool removeSubresource( const QString &resource );

    /**
      Add a subresource with the id @param resource and the parent id
      @param parent
    */
    virtual bool addSubresource( const QString &resource, const QString &parent );

    /**
      Returns the type of the subresource: "event", "todo", or "journal",
      QString if unknown/mixed.
    */
    virtual QString subresourceType( const QString &resource );

  protected:
    /**
      Do the actual loading of the resource data. Called by load().
    */
    virtual bool doLoad( bool syncCache ) = 0;

    /**
      Do the actual saving of the resource data. Called by save().
    */
    virtual bool doSave( bool syncCache ) = 0;

    /**
      Do the actual saving of the resource data. Called by save().
      Save one Incidence. The default implementation calls doSave()
      to save everything.
    */
    virtual bool doSave( bool syncCache, Incidence * );

    /**
      Add info text for concrete resources. Called by infoText().
    */
    virtual void addInfoText( QString & ) const {}

    /**
      A resource should call this function if a load error happens.
    */
    void loadError( const QString &errorMessage = QString() );

    /**
      A resource should call this function if a save error happens.
    */
    void saveError( const QString &errorMessage = QString() );

    bool receivedLoadError() const;
    void setReceivedLoadError( bool b );
    bool receivedSaveError() const;
    void setReceivedSaveError( bool b );

    /**
      Specify whether individual incidences should be set read-only when a
      read-only resource is loaded.
      @param noReadOnly true to inhibit setting incidences read-only,
                        false to allow incidences to be set read-only
    */
    void setNoReadOnlyOnLoad( bool noReadOnly );

    /**
      Return whether individual incidences are inhibited from being set
      read-only when a read-only resources is loaded.
    */
    bool noReadOnlyOnLoad() const;

    using QObject::event;   // prevent warning about hidden virtual method

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( ResourceCalendar )
    class Private;
    Private *const d;
    //@endcond
};

/** Type representing the manager of a ResourceCalendar. */
typedef KRES::Manager<ResourceCalendar> CalendarResourceManager;

}

#endif
