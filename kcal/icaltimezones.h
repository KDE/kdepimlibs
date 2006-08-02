/*
    This file is part of the kcal library.

    Copyright (c) 2005,2006 David Jarvie <software@astrojar.org.uk>

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

#ifndef KCAL_ICALTIMEZONES_H
#define KCAL_ICALTIMEZONES_H

#include <ktimezones.h>

#include "kcal.h"

#ifndef ICALTIMEZONE_DEFINED
#define ICALTIMEZONE_DEFINED
typedef struct _icaltimezone  icaltimezone;
#endif

namespace KCal {

class ICalTimeZoneSource;
class ICalTimeZoneData;
class ICalTimeZonePrivate;
class ICalTimeZoneSourcePrivate;
class ICalTimeZoneDataPrivate;
class Recurrence;


/**
 * The ICalTimeZone class represents an iCalendar VTIMEZONE component.
 *
 * ICalTimeZone instances are normally created by ICalTimeZoneSource::parse().
 *
 * @short An iCalendar time zone
 * @see ICalTimeZoneSource, ICalTimeZoneData
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KCAL_EXPORT ICalTimeZone : public KTimeZone
{
  public:
    /**
     * Creates a time zone. This constructor is normally called from ICalTimeZoneSource::parse().
     *
     * @param source   iCalendar VTIMEZONE reader and parser
     * @param name     time zone's unique name within the iCalendar object
     * @param data     parsed VTIMEZONE data
     */
    ICalTimeZone(ICalTimeZoneSource *source, const QString &name, ICalTimeZoneData *data);
    ICalTimeZone(const ICalTimeZone &);
    virtual ~ICalTimeZone();

    ICalTimeZone &operator=(const ICalTimeZone &);

    /**
     * Returns the name of the city for this time zone, if any. There is no fixed
     * format for the name.
     *
     * @return city name
     */
    QString city() const;

    /**
     * Returns the URL of the published VTIMEZONE definition, if any.
     *
     * @return URL
     */
    QByteArray url() const;

    /**
     * Returns the LAST-MODIFIED time of the VTIMEZONE, if any.
     *
     * @return time, or QDateTime() if none
     */
    QDateTime lastModified() const;

    /**
     * Returns the VTIMEZONE string which represents this time zone.
     *
     * @return VTIMEZONE string
     */
    QByteArray vtimezone() const;

    /**
     * Returns the ICal timezone structure which represents this time zone.
     * The caller is responsible for freeing the returned structure using
     * icaltimezone_free().
     *
     * @return icaltimezone structure
     */
    icaltimezone *icalTimezone() const;

  private:
    ICalTimeZonePrivate *d;
};


/**
 * A class which reads and parses iCalendar VTIMEZONE components.
 *
 * ICalTimeZoneSource is used to parse VTIMEZONE components and create
 * ICalTimeZone instances to represent them.
 *
 * @short Reader and parser for iCalendar time zone data
 * @see ICalTimeZone, ICalTimeZoneData
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KCAL_EXPORT ICalTimeZoneSource : public KTimeZoneSource
{
  public:
    /**
     * Constructs an iCalendar time zone source.
     */
    ICalTimeZoneSource();
    virtual ~ICalTimeZoneSource();

    /**
     * Creates an ICalTimeZone instance containing the detailed information parsed
     * from a VTIMEZONE component.
     *
     * @param vtimezone the VTIMEZONE component from which data is to be extracted
     * @return a ICalTimeZone instance containing the parsed data.
     *         The caller is responsible for deleting the ICalTimeZone instance.
     *         Null is returned on error.
     */
    ICalTimeZone *parse(icalcomponent *vtimezone);

    /**
     * Creates an ICalTimeZone instance for each VTIMEZONE component within a
     * CALENDAR component. The ICalTimeZone instances are added to a KTimeZones
     * collection.
     *
     * If an error occurs while processing any time zone, any remaining time zones
     * are left unprocessed.
     *
     * @param calendar the CALENDAR component from which data is to be extracted
     * @param zones    the time zones collection to which the ICalTimeZone
     *                 instances are to be added
     * @return @c false if any error occurred (either parsing a VTIMEZONE component
     *         or adding an ICalTimeZone to @p zones), @c true otherwise
     */
    bool parse(icalcomponent *calendar, KTimeZones &zones);

    /**
     * Reads an iCalendar file and creates an ICalTimeZone instance for each
     * VTIMEZONE component within it. The ICalTimeZone instances are added to a
     * KTimeZones collection.
     *
     * If an error occurs while processing any time zone, any remaining time zones
     * are left unprocessed.
     *
     * @param fileName the file from which data is to be extracted
     * @param zones    the time zones collection to which the ICalTimeZone
     *                 instances are to be added
     * @return @c false if any error occurred, @c true otherwise
     */
    bool parse(const QString &fileName, KTimeZones &zones);

    /**
     * Creates an ICalTimeZone instance containing the detailed information
     * contained in an icaltimezone structure.
     *
     * Note that an icaltimezone instance may internally refer to a built-in
     * (i.e. system) time zone, in which case the data obtained from @p tz will
     * actually be derived from the built-in time zone rather than from a
     * VTIMEZONE component.
     *
     * @param tz the icaltimezone structure from which data is to be extracted
     * @return a ICalTimeZone instance containing the time zone data.
     *         The caller is responsible for deleting the ICalTimeZone instance.
     *         Null is returned on error.
     */
    ICalTimeZone *parse(icaltimezone *tz);

  private:
    ICalTimeZoneSourcePrivate *d;
};


/**
 * Parsed iCalendar VTIMEZONE data.
 *
 * This class is used by the ICalTimeZoneSource class to pass parsed
 * data to an ICalTimeZone intance.
 *
 * @short Parsed iCalendar time zone data
 * @see ICalTimeZone, ICalTimeZoneSource
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KCAL_EXPORT ICalTimeZoneData : public KTimeZoneData
{
    friend class ICalTimeZoneSource;

  public:
    ICalTimeZoneData();
    ICalTimeZoneData(const ICalTimeZoneData &rhs);
    virtual ~ICalTimeZoneData();
    ICalTimeZoneData &operator=(const ICalTimeZoneData &rhs);

    /**
     * Creates a new copy of this object.
     * The caller is responsible for deleting the copy.
     *
     * @return copy of this instance
     */
    virtual KTimeZoneData *clone();

    /**
     * Returns the name of the city for this time zone, if any. There is no fixed
     * format for the name.
     *
     * @return city name
     */
    QString city() const;

    /**
     * Returns the URL of the published VTIMEZONE definition, if any.
     *
     * @return URL
     */
    QByteArray url() const;

    /**
     * Returns the LAST-MODIFIED time of the VTIMEZONE, if any.
     *
     * @return time, or QDateTime() if none
     */
    QDateTime lastModified() const;

    /**
     * Returns the VTIMEZONE string which represents this time zone.
     *
     * @return VTIMEZONE string
     */
    QByteArray vtimezone() const;

    /**
     * Returns the ICal timezone structure which represents this time zone.
     * The caller is responsible for freeing the returned structure using
     * icaltimezone_free().
     *
     * @return icaltimezone structure
     */
    icaltimezone *icalTimezone() const;

private:
    ICalTimeZoneDataPrivate *d;
};

}

#endif
