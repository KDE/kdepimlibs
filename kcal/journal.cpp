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
  defines the Journal class.

  @brief
  Provides a Journal in the sense of RFC2445.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "journal.h"

using namespace KCal;

Journal::Journal() : d( 0 )
{
}

Journal::~Journal()
{
}

QByteArray Journal::type() const
{
  return "Journal";
}

Journal *Journal::clone()
{
  return new Journal( *this );
}

Journal &Journal::operator=( const Journal &other )
{
  Incidence::operator=( other );
  return *this;
}

bool Journal::operator==( const Journal &journal ) const
{
    return
      static_cast<const Incidence &>( *this ) == static_cast<const Incidence &>( journal );
}

