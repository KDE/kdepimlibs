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
  defines the Period class.

  @author Cornelius Schumacher
*/

#include "period.h"

#include <kdebug.h>
#include <klocale.h>

using namespace KCal;

//@cond PRIVATE
class KCal::Period::Private
{
  public:
    KDateTime mStart;  // period starting date/time
    KDateTime mEnd;    // period ending date/time
    bool mHasDuration; // does period have a duration?
};
//@endcond

Period::Period() : d( new KCal::Period::Private )
{
  d->mHasDuration = false;
}

Period::Period( const KDateTime &start, const KDateTime &end )
  : d( new KCal::Period::Private )
{
  d->mStart = start;
  d->mEnd = end;
  d->mHasDuration = false;
}

Period::Period( const KDateTime &start, const Duration &duration )
  : d( new KCal::Period::Private )
{
  d->mStart = start;
  d->mEnd = duration.end( start );
  d->mHasDuration = true;
}

Period::Period( const Period &period ) : d( new KCal::Period::Private )
{
    d->mStart = period.d->mStart;
    d->mEnd = period.d->mEnd;
    d->mHasDuration = period.d->mHasDuration;
}

Period::~Period()
{
  delete d;
}

bool Period::operator<( const Period &other ) const
{
  return d->mStart < other.d->mStart;
}

bool Period::operator==( const Period &other ) const
{
  return ( d->mStart == other.d->mStart &&
           d->mEnd == other.d->mEnd &&
           d->mHasDuration == other.d->mHasDuration );
}

Period &Period::operator=( const Period &other )
{
  d->mStart = other.d->mStart;
  d->mEnd = other.d->mEnd;
  d->mHasDuration = other.d->mHasDuration;
  return *this;
}

KDateTime Period::start() const
{
  return d->mStart;
}

KDateTime Period::end() const
{
  return d->mEnd;
}

Duration Period::duration() const
{
  return Duration( d->mStart, d->mEnd );
}

bool Period::hasDuration() const
{
  return d->mHasDuration;
}

void Period::shiftTimes( const KDateTime::Spec &oldSpec,
                         const KDateTime::Spec &newSpec )
{
  d->mStart = d->mStart.toTimeSpec( oldSpec );
  d->mStart.setTimeSpec( newSpec );
  d->mEnd = d->mEnd.toTimeSpec( oldSpec );
  d->mEnd.setTimeSpec( newSpec );
}
