/*
  Copyright (c) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and provides
  static methods for mapping Windows timezone names to/from UTC offsets
  or to/from Olson zoneinfo names.

  @author Allen Winter \<allen@kdab.com\>
*/
#ifndef KCAL_TZMAPPING_H
#define KCAL_TZMAPPING_H

#include "kcal_export.h"
#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>

namespace KCal {

namespace TZMaps
{
  /**
    Maps a Windows timezone standard name to a Windows timezone display name.
    @param standardName is a string containing a valid Windows standard
    timezone name.
    @return a QString containing the corresponding Windows timezone display
    name or empty if an invalid Windows timezone standard name was provided.
    @since 4.4
  */
  KCAL_EXPORT QString winZoneStandardToDisplay( const QString &standardName );

  /**
    Maps a Windows timezone display name to a Windows timezone standard name.
    @param displayName is a string containing a valid Windows display
    timezone name.
    @return a QString containing the corresponding Windows timezone standard
    name or empty if an invalid Windows timezone display name was provided.
    @since 4.4
  */
  KCAL_EXPORT QString winZoneDisplayToStandard( const QString &displayName );

  /**
    Maps a Windows timezone display name to an Olson zoneinfo name.
    @param windowsZone is a string containing a valid Windows timezone display name.
    @return a QString containing the corresponding Olson zoneinfo name
    or empty if an invalid Windows timezone display name was provided.
    @since 4.4
  */
  KCAL_EXPORT QString winZoneToOlson( const QString &windowsZone );

  /**
    Maps an Olson zoneinfo name to a Windows timezone display name.
    @param olsonZone is a string containing a valid Olson zoneinfo name.
    @return a QString with the corresponding Windows timezone display name
    or empty if an invalid Olson zoneinfo name was provided.
    @since 4.4
  */
  KCAL_EXPORT QString olsonToWinZone( const QString &olsonZone );

  /**
    Maps a Windows timezone display name to a UTC offset string.
    @param windowsZone is a string containing a valid Windows timezone display name.
    @return a QString containing the corresponding UTC offset
    or empty if an invalid Windows timezone display name was provided.
    A valid return string has the format "UTC(+,-)HH[:MM]"
    @since 4.4
  */
  KCAL_EXPORT QString winZoneToUtcOffset( const QString &windowsZone );

  /**
    Maps a UTC offset string to a Windows timezone display name.
    @param utcOffset is a string containing a valid UTC offset string of the
    format "UTC(+,-)HH[:MM]".
    @return a QString with the corresponding Windows timezone display name
    or empty if an invalid UTC offset string was provided.
    @since 4.4
  */
  KCAL_EXPORT QString utcOffsetToWinZone( const QString &utcOffset );

  /**
    Maps an Olson zoneinfo name to a UTC offset string.
    @param olsonZone is a string containing a valid Olson zoneinfo name.
    @return a QString containing the corresponding UTC offset
    or empty if an invalid Olson zoneinfo name was provided.
    A valid return string has the format "UTC(+,-)HH[:MM]"
    @since 4.4
  */
  KCAL_EXPORT QString olsonToUtcOffset( const QString &olsonZone );

  /**
    Maps a UTC offset string to an Olson zoneinfo name.
    @param utcOffset is a string containing a valid UTC offset string of the
    format "UTC(+,-)HH[:MM]".
    @return a QString with the corresponding Olson zoneinfo name
    or empty if an invalid UTC offset string was provided.
    @since 4.4
  */
  KCAL_EXPORT QString utcOffsetToOlson( const QString &utcOffset );

  /**
    Maps a UTC offset string to a list of timezone abbreviations.
    @param utcOffset is a string containing a valid UTC offset string of the
    format "UTC(+,-)HH[:MM]".
    @return a list of abbreviations corresponding to the specified UTC offset
    or empty if an invalid UTC offset string was provided.
    @since 4.4
  */
  KCAL_EXPORT QList<QByteArray> utcOffsetToAbbreviation( const QString &utcOffset );


  /**
    Convenience method for mapping platform independent timezone name
    (either Olson zoneinfo or Windows timezone display name) to a UTC offset.
    @param zone is a string containing a valid timezone name.
    @return a QString containing the corresponding UTC offset
    or empty if an invalid timezone name was provided.
    A valid return string has the format "UTC(+,-)HH[:MM]"
    @since 4.4
  */
  KCAL_EXPORT QString timezoneToUtcOffset( const QString &zone );

  /**
    Convenience method for mapping a UTC offset into a platform independent
    timezone name (either Olson zoneinfo or Windows timezone display name).
    @param utcOffset is a string containing a valid UTC offset string of the
    format "UTC(+,-)HH[:MM]".
    @return a QString with the corresponding timezone name
    or empty if an invalid UTC offset string was provided.
    @since 4.4
  */
  KCAL_EXPORT QString utcOffsetToTimeZone( const QString &utcOffset );
}

}

#endif
