/*
    This file is part of the kcal library.

    Copyright (c) 2005-2007 David Jarvie <software@astrojar.org.uk>

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

#include <ktimezone.h>

#include "kcal_export.h"

#ifndef ICALCOMPONENT_H
typedef struct icalcomponent_impl icalcomponent;
#endif
#ifndef ICALTIMEZONE_DEFINED
#define ICALTIMEZONE_DEFINED
typedef struct _icaltimezone  icaltimezone;
#endif

namespace KCal {

class ICalTimeZone;
class ICalTimeZoneSource;
class ICalTimeZoneData;
class ICalTimeZonesPrivate;
class ICalTimeZonePrivate;
class ICalTimeZoneSourcePrivate;
class ICalTimeZoneDataPrivate;


/**
 * The ICalTimeZones class represents a time zone database which consists of a
 * collection of individual iCalendar time zone definitions.
 *
 * Each individual time zone is defined in a ICalTimeZone instance. The time zones in the
 * collection are indexed by name, which must be unique within the collection.
 * ICalTimeZone instances in the collection are owned by the ICalTimeZones instance,
 * and are deleted when the ICalTimeZones instance is destructed.
 *
 * Different calendars could define the same time zone differently. As a result,
 * to avoid conflicting definitions, each calendar should normally have its own
 * ICalTimeZones collection.
 *
 * This class is analogous to KTimeZones, but holds ICalTimeZone instances rather
 * than generic KTimeZone instances.
 *
 * @short Represents a collection of iCalendar time zones
 * @author David Jarvie <software@astrojar.org.uk>.
 */
class KCAL_EXPORT ICalTimeZones
{
public:
    ICalTimeZones();
    ~ICalTimeZones();

    /**
     * Returns the time zone with the given name.
     * Note that the ICalTimeZone returned remains a member of the ICalTimeZones
     * collection, and should not be deleted without calling detach() first.
     *
     * @param name name of time zone
     * @return time zone, or 0 if not found
     */
    const ICalTimeZone *zone(const QString &name) const;

    typedef QMap<QString, const ICalTimeZone*> ZoneMap;

    /**
     * Returns all the time zones defined in this collection.
     *
     * @return time zone collection
     */
    const ZoneMap zones() const;

    /**
     * Adds a time zone to the collection.
     * ICalTimeZones takes ownership of the ICalTimeZone instance, which will be deleted
     * when the ICalTimeZones instance is destructed.
     * The time zone's name must be unique within the collection.
     *
     * @param zone time zone to add
     * @return @c true if successful, @c false if zone's name duplicates one already in the collection
     */
    bool add(ICalTimeZone *zone);

    /**
     * Adds a time zone to the collection.
     * ICalTimeZones does not take ownership of the ICalTimeZone instance.
     * The time zone's name must be unique within the collection.
     *
     * @param zone time zone to add
     * @return @c true if successful, @c false if zone's name duplicates one already in the collection
     */
    bool addConst(const ICalTimeZone *zone);

    /**
     * Removes a time zone from the collection.
     * The caller assumes responsibility for deleting the removed ICalTimeZone. If
     * the removed ICalTimeZone was created by the caller, the constness of the return
     * value may safely be cast away.
     *
     * @param zone time zone to remove
     * @return the time zone which was removed, or 0 if not found or not a deletable object
     */
    const ICalTimeZone *detach(const ICalTimeZone *zone);

    /**
     * Removes a time zone from the collection.
     * The caller assumes responsibility for deleting the removed ICalTimeZone.
     *
     * @param name name of time zone to remove
     * @return the time zone which was removed, or 0 if not found or not a deletable object
     */
    const ICalTimeZone *detach(const QString &name);

    /**
     * Clears the collection.
     * All time zone instances owned by the collection are deleted.
     */
    void clear();

    /**
     * Returns a standard UTC time zone, with name "UTC".
     *
     * @note The ICalTimeZone returned by this method does not belong to any
     * ICalTimeZones collection, and is statically allocated and therefore cannot
     * be deleted and cannot be added to a ICalTimeZones collection. Any ICalTimeZones
     * instance may contain its own UTC ICalTimeZone, but that will be a different
     * instance than this ICalTimeZone.
     *
     * @return UTC time zone
     */
    static const ICalTimeZone *utc();

private:
    ICalTimeZones(const ICalTimeZones &);              // prohibit copying
    ICalTimeZones &operator=(const ICalTimeZones &);   // prohibit copying

