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

  @author Cornelius Schumacher
*/

#include "event.h"

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

using namespace KCal;

Event::Event() :
  mHasEndDate( false ), mTransparency( Opaque ), d( 0 )
{
}

Event::Event( const Event &e ) :
  Incidence( e ), d( 0 )
{
  mDtEnd = e.mDtEnd;
  mHasEndDate = e.mHasEndDate;
  mTransparency = e.mTransparency;
}

Event::~Event()
{
}

Event *Event::clone()
{
  return new Event( *this );
}

bool Event::operator==( const Event &e2 ) const
{
  return
    static_cast<const Incidence&>(*this) == static_cast<const Incidence&>(e2) &&
    dtEnd() == e2.dtEnd() &&
    hasEndDate() == e2.hasEndDate() &&
    transparency() == e2.transparency();
}

void Event::setDtEnd( const KDateTime &dtEnd )
{
  if ( mReadOnly ) {
    return;
  }

  mDtEnd = dtEnd;

  setHasEndDate( true );
  setHasDuration( false );

  updated();
}

KDateTime Event::dtEnd() const
{
  if ( hasEndDate() ) {
    return mDtEnd;
  }
  if ( hasDuration() ) {
    return dtStart().addSecs( duration() );
  }

  kDebug(5800) << "Warning! Event '" << summary()
               << "' has neither end date nor duration." << endl;
  return dtStart();
}

QDate Event::dateEnd() const
{
  KDateTime end = dtEnd().toTimeSpec( dtStart() );
  if ( floats() ) {
    return end.date();
  } else {
    return end.addSecs(-1).date();
  }
}

QString Event::dtEndTimeStr( bool shortfmt ) const
{
  return KGlobal::locale()->formatTime( dtEnd().time(), shortfmt );
}

QString Event::dtEndDateStr( bool shortfmt ) const
{
  return
    KGlobal::locale()->formatDate( dtEnd().date(),
                                   ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
}

QString Event::dtEndStr( bool shortfmt ) const
{
  return
    KGlobal::locale()->formatDateTime( dtEnd().dateTime(),
                                       ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
}

void Event::setHasEndDate( bool b )
{
  mHasEndDate = b;
}

bool Event::hasEndDate() const
{
  return mHasEndDate;
}

bool Event::isMultiDay() const
{
  // End date is non inclusive, so subtract 1 second...
  KDateTime start( dtStart() );
  KDateTime end( dtEnd() );
  if ( ! floats() ) {
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
    mDtEnd = mDtEnd.toTimeSpec( oldSpec );
    mDtEnd.setTimeSpec( newSpec );
  }
}

void Event::setTransparency( Event::Transparency transparency )
{
  if ( mReadOnly ) {
    return;
  }
  mTransparency = transparency;
  updated();
}

Event::Transparency Event::transparency() const
{
  return mTransparency;
}

void Event::setDuration( int seconds )
{
  setHasEndDate( false );
  Incidence::setDuration( seconds );
}

// DEPRECATED methods
void Event::setDtEnd( const QDateTime &dtEnd )
{
  if (dtStart().isValid()) {
    // use start as best guess for time zone
    setDtEnd( KDateTime( dtEnd, dtStart().timeSpec() ) );
  } else {
    // use local time zone
    setDtEnd( KDateTime( dtEnd ) );
  }
}
