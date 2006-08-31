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
  defines the Period class.

  @author Cornelius Schumacher
*/

#ifndef KCAL_PERIOD_H
#define KCAL_PERIOD_H

#include <QDateTime>
#include "kcal.h"

#include "duration.h"

namespace KCal {

/**
  @brief
  Represents a period of time.

  The period can be defined by either a start time and an end time or
  by a start time and a duration.
*/
class KCAL_EXPORT Period
{
  public:
    /**
      Constructs a period.
    */
    Period();
    /**
      Constructs a period from @p start to @p end.

      @param start the time the period begins.
      @param end the time the period ends.
    */
    Period( const QDateTime &start, const QDateTime &end );
    /**
      Constructs a period from @p start and lasting @p duration.

      @param start the time when the period starts.
      @param duration how long the period lasts.
    */
    Period( const QDateTime &start, const Duration &duration );

    /**
      Returns true if this element is smaller than the @param other one 
    */
    bool operator<( const Period &other ) const;

    /**
      Returns when this period starts.
    */
    QDateTime start() const;
    /**
      Returns when this period ends.
    */
    QDateTime end() const;
    /**
      Returns the duration of the period.
    */
    Duration duration() const;
    /**
      Returns true if this period has a set duration, false
      if it just has a start and an end.
    */
    bool hasDuration() const;

  private:
    //@cond PRIVATE
    class Private;
    Private *d;
    //@endcond
};

}

#endif
