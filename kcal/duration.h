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

#include "kcal.h"

class KDateTime;

namespace KCal {

/**
  @brief
  This class represents a duration.
*/
class KCAL_EXPORT Duration
{
  public:
    Duration();
    Duration( const KDateTime &start, const KDateTime &end );
    Duration( int seconds ); //not explicit

    KDateTime end( const KDateTime &start ) const;

    int asSeconds() const;

  private:
    int mSeconds;

    class Private;
    Private *d;
};

bool operator==( const Duration &d1, const Duration &d2 );
inline bool operator!=( const Duration &d1, const Duration &d2 )
{
  return !operator==( d1, d2 );
}

}

#endif
