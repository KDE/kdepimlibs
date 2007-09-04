/*
  This file is part of the kcal library.

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

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <ksystemtimezone.h>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::Event::Private
{
  public:
    Private()
      : mHasEndDate( false ),
        mTransparency( Opaque )
    {}
    Private( const KCal::Event::Private &other )
      : mDtEnd( other.mDtEnd ),
        mHasEndDate( other.mHasEndDate ),
        mTransparency( other.mTransparency )
    {}

    KDateTime mDtEnd;
    bool mHasEndDate;
    Transparency mTransparency;
};
//@endcond

Event::Event()
  : d( new KCal::Event::Private )
{
}

Event::Event( const Event &other )
  : Incidence( other ), d( new KCal::Event::Private( *other.d ) )
{
}

Event::~Event()
{
  delete d;
}

Event *Event::clone()
{
  return new Event( *this );
}

bool Event::operator==( const Event &other ) const
{
  return
    static_cast<const Incidence &>( *this ) == static_cast<const Incidence &>( other ) &&
    dtEnd() == other.dtEnd() &&
    hasEndDate() == other.hasEndDate() &&
    transparency() == other.transparency();
}

QByteArray Event::type() const
{
  return "Event";
}

void Event::setDtEnd( const KDateTime &dtEnd )
{
  if ( mReadOnly ) {
    return;
  }

  d->mDtEnd = dtEnd;
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
    return duration().end( dtStart() );
  }

  kDebug(5800) << "Warning! Event '" << summary()
               << "' has neither end date nor duration.";
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

QString Event::dtEndTimeStr( bool shortfmt, const KDateTime::Spec &spec ) const
{
  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return KGlobal::locale()->formatTime( dtEnd().toTimeSpec( spec ).time(), shortfmt )
      + timeZone;
  } else {
    return KGlobal::locale()->formatTime( dtEnd().time(), shortfmt );
  }
}

QString Event::dtEndDateStr( bool shortfmt, const KDateTime::Spec &spec ) const
{
  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return KGlobal::locale()->formatDate(
      dtEnd().toTimeSpec( spec ).date(),
      ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) )
      + timeZone;
  } else {
    return KGlobal::locale()->formatDate(
      dtEnd().date(),
      ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
  }
}

QString Event::dtEndStr( bool shortfmt, const KDateTime::Spec &spec ) const
{
  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return KGlobal::locale()->formatDateTime(
      dtEnd().toTimeSpec( spec ).dateTime(),
      ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) )
      + timeZone;
  } else {
    return KGlobal::locale()->formatDateTime(
      dtEnd().dateTime(),
      ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
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
  // End date is non inclusive, so subtract 1 second...
  KDateTime start, end;
  if ( spec.isValid() ) {
    start = dtStart().toTimeSpec( spec );
    end = dtEnd().toTimeSpec( spec );
  } else {
    start = dtStart();
    end = dtEnd();
  }

  if ( !allDay() ) {
    end = end.addSecs( -1 );
  }

  bool multi = ( start.date() != end.date() && start <= end );
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

KDateTime Event::endDateRecurrenceBase() const
{
  return dtEnd();
}
