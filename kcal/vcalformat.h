/*
    This file is part of the kcal library.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_VCALFORMAT_H
#define KCAL_VCALFORMAT_H

#include <QtCore/QByteArray>

#include <kdatetime.h>

#include "calformat.h"
#include "todo.h"
#include "event.h"
#include "kcal.h"

#define _VCAL_VERSION "1.0"

struct VObject;

namespace KCal {

/**
  This class implements the vCalendar format. It provides methods for
  loading/saving/converting vCalendar format data into the internal KOrganizer
  representation as Calendar and Events.

  @short vCalendar format implementation
*/
class KCAL_EXPORT VCalFormat : public CalFormat
{
  public:
    VCalFormat();
    virtual ~VCalFormat();

    /**
      Loads a calendar on disk in vCalendar format into the given calendar.

      @param calendar Calendar object the loaded data is stored into.
      @param fileName Name of the vCalendar file on disk.
      @return true on success, otherwise false
    */
    bool load( Calendar *calendar, const QString &fileName );

    /**
      Writes out the given calendar to disk in vCalendar format.

      @param calendar Calendar object holding data to be written
      @param fileName the name of the file
      @return true on success, otherwise false
    */
    bool save( Calendar *calendar, const QString &fileName );

    /**
      Parse string and populate calendar with that information.
    */
    bool fromString( Calendar *calendar, const QString &text );

    /**
      Return calendar information as string.
    */
    QString toString( Calendar *calendar );

    /**
      Parse string and return first vcal component of a raw byte array of
      a utf8 encoded string. This is an overload used for efficiency reading
      to avoid utf8 conversions, which are expensive, when reading from disk.
    */
    bool fromRawString( Calendar *calendar, const QByteArray &data );

  protected:
    /**
      Translates a VObject of the TODO type into an Event.
    */
    Todo *VTodoToEvent( VObject *vtodo );

    /**
      Translates a VObject into a Event and returns a pointer to it.
    */
    Event *VEventToEvent( VObject *vevent );

    /**
      Translates an Event into a VTodo-type VObject and return pointer.
    */
    VObject *eventToVTodo( const Todo *anEvent );

    /**
      Translates an Event into a VObject and returns a pointer to it.
    */
    VObject* eventToVEvent( const Event *anEvent );

    /**
      Takes a QDate and returns a string in the format YYYYMMDDTHHMMSS.
    */
    QString qDateToISO( const QDate &date );

    /**
      Takes a QDateTime and returns a string in format YYYYMMDDTHHMMSS.
    */
    QString kDateTimeToISO( const KDateTime &date, bool zulu=true );

    /**
      Takes a string in YYYYMMDDTHHMMSS format and returns a valid QDateTime.
    */
    KDateTime ISOToKDateTime( const QString &dtStr );

    /**
      Takes a string in the YYYYMMDD format and returns a valid QDate.
    */
    QDate ISOToQDate( const QString &dtStr );

    /**
      Takes a vCalendar tree of VObjects, and puts all of them that have the
      "event" property into the dictionary, todos in the todo-list, etc.
    */
    void populate( VObject *vcal );

    /**
      Takes a number 0 - 6 and returns the two letter string of that day,
      i.e. MO, TU, WE, etc.
    */
    const char *dayFromNum( int day );

    /** the reverse of the above function. */
    int numFromDay( const QString &day );

    Attendee::PartStat readStatus( const char *s ) const;
    QByteArray writeStatus( Attendee::PartStat status ) const;

  private:
    Calendar *mCalendar;
    Event::List mEventsRelate;           // events with relations
    Todo::List mTodosRelate;             // todos with relations

    class Private;
    Private *const d;
};

}

#endif
