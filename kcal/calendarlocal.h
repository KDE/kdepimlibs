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

  @author Preston Brown
  @author Cornelius Schumacher
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
      Constructs a new calendar, with variables initialized to sane values.
    */
    explicit CalendarLocal( const KDateTime::Spec &timeSpec );
    /**
      Constructs a new calendar, with variables initialized to sane values.
    */
    explicit CalendarLocal( const QString &timeZoneId );

    /**
      Destructor.
    */
    ~CalendarLocal();

    /**
      Loads a calendar on disk in vCalendar or iCalendar format into the current
      calendar. Incidences already present are preserved. If an event of the
      file to be loaded has the same unique id as an incidence already present
      the new incidence is ignored.

      To load a CalendarLocal object from a file without preserving existing
      incidences call close() before load().

      @return true, if successful, false on error.
      @param fileName the name of the calendar on disk.
      @param format the format to use. If 0, iCalendar and vCalendar will be used
    */
    bool load( const QString &fileName, CalFormat *format = 0 );

    /**
     * Reloads the contents of the storage into memory. The associated file name
     * must be known, in other words a previous load() must have been executed.
     * @return success or failure
    */
    bool reload();

    /**
      Writes out the calendar to disk in the specified \a format.
      CalendarLocal takes ownership of the CalFormat object.
      @param fileName the name of the file
      @param format the format to use
      @return true, if successful, false on error.
    */
    bool save( const QString &fileName, CalFormat *format = 0 );

    /**
      Clears out the current calendar, freeing all used memory etc. etc.
    */
    void close();

    void save() {}

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
      >Deletes all events from this calendar.
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
      Calendar::rawEvents(const QDate &, const QDate &, bool)
    */
    Event::List rawEvents( const QDate &start, const QDate &end,
                           bool inclusive = false );

    /**
      @copydoc
      Calendar::rawEventsForDate(const QDate &, EventSortField, SortDirection)
    */
    Event::List rawEventsForDate(
      const QDate &date, EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      @copydoc
      Calendar::rawEventsForDate(const KDateTime &)
    */
    Event::List rawEventsForDate( const KDateTime &dt );

    /**
      @copydoc
      Calendar::event(const QString &)
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
      Calendar::deleteTodo(Todo *)
    */
    bool deleteTodo( Todo *todo );

    /**
      Deletes all to-dos from this calendar.
    */
    void deleteAllTodos();

    /**
      @copydoc
      Calendar::rawTodos(TodoSortField, SortDirection)
    */
    Todo::List rawTodos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      @copydoc
      Calendar::rawTodosForDate(const QDate &)
    */
    Todo::List rawTodosForDate( const QDate &date );

    /**
      @copydoc
      Calendar::todo(const QString &)
    */
    Todo *todo( const QString &uid );

  // Journal Specific Methods //

    /**
      @copydoc
      Calendar::addJournal(Journal *)
    */
    bool addJournal( Journal *journal );

    /**
      @copydoc
      Calendar::deleteJournal(Journal *)
    */
    bool deleteJournal( Journal *journal );

    /**
      >Deletes all journals from this calendar.
    */
    void deleteAllJournals();

    /**
       @copydoc
       Calendar::rawJournals(JournalSortField, SortDirection)
    */
    Journal::List rawJournals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      @copydoc
      Calendar::rawJournalsForDate(const QDate &)
    */
    Journal::List rawJournalsForDate( const QDate &date );

    /**
      @copydoc
      Calendar::journal(const QString &)
    */
    Journal *journal( const QString &uid );

  // Alarm Specific Methods //

    /**
      @copydoc
      Calendar::alarms(const KDateTime &, const KDateTime &)
    */
    Alarm::List alarms( const KDateTime &from, const KDateTime &to );

    /**
      >Return all alarms, which occur before given date.
    */
    Alarm::List alarmsTo( const KDateTime &to );

  private:
    /** inserts an event into its "proper place" in the calendar. */
    void insertEvent( Event *event );
    void insertTodo( Todo *todo );
    void insertJournal( Journal *journal );

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
