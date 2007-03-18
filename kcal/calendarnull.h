/*
    This file is part of the kcal library.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
/*
  @file
  This file is part of the API for handling calendar data and
  defines the CalendarNull class.

  @author Cornelius Schumacher
*/
#ifndef KCAL_CALENDARNULL_H
#define KCAL_CALENDARNULL_H

#include "calendar.h"
#include "kcal.h"

class KConfig;

namespace KCal {

/**
   @brief
   Represents a null calendar class; that is, a calendar which contains
   no information and provides no capabilities.

   The null calendar can be passed to functions which need a calendar object
   when there is no real calendar available yet.

   CalendarNull can be used to implement the null object design pattern:
   pass a CalendarNull object instead of passing a 0 pointer and checking
   for 0 with each access.
*/
class KCAL_EXPORT CalendarNull : public Calendar
{
  public:
    /**
      Construct Calendar object using a time specification (time zone, etc.).
      The time specification is used for creating or modifying incidences
      in the Calendar. It is also used for viewing incidences (see
      setViewTimeSpec()). The time specification does not alter existing
      incidences.

      @param timeSpec time specification
    */
    explicit CalendarNull( const KDateTime::Spec &timeSpec );

    /**
      Constructs a null calendar with a specified time zone @p timeZoneId.

      @param timeZoneId is a string containing a time zone ID, which is
      assumed to be valid.  If no time zone is found, the viewing time
      specification is set to local clock time.
      @e Example: "Europe/Berlin"
    */
    explicit CalendarNull( const QString &timeZoneId );

    /**
      Destroys the null calendar.
    */
    ~CalendarNull();

    /**
      Returns a pointer to the CalendarNull object, of which there can
      be only one.  The object is constructed if necessary.
    */
    static CalendarNull *self();

    /**
      @copydoc
      Calendar::close()
    */
    void close() {}

    /**
      @copydoc
      Calendar::save()
    */
    void save() {}

    /**
      @copydoc
      Calendar::reload()
    */
    bool reload() { return true; }

  // Event Specific Methods //

    /**
      @copydoc
      Calendar::addEvent()
    */
    bool addEvent( Event *event ) { Q_UNUSED ( event ); return false; }

    /**
      @copydoc
      Calendar::deleteEvent()
    */
    bool deleteEvent( Event *event ) { Q_UNUSED( event ); return false; }

    /**
      @copydoc
      Calendar::rawEvents(EventSortField, SortDirection)
    */
    Event::List rawEvents( EventSortField sortField,
                           SortDirection sortDirection )
      { Q_UNUSED( sortField ); Q_UNUSED( sortDirection );
        return Event::List(); }

    /**
      @copydoc
      Calendar::rawEvents(const QDate &, const QDate &, bool)
    */
    Event::List rawEvents( const QDate &start, const QDate &end,
                           bool inclusive )
      { Q_UNUSED( start ); Q_UNUSED( end ); Q_UNUSED( inclusive );
        return Event::List(); }

    /**
      @copydoc
      Calendar::rawEventsForDate(const QDate &, EventSortField, SortDirection)
    */
    Event::List rawEventsForDate( const QDate &date,
                                  EventSortField sortField=EventSortUnsorted,
                                  SortDirection sortDirection=SortDirectionAscending )
      { Q_UNUSED( date ); Q_UNUSED( sortField ); Q_UNUSED( sortDirection );
        return Event::List(); }

    /**
      @copydoc
      Calendar::rawEventsForDate(const KDateTime &)
    */
    Event::List rawEventsForDate( const KDateTime &dt )
      { Q_UNUSED( dt ); return Event::List(); }

    /**
      @copydoc
      Calendar::event(const QString &)
    */
    Event *event( const QString &uid ) { Q_UNUSED( uid ); return 0; }

  // To-do Specific Methods //

    /**
      @copydoc
      Calendar::addTodo(Todo *)
    */
    bool addTodo( Todo *todo ) { Q_UNUSED( todo ); return false; }

    /**
      @copydoc
      Calendar::deleteTodo(Todo *)
    */
    bool deleteTodo( Todo *todo ) { Q_UNUSED( todo ); return false; }

    /**
      @copydoc
      Calendar::rawTodos(TodoSortField, SortDirection)
    */
    Todo::List rawTodos( TodoSortField sortField,
                         SortDirection sortDirection )
      { Q_UNUSED( sortField ); Q_UNUSED( sortDirection ); return Todo::List(); }

    /**
      @copydoc
      Calendar::rawTodosForDate(const QDate &)
    */
    Todo::List rawTodosForDate( const QDate &date )
      { Q_UNUSED ( date ); return Todo::List(); }

    /**
      @copydoc
      Calendar::todo(const QString &)
    */
    Todo *todo( const QString &uid ) { Q_UNUSED( uid ); return 0; }

  // Journal Specific Methods //

    /**
      @copydoc
      Calendar::addJournal(Journal *)
    */
    bool addJournal( Journal *journal ) { Q_UNUSED( journal ); return false; }

    /**
      @copydoc
      Calendar::deleteJournal(Journal *)
    */
    bool deleteJournal( Journal *journal )
      { Q_UNUSED( journal ); return false; }

    /**
      @copydoc
      Calendar::rawJournals(JournalSortField, SortDirection)
    */
    Journal::List rawJournals( JournalSortField sortField,
                               SortDirection sortDirection )
      { Q_UNUSED( sortField ); Q_UNUSED( sortDirection );
        return Journal::List(); }

    /**
      @copydoc
      Calendar::rawJournalsForDate(const QDate &)
    */
    Journal::List rawJournalsForDate( const QDate &date )
      { Q_UNUSED( date ); return Journal::List(); }

    /**
      @copydoc
      Calendar::journal(const QString &)
    */
    Journal *journal( const QString &uid ) { Q_UNUSED( uid ); return 0; }

  // Alarm Specific Methods //

    /**
      @copydoc
      Calendar::alarms(const KDateTime &, const KDateTime &)
    */
    Alarm::List alarms( const KDateTime &from, const KDateTime &to )
      { Q_UNUSED( from ); Q_UNUSED( to ); return Alarm::List(); }

  // Observer Specific Methods //

    /**
      @copydoc
      Calendar::incidenceUpdated(IncidenceBase *)
    */
    void incidenceUpdated( IncidenceBase *incidenceBase )
      { Q_UNUSED( incidenceBase ); }

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
