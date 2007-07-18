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
  defines the Event class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/
#ifndef KCAL_EVENT_H
#define KCAL_EVENT_H

#include "incidence.h"

#include <QtCore/QByteArray>

namespace KCal {

/**
  @brief
  This class provides an Event in the sense of RFC2445.
*/
class KCAL_EXPORT Event : public Incidence
{
  public:
    /**
      Event transparency types.
    */
    enum Transparency {
      Opaque,      /**< Event appears in free/busy time */
      Transparent  /**< Event does @b not appear in free/busy time */
    };

    typedef ListBase<Event> List;

    Event();
    Event( const Event & );
    ~Event();
    bool operator==( const Event & ) const;

    QByteArray type() const { return "Event"; }

    /**
      Return copy of this Event. The caller owns the returned object.
    */
    Event *clone();

    /**
      Set end date and time.
    */
    void setDtEnd( const KDateTime &dtEnd );

    /**
      Return end date and time.
    */
    virtual KDateTime dtEnd() const;

    /**
      Returns the day when the event ends. This might be different from
      dtEnd().date, since the end date/time is non-inclusive. So timed events
      ending at 0:00 have their end date on the day before.
    */
    QDate dateEnd() const;

    /**
      Returns the end time as a string formatted according to the user's
      locale settings.
      @param shortfmt If set to true, use short date format, if set to false use
                      long format.
    */
    QString dtEndTimeStr( bool shortfmt = true ) const;

    /**
      Returns the end date as a string formatted according to the user's
      locale settings.
      @param shortfmt If set to true, use short date format, if set to false use
                      long format.

      @param shortfmt if true return string in short format, if false return
                      long format
    */
    QString dtEndDateStr( bool shortfmt = true ) const;

    /**
      Returns the end date/time as string formatted according to the user's
      locale settings.
      @param shortfmt If set to true, use short date format, if set to false use
                      long format.
    */
    QString dtEndStr( bool shortfmt = true ) const;

    /**
      Set whether the event has an end date/time.
    */
    void setHasEndDate( bool );

    /**
      Return whether the event has an end date/time.
    */
    bool hasEndDate() const;

    /**
      Return true if the event spans multiple days, otherwise return false.
    */
    bool isMultiDay() const;

    /**
      @copydoc
      IncidenceBase::shiftTimes()
    */
    virtual void shiftTimes( const KDateTime::Spec &oldSpec,
                             const KDateTime::Spec &newSpec );

    /**
      Set the event's time transparency level.
    */
    void setTransparency( Transparency transparency );

    /**
      Return the event's time transparency level.
    */
    Transparency transparency() const;

    /**
      Sets duration of this event.
    */
    void setDuration( const Duration &duration );

  protected:

    /**
      Returns the end date/time of the base incidence.
    */
    virtual KDateTime endDateRecurrenceBase() const { return dtEnd(); }

  private:
    bool accept( Visitor &v ) { return v.visit( this ); }

    KDateTime mDtEnd;
    bool mHasEndDate;
    Transparency mTransparency;

    class Private;
    Private *const d;
};

}

#endif
