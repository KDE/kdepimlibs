/*
  This file is part of the kcal library.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CalendarResources class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#ifndef KCAL_CALENDARRESOURCES_H
#define KCAL_CALENDARRESOURCES_H

#include <QtCore/QMap>

#include "calendar.h"
#include "exceptions.h"
#include "resourcecalendar.h"

#include "kcal_export.h"

class QWidget;

namespace KCal {

/**
   @brief
   This class provides a Calendar which is composed of other Calendars
   known as "Resources".

   Examples of Calendar Resources are:
     - Calendars stored as local ICS formatted files
     - a set of incidences (one-per-file) within a local directory
     - birthdays and anniversaries contained in an addressbook

*/
class KCAL_EXPORT CalendarResources
  : public Calendar, public KRES::ManagerObserver<ResourceCalendar>
{
  Q_OBJECT
  public:
    /**
      @class DestinationPolicy
    */
    class DestinationPolicy
    {
      public:
        /**
          Constructs a destination policy.
          @param manager is a pointer to the CalendarResourceManager.
          @param parent is a pointer to a QWidget to use for new dialogs.
        */
        explicit DestinationPolicy( CalendarResourceManager *manager, QWidget *parent = 0 );

        /**
          Destructor.
        */
        virtual ~DestinationPolicy();

        /**
          Returns parent widget to use for new dialogs.
        */
        virtual QWidget *parent();

        /**
          Sets the parent widget for new dialogs.
          @param parent is a pointer to a QWidget containing the parent.
        */
        virtual void setParent( QWidget *parent );

        /**
          Returns the destination ResourceCalendar for the specified incidence.
          @param incidence is a pointer to a valid Incidence object.
        */
        virtual ResourceCalendar *destination( Incidence *incidence ) = 0;

        /**
	   Return true if we have resources configure. Otherwise returns false.
           @since 4.3
        */
        bool hasCalendarResources();
      protected:
        /**
          Returns the CalendarResourceManager used by this calendar.
        */
        CalendarResourceManager *resourceManager();

      private:
        //@cond PRIVATE
        Q_DISABLE_COPY( DestinationPolicy )
        class Private;
        Private *d;
        //@endcond
    };

    /**
      @class StandardDestinationPolicy
    */
    class StandardDestinationPolicy : public DestinationPolicy
    {
      public:
        /**
          Constructs a standard destination policy.
          @param manager is a pointer to the CalendarResourceManager.
          @param parent is a pointer to a QWidget to use for new dialogs.
        */
        explicit StandardDestinationPolicy( CalendarResourceManager *manager, QWidget *parent = 0 );

        /**
          Destructor.
        */
        virtual ~StandardDestinationPolicy();

        /**
          Returns the destination ResourceCalendar for the specified incidence.
          @param incidence is a pointer to a valid Incidence object.
        */
        ResourceCalendar *destination( Incidence *incidence );

      private:
        //@cond PRIVATE
        Q_DISABLE_COPY( StandardDestinationPolicy )
        class Private;
        Private *d;
        //@endcond
    };

    /**
      @class AskDestinationPolicy
    */
    class AskDestinationPolicy : public DestinationPolicy
    {
      public:
        /**
          Constructs an Ask destination policy.
          @param manager is a pointer to the CalendarResourceManager.
          @param parent is a pointer to a QWidget to use for new dialogs.
        */
        explicit AskDestinationPolicy( CalendarResourceManager *manager, QWidget *parent = 0 );

        /**
          Destructor.
        */
        virtual ~AskDestinationPolicy();

        /**
          Returns the destination ResourceCalendar for the specified incidence.
          @param incidence is a pointer to a valid Incidence object.
        */
        ResourceCalendar *destination( Incidence *incidence );
      private:
        //@cond PRIVATE
        Q_DISABLE_COPY( AskDestinationPolicy )
        class Private;
        Private *d;
        //@endcond
    };

    /**
      @class Ticket
    */
    class Ticket
    {
      friend class CalendarResources;

      public:
        /**
          Returns the ResourceCalendar associated with the ticket.
        */
        ResourceCalendar *resource() const;

        /**
          Destructor.
        */
        ~Ticket();

      private:
        /**
          Constructs a Ticket for a ResourceCalendar.
          @param resource is a pointer to a valid ResourceCalendar object.
        */
        Ticket( ResourceCalendar *resource );

        //@cond PRIVATE
        Q_DISABLE_COPY( Ticket )
        class Private;
        Private *d;
        //@endcond
    };

    /**
      Construct CalendarResource object using a time specification (time
      zone, etc.) and a Family name.

      @param timeSpec is a time specification which is used for creating
      or modifying incidences in the Calendar. It is also used for viewing
      incidences (see setViewTimeSpec()).

      @param family is any QString representing a unique name.
    */
    CalendarResources(
      const KDateTime::Spec &timeSpec,
      const QString &family = QLatin1String( "calendar" ) );

    /**
      Construct CalendarResource object using a time zone ID and a Family name.

      @param timeZoneId is used for creating or modifying incidences in the
      Calendar. It is also used for viewing incidences. The time zone does
      not alter existing incidences.

      @param family is any QString representing a unique name.
    */
    CalendarResources(
      const QString &timeZoneId,
      const QString &family = QLatin1String( "calendar" ) );

    /**
      Destroys the Calendar Resources.
    */
    ~CalendarResources();

    /**
      Clears the exception status.
      @since 4.2
    */
    void clearException();

    /**
      Returns an exception, if there is any, containing information about the
      last error that occurred.
      @since 4.2
    */
    ErrorFormat *exception();

    /**
      Loads all Incidences from the Resources.  The Resources must be added
      first using either readConfig(KConfig *config), which adds the system
      Resources, or manually using resourceAdded(ResourceCalendar *resource).
    */
    void load();

    /**
     * Reloads all Incidences from all Resources.
     * @return true if the reload was successful; otherwise failure.
     */
    bool reload();

    /**
      @copydoc
      Calendar::close()
    */
    void close();

    /**
      Saves this Calendar.
      If the save is successful the Ticket is deleted.  Otherwise, the
      caller must release the Ticket with releaseSaveTicket() to abandon
      the save operation or call save() to try the save again.

      @param ticket is a pointer to the Ticket object.
      @param incidence is a pointer to the Incidence object.
      If incidence is null, save the entire Calendar (all Resources)
      else only the specified Incidence is saved.

      @return true if the save was successful; false otherwise.
    */
    virtual bool save( Ticket *ticket, Incidence *incidence = 0 );

    /**
      @copydoc
      Calendar::save()
    */
    bool save();

    /**
      @copydoc
      Calendar::isSaving()
    */
    bool isSaving();

    /**
      Returns the CalendarResourceManager used by this calendar.
    */
    CalendarResourceManager *resourceManager() const;

    /**
      Returns the Resource associated with a specified Incidence.

      @param incidence is a pointer to an Incidence whose Resource
      is to be located.
    */
    ResourceCalendar *resource( Incidence *incidence );

    /**
      Reads the Resources settings from a config file.

      @param config The KConfig object which points to the config file.
      If no object is given (@p config is 0) the standard config file is used.

      @note Call this method <em>before</em> load().
    */
    void readConfig( KConfig *config = 0 );

    /**
      Set the destination policy such that Incidences are always added
      to the standard Resource.
    */
    void setStandardDestinationPolicy();

    /**
      Set the destination policy such that Incidences are added to a
      Resource which is queried.
    */
    void setAskDestinationPolicy();

    /**
      Return true if we have resources configure. Otherwise returns false.
      @since 4.3
    */
    bool hasCalendarResources();

    /**
      Returns the current parent for new dialogs. This is a bad hack, but we
      need to properly set the parent for the resource selection dialog.
      Otherwise the dialog will not be modal to the editor dialog in
      korganizer and the user can still work in the editor dialog (and thus
      crash korganizer).
      Afterwards we need to reset it (to avoid pointers to widgets that are
      already deleted) so we also need the accessor

      @return a pointer to the parent QWidget.
      @see setDialogParentWidget()
    */
    QWidget *dialogParentWidget();

    /**
      Set the widget parent for new dialogs. This is a bad hack, but we need
      to properly set the parent for the resource selection dialog. Otherwise
      the dialog will not be modal to the editor dialog in korganizer and
      the user can still work in the editor dialog (and thus crash korganizer).

      @param parent is a pointer to the parent QWidget.
      @see dialogparentWidget()
    */
    void setDialogParentWidget( QWidget *parent );

    /**
      Requests a ticket for saving the Calendar.  If a ticket is returned the
      Calendar is locked for write access until save() or releaseSaveTicket()
      is called.

      @param resource is a pointer to the ResourceCalendar object.

      @return a pointer to a Ticket object indicating that the Calendar
      is locked for write access; otherwise a null pointer.
      @see releaseSaveTicket()
    */
    Ticket *requestSaveTicket( ResourceCalendar *resource );

    /**
      Releases the save Ticket. The Calendar is unlocked without saving.

      @param ticket is a pointer to a Ticket object.
      @see requestSaveTicket()
    */
    virtual void releaseSaveTicket( Ticket *ticket );

    /**
      Add an active Resource to the Calendar, and loads that resource
      if it is open.  Additionally, emits the @b signalResourceAdded signal.

      @note This method must be public, because in-process added Resources
      do not emit the corresponding signal, so this method has to be
      called manually!

      @param resource is a pointer to the ResourceCalendar to add.
      @see signalResourceAdded()
    */
    void resourceAdded( ResourceCalendar *resource );

  // Incidence Specific Methods //

    /**
      Inserts an Incidence into the calendar.
      @param incidence is a pointer to the Incidence to insert.
      @return true if the Incidence was successfully inserted; false otherwise.
      @return Will also return false if there are multiple writable resources
      and the user declines to select one to those resources in which to save
      the Incidence.
    */
    bool addIncidence( Incidence *incidence );

    /**
      Inserts an Incidence into a Calendar Resource.

      @param incidence is a pointer to the Incidence to insert.
      @param resource is a pointer to the ResourceCalendar to be added to.

      @return true if the Incidence was successfully inserted; false otherwise.
    */
    bool addIncidence( Incidence *incidence, ResourceCalendar *resource );

    /**
      @copydoc
      Calendar::beginChange()
    */
    bool beginChange( Incidence *incidence );

    /**
      @copydoc
      Calendar::endChange()
    */
    bool endChange( Incidence *incidence );

  // Event Specific Methods //

    /**
      @copydoc
      Calendar::addEvent()
    */
    bool addEvent( Event *event );

    /**
      Inserts an Event into a Calendar Resource.

      @param event is a pointer to the Event to insert.
      @param resource is a pointer to the ResourceCalendar to be added to.

      @return true if the Event was successfully inserted; false otherwise.

      @note In most cases use
      addIncidence( Incidence *incidence, ResourceCalendar *resource ) instead.
    */
    bool addEvent( Event *event, ResourceCalendar *resource );

    /**
      @copydoc
      Calendar::deleteEvent()
    */
    bool deleteEvent( Event *event );

    /**
      @copydoc
      Calendar::deleteAllEvents()
    */
    void deleteAllEvents();

    /**
      @copydoc
      Calendar::rawEvents(EventSortField, SortDirection)
    */
    Event::List rawEvents(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      @copydoc
      Calendar::rawEventsForDate( const KDateTime &)
    */
    Event::List rawEventsForDate( const KDateTime &dt );

    /**
      @copydoc
      Calendar::rawEvents(const QDate &, const QDate &, const KDateTime::Spec &, bool)
    */
    Event::List rawEvents( const QDate &start, const QDate &end,
                           const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                           bool inclusive = false );

    /**
      Returns an unfiltered list of all Events which occur on the given date.

      @param date request unfiltered Event list for this QDate only.
      @param timeSpec time zone etc. to interpret @p date, or the calendar's
                      default time spec if none is specified
      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of unfiltered Events occurring on the specified QDate.
    */
    Event::List rawEventsForDate( const QDate &date,
                                  const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                                  EventSortField sortField = EventSortUnsorted,
                                  SortDirection sortDirection = SortDirectionAscending );

    /**
      @copydoc
      Calendar::event()
    */
    Event *event( const QString &uid );

  // Todo Specific Methods //

    /**
      @copydoc
      Calendar::addTodo()
    */
    bool addTodo( Todo *todo );

    /**
      Inserts a Todo into a Calendar Resource.

      @param todo is a pointer to the Todo to insert.
      @param resource is a pointer to the ResourceCalendar to be added to.

      @return true if the Todo was successfully inserted; false otherwise.

      @note In most cases use
      addIncidence( Incidence *incidence, ResourceCalendar *resource ) instead.
    */
    bool addTodo( Todo *todo, ResourceCalendar *resource );

    /**
      @copydoc
      Calendar::deleteTodo()
    */
    bool deleteTodo( Todo *todo );

    /**
      @copydoc
      Calendar::deleteAllTodos()
    */
    void deleteAllTodos();

    /**
      @copydoc
      Calendar::rawTodos()
    */
    Todo::List rawTodos( TodoSortField sortField = TodoSortUnsorted,
                         SortDirection sortDirection = SortDirectionAscending );

    /**
      @copydoc
      Calendar::rawTodosForDate()
    */
    Todo::List rawTodosForDate( const QDate &date );

    /**
      @copydoc
      Calendar::todo()
    */
    Todo *todo( const QString &uid );

  // Journal Specific Methods //

    /**
      @copydoc
      Calendar::addJournal()
    */
    bool addJournal( Journal *journal );

    /**
      Inserts a Journal into a Calendar Resource.

      @param journal is a pointer to the Journal to insert.
      @param resource is a pointer to the ResourceCalendar to be added to.

      @return true if the Journal was successfully inserted; false otherwise.

      @note In most cases use
      addIncidence( Incidence *incidence, ResourceCalendar *resource ) instead.
    */
    bool addJournal( Journal *journal, ResourceCalendar *resource );

    /**
      @copydoc
      Calendar::deleteJournal()
    */
    bool deleteJournal( Journal *journal );

    /**
      @copydoc
      Calendar::deleteAllJournals()
    */
    void deleteAllJournals();

    /**
      @copydoc
      Calendar::rawJournals()
    */
    Journal::List rawJournals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      @copydoc
      Calendar::rawJournalsForDate()
    */
    Journal::List rawJournalsForDate( const QDate &date );

    /**
      @copydoc
      Calendar::journal()
    */
    Journal *journal( const QString &uid );

  // Alarm Specific Methods //

    /**
      @copydoc
      Calendar::alarms()
    */
    Alarm::List alarms( const KDateTime &from, const KDateTime &to );

    /**
      Return a list of Alarms that occur before the specified timestamp.

      @param to is the ending timestamp.

      @return the list of Alarms occurring before the specified KDateTime.
    */
    Alarm::List alarmsTo( const KDateTime &to );

    using QObject::event;   // prevent warning about hidden virtual method

  Q_SIGNALS:
    /**
      Signals that the Resource has been modified.

      @param resource is a pointer to a ResourceCalendar that was changed.
      @see resourceModified()
    */
    void signalResourceModified( ResourceCalendar *resource );

    /**
      Signals that an Incidence has been inserted to the Resource.

      @param resource is a pointer to a ResourceCalendar that was added.
      @see resourceAdded()
    */
    void signalResourceAdded( ResourceCalendar *resource );

    /**
      Signals that an Incidence has been removed from the Resource.

      @param resource is a pointer to a ResourceCalendar that was removed.
      @see resourceDeleted()
    */
    void signalResourceDeleted( ResourceCalendar *resource );

    /**
      Signals an error message.
      @param err is the error message.
    */
    void signalErrorMessage( const QString &err );

  protected:
    /**
      Connects all necessary signals and slots to the resource.
      @param resource is a pointer to a ResourceCalendar.
    */
    void connectResource( ResourceCalendar *resource );

    /**
      Emits the @b signalResourceModified signal for the specified @p resource.

      @param resource is a pointer to a ResourceCalendar that was changed.
      @see signalResourceDeleted()
    */
    void resourceModified( ResourceCalendar *resource );

    /**
      Emits the @b signalResourceDeleted signal for the specified @p resource.
      @param resource is a pointer to a ResourceCalendar that was removed.
      @see signalResourceModified()
    */
    void resourceDeleted( ResourceCalendar *resource );

    /**
      @copydoc
      Calendar::doSetTimeSpec()
    */
    virtual void doSetTimeSpec( const KDateTime::Spec &timeSpec );

    /**
      Increment the number of times this Resource has been changed by 1.

      @param resource is a pointer to the ResourceCalendar to be counted.
      @return the new number of times this Resource has been changed.
      @see decrementChangeCount()
    */
    int incrementChangeCount( ResourceCalendar *resource );

    /**
      Decrement the number of times this Resource has been changed by 1.

      @param resource is a pointer to the ResourceCalendar to be counted.
      @return the new number of times this Resource has been changed.
      @see incrementChangeCount()
    */
    int decrementChangeCount( ResourceCalendar *resource );

  protected Q_SLOTS:
    /**
      Emits the @b signalErrorMessage signal with an error message
      when an error occurs loading a ResourceCalendar.

      @param resource is a pointer to the ResourceCalendar that failed to load.
      @param err is the error message.
      @see slotSaveError()
    */
    void slotLoadError( ResourceCalendar *resource, const QString &err );

    /**
      Emits the @b signalErrorMessage signal with an error message
      when an error occurs saving a ResourceCalendar.

      @param resource is a pointer to the ResourceCalendar that failed to save.
      @param err is the error message.
      @see slotLoadError()
    */
    void slotSaveError( ResourceCalendar *resource, const QString &err );

    /**
      All addIncidence( Incidence * ), addTodo( Todo * ) addEvent( Event * )
      and addJournal( Journal * ) calls made between beginAddingIncidences()
      and endAddingIncidences() will only ask the user to choose a resource once.
      @since 4.4
    */
    void beginAddingIncidences();

    /**
      @see beginAddingIncidences()
      @since 4.4
    */
    void endAddingIncidences();

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( CalendarResources )
    class Private;
    Private *d;
    //@endcond
};

}

#endif
