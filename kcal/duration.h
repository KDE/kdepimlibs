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
  defines the Duration class.

  @author Cornelius Schumacher
*/
#ifndef KCAL_DURATION_H
#define KCAL_DURATION_H

#include "kcal_export.h"

class KDateTime;

namespace KCal {

/**
  @brief
  Represents a span of time measured in seconds.

  A duration is a span of time measured in seconds.  Construction
  can be done by specifying a stop and end time, or simply by
  specifying the number of seconds.
*/
class KCAL_EXPORT Duration
{
  public:
    /**
      Constructs a duration of 0 seconds.
    */
    Duration();

    /**
      Constructs a duration from @p start to @p end.

      @param start is the time the duration begins.
      @param end is the time the duration ends.
    */
    Duration( const KDateTime &start, const KDateTime &end );

    /**
      Constructs a duration with a number of @p seconds.

      @param seconds is the number of seconds in the duration.
    */
    Duration( int seconds ); //not explicit

    /**
      Constructs a duration by copying another duration object.

      @param duration is the duration to copy.
    */
    Duration( const Duration &duration );

    /**
      Destroys a duration.
    */
    ~Duration();

    /**
      Returns true if this duration is smaller than the @p other one.

      @param other is the other duration to compare.
    */
    bool operator<( const Duration &other ) const;

    /**
      Sets this duration equal to @p duration.

      @param duration is the duration to copy.
    */
    Duration &operator=( const Duration &duration );

    /**
      Returns true if this duration is equal to the @p other one.

      @param other is the other duration to compare.
    */
    bool operator==( const Duration &other ) const;

    /**
      Returns true if this duration is not equal to the @p other one.

      @param other is the other duration to compare.
    */
    bool operator!=( const Duration &other ) const;

    /**
      Computes a duration end time by adding the number of seconds
      in the duration to the specified @p start time.

      @param start is a start time.
      @return a new KDateTime representing an end time.
    */
    KDateTime end( const KDateTime &start ) const;

    /**
      Returns the length of the duration in seconds.
    */
    int asSeconds() const;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
