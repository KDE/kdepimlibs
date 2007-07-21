/*
  This file is part of the kcal library.

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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the ICalFormat class.

  This class implements the iCalendar format. It provides methods for
  loading/saving/converting iCalendar format data into the internal
  representation as Calendar and Incidences.

  @brief
  iCalendar format implementation.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#ifndef KCAL_ICALFORMAT_H
#define KCAL_ICALFORMAT_H

#include "calformat.h"
#include "scheduler.h"

#include <kdatetime.h>

#include <QtCore/QString>
#include <QtCore/QByteArray>

namespace KCal {

class ICalFormatImpl;
class FreeBusy;

class KCAL_EXPORT ICalFormat : public CalFormat
{
  public:
    /**
      Constructor an ICalFormat with an empty ICalFormatImpl.
    */
    ICalFormat();

    /**
      Destructor.
    */
    virtual ~ICalFormat();

    /**
      Loads a calendar on disk in iCalendar format into calendar.
      Returns true if successful, else returns false. Provides more error
      information by exception().

      @param calendar Calendar object to be filled.
      @param fileName The name of the calendar file on disk.
      @return true if successful; false otherwise.
    */
    bool load( Calendar *calendar, const QString &fileName );

    /**
      Writes out the calendar to disk in iCalendar format.

      @param calendar The Calendar object to be written.
      @param fileName The name of the calendar file on disk.
      @return true if successful; false otherwise.
    */
    bool save( Calendar *calendar, const QString &fileName );

    /**
      Parse string and populate calendar with that information.

      @param calendar Calendar object to be filled.
      @param text is a QString containing the data to be parsed.

      @return true if the parsing was successful; false otherwise.
    */
    bool fromString( Calendar *calendar, const QString &text );

    /**
      Parses a string, returning the first iCal component as an Incidence.
      @return non-zero pointer if the parsing was successful; 0 otherwise.

      @param text is a QString containing the data to be parsed.
    */
    Incidence *fromString( const QString &text );

    /**
      Converts Calendar information to a QString.

      @param calendar Calendar object containing the information to be converted
      into a QString.
      @return the QString will be Null if the conversion was unsuccessful.
    */
    QString toString( Calendar *calendar );

    /**
      Converts an Incidence to iCalendar formatted text.

      @param incidence is a pointer to an Incidence object to be converted
      into iCal formatted text.
      @return the QString will be Null if the conversion was unsuccessful.
    */
    QString toICalString( Incidence *incidence );

    /**
      Converts an Incidence to a QString.
      @param incidence is a pointer to an Incidence object to be converted
      into a QString.

      @return the QString will be Null if the conversion was unsuccessful.
    */
    QString toString( Incidence *incidence );

    /**
      Return recurrence rule as iCalendar formatted text.
    */
    QString toString( RecurrenceRule *rule );

    /**
      Parse string and fill recurrence object with
      that information
    */
    bool fromString ( RecurrenceRule *rule, const QString &s );

    /**
      Create a scheduling message for event @p e using method @p m.
    */
    QString createScheduleMessage( IncidenceBase *e, Scheduler::Method m );

    /**
      Parse scheduling message provided as string @p s.
    */
    ScheduleMessage *parseScheduleMessage( Calendar *calendar, const QString &s );

    /**
      Parse FREEBUSY object.
    */
    FreeBusy *parseFreeBusy( const QString &s );

    /**
      Set time specification (time zone, etc.) used.
    */
    void setTimeSpec( const KDateTime::Spec &timeSpec );

    /**
      Return time specification used.
    */
    KDateTime::Spec timeSpec() const;

    /**
      Return id string of timezone used (if any).
    */
    QString timeZoneId() const;

    /**
      Parses a utf8 encoded string, returning the first iCal component
      encountered in that string. This is an overload used for efficient
      reading to avoid utf8 conversions, which are expensive when reading
      from disk.
    */
    bool fromRawString( Calendar *calendar, const QByteArray &s );

  protected:
    /**
      Sets the iCal format implementation.

      @param impl is a pointer to an ICalFormatImpl object.
    */
    void setImplementation( ICalFormatImpl *impl );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
