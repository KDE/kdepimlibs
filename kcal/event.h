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
#include <kpimutils/supertrait.h>
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
      The different Event transparency types.
    */
    enum Transparency {
      Opaque,      /**< Event appears in free/busy time */
      Transparent  /**< Event does @b not appear in free/busy time */
    };

    /**
      List of events.
    */
    typedef ListBase<Event> List;

    /**
      A shared pointer to an Event object.
    */
    typedef boost::shared_ptr<Event> Ptr;

    /**
      A shared pointer to a non-mutable Event.
    */
    typedef boost::shared_ptr<const Event> ConstPtr;

    /**
      Constructs an event.
    */
    Event();

    /**
      Copy constructor.
      @param other is the event to copy.
    */
    Event( const Event &other );

    /**
      Destroys the event.
    */
    ~Event();

    /**
      Assignment operator.
      @param other is the event to assign.
    */
    Event &operator=( const Event &other );

    /**
      Compares two events for equality.
      @param event is the event to compare.
    */
    bool operator==( const Event &event ) const;

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
      Returns an exact copy of this Event. The caller owns the returned object.
    */
    Event *clone();

    /**
      Sets the event end date and time.
      @param dtEnd is a KDateTime specifying when the event ends.
      @see dtEnd(), dateEnd().
    */
    void setDtEnd( const KDateTime &dtEnd );

    /**
      Returns the event end date and time.
      @see setDtEnd().
    */
    virtual KDateTime dtEnd() const;

    /**
      Returns the date when the event ends. This might be different from
      dtEnd().date, since the end date/time is non-inclusive. So timed events
      ending at 0:00 have their end date on the day before.
    */
    QDate dateEnd() const;

    /**
      Returns the event end time as a string formatted according to the
      user's locale settings.

      @param shortfmt If set, use short date format; else use long format.
      @param spec If set, return the time in the given spec, else use the
      event's current spec.

      @deprecated use IncidenceFormatter::timeToString()
    */
    KDE_DEPRECATED QString dtEndTimeStr(
      bool shortfmt = true, const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Returns the event end date as a string formatted according to the
      user's locale settings.

      @param shortfmt If set, use short date format; else use long format.
      @param spec If set, return the date in the given spec, else use the
      event's current spec.

      @deprecated use IncidenceFormatter::dateToString()
    */
    KDE_DEPRECATED QString dtEndDateStr(
      bool shortfmt = true, const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Returns the event end date/time as string formatted according to the
      user's locale settings.

      @param shortfmt If set, use short date format; else use long format.
      @param spec If set, return the date/time in the given spec, else use
      the event's current spec.

      @deprecated use IncidenceFormatter::dateTimeToString()
    */
    KDE_DEPRECATED QString dtEndStr(
      bool shortfmt = true, const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Sets whether the event has an end date/time.
      @param b If set, indicates the event has an end date.
    */
    void setHasEndDate( bool b );

    /**
      Returns whether the event has an end date/time.
    */
    bool hasEndDate() const;

    /**
      Returns true if the event spans multiple days, otherwise return false.

      @param spec If set, looks if the event is multiday for the given spec.
      If not set, looks if event this multiday for its spec.
    */
    bool isMultiDay( const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      @copydoc
      IncidenceBase::shiftTimes()
    */
    virtual void shiftTimes( const KDateTime::Spec &oldSpec,
                             const KDateTime::Spec &newSpec );

    /**
      Sets the event's time transparency level.
      @param transparency is the event Transparency level.
    */
    void setTransparency( Transparency transparency );

    /**
      Returns the event's time transparency level.
    */
    Transparency transparency() const;

    /**
      Sets the duration of this event.
      @param duration is the event Duration.
    */
    void setDuration( const Duration &duration );

  protected:
    /**
      Returns the end date/time of the base incidence.
    */
    virtual KDateTime endDateRecurrenceBase() const;

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

//@cond PRIVATE
// super class trait specialization
namespace KPIMUtils {
  template <> struct SuperClass<KCal::Event> : public SuperClassTrait<KCal::Incidence>{};
}
//@endcond

#endif
