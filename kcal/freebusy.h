/*
  This file is part of the kcal library.

  Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
  defines the FreeBusy class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#ifndef KCAL_FREEBUSY_H
#define KCAL_FREEBUSY_H

#include "incidencebase.h"
#include "event.h"
#include "freebusyperiod.h"

#include <QtCore/QByteArray>

namespace KCal {

class Calendar;

/**
  @brief
  Provides information about the free/busy time of a calendar.

  A free/busy is a collection of Periods (@see Period).
*/
class KCAL_EXPORT FreeBusy : public IncidenceBase
{
  public:
    /**
      Constructs an free/busy without any periods.
    */
    FreeBusy();

    /**
      Copy constructor.
      @param other is the free/busy to copy.
    */
    FreeBusy( const FreeBusy &other );

    /**
      Constructs a free/busy from a list of periods.

      @param busyPeriods is a QList of periods.
    */
    explicit FreeBusy( const Period::List &busyPeriods );

    /**
      Constructs a free/busy from a list of periods.

      @param busyPeriods is a QList of periods.
    */
    explicit FreeBusy( const FreeBusyPeriod::List &busyPeriods );

    /**
      Constructs a free/busy from a single period.

      @param start is the start datetime of the period.
      @param end is the end datetime of the period.
    */
    FreeBusy( const KDateTime &start, const KDateTime &end );

    /**
      Constructs a freebusy for a specified calendar give a single period.

      @param calendar is a pointer to a valid Calendar object.
      @param start is the start datetime of the period.
      @param end is the end datetime of the period.
    */
    FreeBusy( Calendar *calendar, const KDateTime &start, const KDateTime &end );

    /**
      Constructs a freebusy for a specified list of events given a single period.

      @param events list of events.
      @param start is the start datetime of the period.
      @param end is the end datetime of the period.
      @since 4.4
    */
    FreeBusy( const Event::List &events, const KDateTime &start, const KDateTime &end );

    /**
      Destroys a free/busy.
    */
    ~FreeBusy();

    /**
      @copydoc
      IncidenceBase::type()
    */
    QByteArray type() const;

    /**
      @copydoc
      IncidenceBase::typeStr()
    */
    //KDE5: QString typeStr() const;

    /**
      Sets the start datetime for the free/busy. Note that this datetime
      may be later or earlier than all periods within the free/busy.

      @param start is a KDateTime specifying an start datetime.
      @see IncidenceBase::dtStart(), setDtEnd().
    */
    virtual void setDtStart( const KDateTime &start );

    /**
      Sets the end datetime for the free/busy. Note that this datetime
      may be later or earlier than all periods within the free/busy.

      @param end is a KDateTime specifying an end datetime.
      @see dtEnd(), setDtStart().
    */
    void setDtEnd( const KDateTime &end );

    /**
      Returns the end datetime for the free/busy.
      FIXME: calling addPeriod() does not change mDtEnd. Is that incorrect?
      @see setDtEnd().
    */
    virtual KDateTime dtEnd() const;

    /**
      @copydoc
      IncidenceBase::shiftTimes()
    */
    virtual void shiftTimes( const KDateTime::Spec &oldSpec,
                             const KDateTime::Spec &newSpec );

    /**
      Returns the list of all periods within the free/busy.
    */
    Period::List busyPeriods() const;

    /**
      Returns the list of all periods within the free/busy.
    */
    FreeBusyPeriod::List fullBusyPeriods() const;

    /**
      Adds a period to the freebusy list and sorts the list.

      @param start is the start datetime of the period.
      @param end is the end datetime of the period.
    */
    void addPeriod( const KDateTime &start, const KDateTime &end );

    /**
      Adds a period to the freebusy list and sorts the list.

      @param start is the start datetime of the period.
      @param duration is the Duration of the period.
    */
    void addPeriod( const KDateTime &start, const Duration &duration );

    /**
      Adds a list of periods to the freebusy object and then sorts that list.
      Use this if you are adding many items, instead of the addPeriod method,
      to avoid sorting repeatedly.

      @param list is a QList of Period objects.
    */
    void addPeriods( const Period::List &list );

    /**
      Adds a list of periods to the freebusy object and then sorts that list.
      Use this if you are adding many items, instead of the addPeriod method,
      to avoid sorting repeatedly.

      @param list is a QList of FreeBusyPeriod objects.
    */
    void addPeriods( const FreeBusyPeriod::List &list );

    /**
      Sorts the list of free/busy periods into ascending order.
    */
    void sortList();

    /**
      Merges another free/busy into this free/busy.

      @param freebusy is a pointer to a valid FreeBusy object.
    */
    void merge( FreeBusy *freebusy );

    /**
      Assignment operator.
    */
    FreeBusy &operator=( const FreeBusy &other );

    /**
      Compare this with @p freebusy for equality.

      @param freebusy is the FreeBusy to compare.
    */
    bool operator==( const FreeBusy &freebusy ) const;

  private:
    /**
     @copydoc
     IncidenceBase::accept()
    */
    bool accept( Visitor &v ) { return v.visit( this ); }

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
