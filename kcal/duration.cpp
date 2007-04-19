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
  defines the Duration class.

  @brief
  Represents a span of time measured in seconds.

  @author Cornelius Schumacher
*/

#include "duration.h"

#include <kdatetime.h>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::Duration::Private
{
  public:
    int mSeconds; // number of seconds in the duration
};
//@endcond

Duration::Duration() : d( new KCal::Duration::Private )
{
  d->mSeconds = 0;
}

Duration::Duration( const KDateTime &start, const KDateTime &end )
  : d( new KCal::Duration::Private )
{
  d->mSeconds = start.secsTo( end );
}

Duration::Duration( int seconds ) : d( new KCal::Duration::Private )
{
  d->mSeconds = seconds;
}

Duration::Duration( const Duration &duration )
  : d( new KCal::Duration::Private )
{
  d->mSeconds = duration.d->mSeconds;
}

Duration::~Duration()
{
  delete d;
}

bool Duration::operator<( const Duration &other ) const
{
  return d->mSeconds < other.d->mSeconds;
}

Duration &Duration::operator=( const Duration &duration )
{
  d->mSeconds = duration.d->mSeconds;
  return *this;
}

bool Duration::operator==( const Duration &other ) const
{
  return d->mSeconds == other.d->mSeconds;
}

bool Duration::operator!=( const Duration &other ) const
{
  return d->mSeconds != other.d->mSeconds;
}

KDateTime Duration::end( const KDateTime &start ) const
{
  return start.addSecs( d->mSeconds );
}

int Duration::asSeconds() const
{
  return d->mSeconds;
}
