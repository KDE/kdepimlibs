/*
  This file is part of the kcalcore library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

  @brief
  This class provides an Event in the sense of RFC2445.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "event.h"
#include "visitor.h"

#include <kdebug.h>

using namespace KCalCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCalCore::Event::Private
{
  public:
    Private()
      : mHasEndDate( false ),
        mTransparency( Opaque ),
        mMultiDayValid( false ),
        mMultiDay( false )
    {}
    Private( const KCalCore::Event::Private &other )
      : mDtEnd( other.mDtEnd ),
        mHasEndDate( other.mHasEndDate ),
        mTransparency( other.mTransparency ),
        mMultiDayValid( false ),
        mMultiDay( false )
    {}

    KDateTime mDtEnd;
    bool mHasEndDate;
    Transparency mTransparency;
    bool mMultiDayValid;
    bool mMultiDay;
};
//@endcond

Event::Event()
  : d( new KCalCore::Event::Private )
{
}

Event::Event( const Event &other )
  : Incidence( other ), d( new KCalCore::Event::Private( *other.d ) )
{
}

Event::~Event()
{
  delete d;
}

Event *Event::clone() const
{
  return new Event( *this );
}

IncidenceBase &Event::assign( const IncidenceBase &other )
{
  if ( &other != this ) {
    Incidence::assign( other );
    const Event *e = static_cast<const Event*>( &other );
    *d = *( e->d );
  }
  return *this;
}

bool Event::equals( const IncidenceBase &event ) const
{
  kDebug();
  if ( !Incidence::equals( event ) ) {
    return false;
  } else {
    // If they weren't the same type IncidenceBase::equals would had returned false already
    const Event *e = static_cast<const Event*>( &event );
    return
      ( ( dtEnd() == e->dtEnd() ) ||
        ( !dtEnd().isValid() && !e->dtEnd().isValid() ) ) &&
      hasEndDate() == e->hasEndDate() &&
      transparency() == e->transparency();
  }
}

Incidence::IncidenceType Event::type() const
{
  return TypeEvent;
}

QByteArray Event::typeStr() const
{
  return "Event";
}

void Event::setDtStart( const KDateTime &dt )
{
  d->mMultiDayValid = false;
  Incidence::setDtStart( dt );
}

void Event::setDtEnd( const KDateTime &dtEnd )
{
  if ( mReadOnly ) {
    return;
  }

  update();

  d->mDtEnd = dtEnd;
  d->mMultiDayValid = false;
  setHasEndDate( true );
  setHasDuration( false );

  updated();
}

KDateTime Event::dtEnd() const
{
  if ( hasEndDate() ) {
    return d->mDtEnd;
  }

  if ( hasDuration() ) {
    if ( allDay() ) {
      // For all day events, dtEnd is always inclusive
      KDateTime end = duration().end( dtStart() ).addDays( -1 );
      return end >= dtStart() ? end : dtStart();
    } else {
      return duration().end( dtStart() );
    }
  }

  // It is valid for a VEVENT to be without a DTEND. See RFC2445, Sect4.6.1.
  // Be careful to use Event::dateEnd() as appropriate due to this possibility.
  return dtStart();
}

QDate Event::dateEnd() const
{
  KDateTime end = dtEnd().toTimeSpec( dtStart() );
  if ( allDay() ) {
    return end.date();
  } else {
    return end.addSecs(-1).date();
  }
}

void Event::setHasEndDate( bool b )
{
  d->mHasEndDate = b;
}

bool Event::hasEndDate() const
{
  return d->mHasEndDate;
}

bool Event::isMultiDay( const KDateTime::Spec &spec ) const
{
  // First off, if spec's not valid, we can check for cache
  if ( ( !spec.isValid() ) && d->mMultiDayValid ) {
    return d->mMultiDay;
  }

  // Not in cache -> do it the hard way
  KDateTime start, end;

  if ( !spec.isValid() ) {
    start = dtStart();
    end = dtEnd();
  } else {
    start = dtStart().toTimeSpec( spec );
    end = dtEnd().toTimeSpec( spec );
  }

  // End date is non inclusive, so subtract 1 second... except if we
  // got the event from some braindead implementation which gave us
  // start == end one (those do happen)
  if ( start != end ) {
    end = end.addSecs( -1 );
  }

  bool multi = ( start.date() != end.date() && start <= end );

  // Update the cache
  if ( spec.isValid() ) {
    d->mMultiDayValid = true;
    d->mMultiDay = multi;
  }
  return multi;
}

void Event::shiftTimes( const KDateTime::Spec &oldSpec,
                        const KDateTime::Spec &newSpec )
{
  Incidence::shiftTimes( oldSpec, newSpec );
  if ( hasEndDate() ) {
    d->mDtEnd = d->mDtEnd.toTimeSpec( oldSpec );
    d->mDtEnd.setTimeSpec( newSpec );
  }
}

void Event::setTransparency( Event::Transparency transparency )
{
  if ( mReadOnly ) {
    return;
  }
  update();
  d->mTransparency = transparency;
  updated();
}

Event::Transparency Event::transparency() const
{
  return d->mTransparency;
}

void Event::setDuration( const Duration &duration )
{
  setHasEndDate( false );
  Incidence::setDuration( duration );
}

bool Event::accept( Visitor &v, IncidenceBase::Ptr incidence )
{
  return v.visit( incidence.staticCast<Event>() );
}

KDateTime Event::dateTime( DateTimeRole role ) const
{
  switch ( role ) {
  case RoleAlarmStartOffset:
  case RoleStartTimeZone:
  case RoleSort:
    return dtStart();
  case RoleCalendarHashing:
    return !recurs() && !isMultiDay() ? dtStart() :
                                        KDateTime();
  case RoleAlarmEndOffset:
  case RoleEndTimeZone:
  case RoleEndRecurrenceBase:
  case RoleEnd:
  case RoleDisplayEnd:
    return dtEnd();
  default:
    return KDateTime();
  }
}

void Event::setDateTime( const KDateTime &dateTime, DateTimeRole role )
{
  Q_UNUSED( dateTime );
  Q_UNUSED( role );
}

void Event::virtual_hook( int id, void *data )
{
  Q_UNUSED( id );
  Q_UNUSED( data );
  Q_ASSERT( false );
}

QLatin1String KCalCore::Event::mimeType() const
{
  return Event::eventMimeType();
}

QLatin1String Event::eventMimeType()
{
  return QLatin1String( "application/x-vnd.akonadi.calendar.event" );
}