    ICalTimeZonesPrivate *const d;
};


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

    /**
     * Constructor which converts a KTimeZone to an ICalTimeZone instance.
     *
     * @param tz KTimeZone instance
     * @param earliest earliest date for which time zone data should be stored
     */
    explicit ICalTimeZone(const KTimeZone &tz, const QDate &earliest = QDate());

    /**
     * Copy constructor.
     */
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

    /**
     * Return whether daylight saving transitions are available for the time zone.
     *
     * @return @c true
     */
    virtual bool hasTransitions() const;

    /**
     * Update the definition of the time zone to be identical to another
     * ICalTimeZone instance. A prerequisite is that the two instances must
     * have the same name.
     *
     * The purpose of this method is to enable updates of ICalTimeZone
     * definitions when a calendar is reloaded, without invalidating pointers
     * to the instance (particularly pointers held by KDateTime objects).
     *
     * @param other time zone whose definition is to be used
     * @return true if definition was updated (i.e. names are the same)
     */
    bool update(const ICalTimeZone *other);

  private:
    ICalTimeZonePrivate *const d;
};


/**
 * A class which reads and parses iCalendar VTIMEZONE components, and accesses
 * libical time zone data.
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
     * CALENDAR component. The ICalTimeZones collection is updated with each new
     * instance: if a time zone of the same name already exists in the collection,
     * the existing ICalTimeZone instance is updated with the new data, else the
     * new ICalTimeZone instance is added to the collection.
     *
     * If an error occurs while processing any time zone, any remaining time zones
     * are left unprocessed.
     *
     * @param calendar the CALENDAR component from which data is to be extracted
     * @param zones    the time zones collection to be updated with the new
     *                 ICalTimeZone instances
     * @return @c false if any error occurred (either parsing a VTIMEZONE component
     *         or adding an ICalTimeZone to @p zones), @c true otherwise
     */
    bool parse(icalcomponent *calendar, ICalTimeZones &zones);

    /**
     * Reads an iCalendar file and creates an ICalTimeZone instance for each
     * VTIMEZONE component within it. The ICalTimeZone instances are added to a
     * ICalTimeZones collection, or existing instances in the collection are
     * updated.
     *
     * If an error occurs while processing any time zone, any remaining time zones
     * are left unprocessed.
     *
     * @param fileName the file from which data is to be extracted
     * @param zones    the time zones collection to be updated with the new
     *                 ICalTimeZone instances
     * @return @c false if any error occurred, @c true otherwise
     */
    bool parse(const QString &fileName, ICalTimeZones &zones);

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

    /**
     * Creates an ICalTimeZone instance for a standard time zone. The system
     * time zone definition is used in preference; otherwise, the built-in
     * libical time zone definition is used.
     *
     * @param zone time zone name, which may optionally include the libical
     *             prefix string
     * @param icalBuiltIn @p true to fetch only the libical built-in time zone,
     *                    and ignore system time zone definitions
     * @return a ICalTimeZone instance containing the time zone data.
     *         The caller is responsible for deleting the ICalTimeZone instance.
     *         Null is returned on error.
     */
    ICalTimeZone *standardZone(const QString &zone, bool icalBuiltIn = false);

    /**
     * Returns the prefix string used in the TZID field in built-in libical
     * time zones. The prefix string starts and ends with '/'. The name
     * normally used for the time zone is obtained by stripping the prefix and
     * the following characters up to the next '/', inclusive.
     *
     * @return prefix string
     */
    static QByteArray icalTzidPrefix();

  private:
    ICalTimeZoneSourcePrivate *const d;
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
    /**
     * Constructor which converts a KTimeZoneData to an ICalTimeZoneData instance.
     * If @p data is for a system time zone (i.e. @p tz is a KSystemTimeZone
     * instance), the full time zone data is read from the system time zone
     * database if possible; otherwise, the built-in libical time zone's data
     * is used.
     *
     * @param rhs KTimeZoneData instance
     * @param tz  time zone which @p rhs belongs to
     * @param earliest earliest date for which time zone data should be stored
     */
    ICalTimeZoneData(const KTimeZoneData &rhs, const KTimeZone &tz, const QDate &earliest);
    virtual ~ICalTimeZoneData();
    ICalTimeZoneData &operator=(const ICalTimeZoneData &rhs);

    /**
     * Creates a new copy of this object.
     * The caller is responsible for deleting the copy.
     *
     * @return copy of this instance
     */
    virtual KTimeZoneData *clone() const;

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

    /**
     * Return whether daylight saving transitions are available for the time zone.
     *
     * @return @c true
     */
    virtual bool hasTransitions() const;

private:
    ICalTimeZoneDataPrivate *const d;
};

}

#endif
