/*
  This file is part of the kcal library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
  defines the CalendarLocal class.

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
 */
#ifndef KCAL_CALENDARLOCAL_H
#define KCAL_CALENDARLOCAL_H

#include "calendar.h"

namespace KCal {

class CalFormat;

/**
  @brief
  This class provides a calendar stored as a local file.
*/
class KCAL_EXPORT CalendarLocal : public Calendar
{
  public:
    /**
      @copydoc
      Calendar::Calendar(const KDateTime::Spec &)
    */
    explicit CalendarLocal( const KDateTime::Spec &timeSpec );

    /**
      @copydoc
      Calendar::Calendar(const QString &)
    */
    explicit CalendarLocal( const QString &timeZoneId );

    /**
      @copydoc
      Calendar::~Calendar()
    */
    ~CalendarLocal();

    /**
      Loads a calendar on disk in vCalendar or iCalendar format into the
      current calendar. Incidences already present are preserved. If an
      incidence of the file to be loaded has the same unique id as an
      incidence already present the new incidence is ignored.

      To load a CalendarLocal object from a file without preserving existing
      incidences call close() before load().

      @param fileName the name of the calendar on disk.
      @param format the format to use. If 0, an attempt is made to load
      iCalendar format, and if that fails tries vCalendar format.
      @return true, if successful, false on error.

      @see save( const QString &, CalFormat *)
    */
    bool load( const QString &fileName, CalFormat *format = 0 );

    /**
      Reloads the contents of the storage into memory. The associated file name
      must be known, in other words a previous load() must have been executed.
      @return true if the reload was successful; false otherwise.
    */
    bool reload();

    /**
      Writes the calendar to disk. The associated file name and format must
      be known, in other words a previous load() must have been executed.

      @return true if the save was successful; false otherwise.
      @see save(const QString &, CalFormat *)
    */
    bool save();

    /**
      Writes the calendar to disk in the specified @p format.
      CalendarLocal takes ownership of the CalFormat object.

      @param fileName the name of the file
      @param format the format to use. If 0, iCalendar will be used.
      @return true if the save was successful; false otherwise.

      @see save(), load( const QString &, CalFormat *)
    */
    bool save( const QString &fileName, CalFormat *format = 0 );

    /**
      Clears out the current calendar, freeing all used memory etc. etc.
    */
    void close();

  // Event Specific Methods //

    /**
      @copydoc
      Calendar::addEvent()
    */
    bool addEvent( Event *event );

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
    Event::List rawEventsForDate(
      const QDate &date, const KDateTime::Spec &timeSpec = KDateTime::Spec(),
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      @copydoc
      Calendar::rawEventsForDate(const KDateTime &)
    */
    Event::List rawEventsForDate( const KDateTime &dt );

    /**
      @copydoc
      Calendar::event()
    */
    Event *event( const QString &uid );

  // To-do Specific Methods //

    /**
      @copydoc
      Calendar::addTodo()
    */
    bool addTodo( Todo *todo );

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
    Todo::List rawTodos(
      TodoSortField sortField = TodoSortUnsorted,
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

    /**
      Notify the IncidenceBase::Observer that the incidence has been updated.

      @param incidenceBase is a pointer to the updated IncidenceBase.
    */
    void incidenceUpdated( IncidenceBase *incidenceBase );

    using QObject::event;   // prevent warning about hidden virtual method

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( CalendarLocal )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
