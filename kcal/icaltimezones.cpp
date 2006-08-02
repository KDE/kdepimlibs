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

#include <QDateTime>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include <kdebug.h>

extern "C" {
  #include <ical.h>
  #include <icaltimezone.h>
}

#include "icalformat.h"
#include "icalformatimpl.h"
#include "icaltimezones.h"


using namespace KCal;


// Convert an ical time to QDateTime, preserving the UTC indicator
static QDateTime toQDateTime(const icaltimetype &t)
{
  return QDateTime(QDate(t.year, t.month, t.day),
                   QTime(t.hour, t.minute, t.second),
                   (t.is_utc ? Qt::UTC : Qt::LocalTime));
}

// Maximum date for time zone data.
// It's not sensible to try to predict them very far in advance, because
// they can easily change. Plus, it limits the processing required.
static QDateTime MAX_DATE()
{
  static QDateTime dt;
  if ( !dt.isValid() )
    dt = QDateTime( QDate::currentDate().addYears(20), QTime(0,0,0) );
  return dt;
}


namespace KCal {


/******************************************************************************/

ICalTimeZone::ICalTimeZone(ICalTimeZoneSource *source, const QString &name, ICalTimeZoneData *data)
  : KTimeZone(source, name)
{
  setData(data);
}

ICalTimeZone::ICalTimeZone(const ICalTimeZone &tz)
  : KTimeZone(tz)
{
}

ICalTimeZone::~ICalTimeZone()
{
}

ICalTimeZone &ICalTimeZone::operator=(const ICalTimeZone &tz)
{
  KTimeZone::operator=(tz);
  return *this;
}

QString ICalTimeZone::city() const
{
  const ICalTimeZoneData *dat = static_cast<const ICalTimeZoneData*>(data());
  return dat ? dat->city() : QString();
}

QByteArray ICalTimeZone::url() const
{
  const ICalTimeZoneData *dat = static_cast<const ICalTimeZoneData*>(data());
  return dat ? dat->url() : QByteArray();
}

QDateTime ICalTimeZone::lastModified() const
{
  const ICalTimeZoneData *dat = static_cast<const ICalTimeZoneData*>(data());
  return dat ? dat->lastModified() : QDateTime();
}

QByteArray ICalTimeZone::vtimezone() const
{
  const ICalTimeZoneData *dat = static_cast<const ICalTimeZoneData*>(data());
  return dat ? dat->vtimezone() : QByteArray();
}

icaltimezone *ICalTimeZone::icalTimezone() const
{
  const ICalTimeZoneData *dat = static_cast<const ICalTimeZoneData*>(data());
  return dat ? dat->icalTimezone() : 0;
}


/******************************************************************************/

class ICalTimeZoneDataPrivate
{
public:
    ICalTimeZoneDataPrivate() : icalComponent(0) {}
    ~ICalTimeZoneDataPrivate()
    {
      if (icalComponent)
        icalcomponent_free(icalComponent);
    }
    void setComponent(icalcomponent *c)
    {
      if (icalComponent)
        icalcomponent_free(icalComponent);
      icalComponent = c;
    }
    QString       location;       // name of city for this time zone
    QByteArray    url;            // URL of published VTIMEZONE definition (optional)
    QDateTime     lastModified;   // time of last modification of the VTIMEZONE component (optional)
    icalcomponent *icalComponent; // ical component representing this time zone
};


ICalTimeZoneData::ICalTimeZoneData()
  : d(new ICalTimeZoneDataPrivate())
{
}

ICalTimeZoneData::ICalTimeZoneData(const ICalTimeZoneData &rhs)
  : KTimeZoneData(rhs),
    d(new ICalTimeZoneDataPrivate())
{
  d->location      = rhs.d->location;
  d->url           = rhs.d->url;
  d->lastModified  = rhs.d->lastModified;
  d->icalComponent = icalcomponent_new_clone(rhs.d->icalComponent);
}

ICalTimeZoneData::~ICalTimeZoneData()
{
  delete d;
}

ICalTimeZoneData &ICalTimeZoneData::operator=(const ICalTimeZoneData &rhs)
{
  KTimeZoneData::operator=(rhs);
  d->location      = rhs.d->location;
  d->url           = rhs.d->url;
  d->lastModified  = rhs.d->lastModified;
  d->setComponent( icalcomponent_new_clone(rhs.d->icalComponent) );
  return *this;
}

KTimeZoneData *ICalTimeZoneData::clone()
{
  return new ICalTimeZoneData(*this);
}

QString ICalTimeZoneData::city() const
{
  return d->location;
}

QByteArray ICalTimeZoneData::url() const
{
  return d->url;
}

QDateTime ICalTimeZoneData::lastModified() const
{
  return d->lastModified;
}

QByteArray ICalTimeZoneData::vtimezone() const
{
  return icalcomponent_as_ical_string( d->icalComponent );
}

icaltimezone *ICalTimeZoneData::icalTimezone() const
{
  icaltimezone *icaltz = icaltimezone_new();
  if ( !icaltz )
    return 0;
  if ( !icaltimezone_set_component(icaltz, d->icalComponent) )
  {
    icaltimezone_free(icaltz, 1);
    return 0;
  }
  return icaltz;
}


/******************************************************************************/

class ICalTimeZoneSourcePrivate
{
  public:
    static KTimeZonePhase *parsePhase(icalcomponent*, bool daylight);
};


ICalTimeZoneSource::ICalTimeZoneSource()
{
}

ICalTimeZoneSource::~ICalTimeZoneSource()
{
}

bool ICalTimeZoneSource::parse(const QString &fileName, KTimeZones &zones)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
    return false;
  QTextStream ts(&file);
  ts.setCodec( "ISO 8859-1" );
  QByteArray text = ts.readAll().trimmed().toLatin1();
  file.close();

  bool result = false;
  icalcomponent *calendar = icalcomponent_new_from_string(text.data());
  if (calendar) {
    if (icalcomponent_isa(calendar) == ICAL_VCALENDAR_COMPONENT)
      result = parse(calendar, zones);
    icalcomponent_free(calendar);
  }
  return result;
}

bool ICalTimeZoneSource::parse(icalcomponent *calendar, KTimeZones &zones)
{
  for (icalcomponent *c = icalcomponent_get_first_component(calendar, ICAL_VTIMEZONE_COMPONENT);
       c;  c = icalcomponent_get_next_component(calendar, ICAL_VTIMEZONE_COMPONENT))
  {
    ICalTimeZone *zone = parse(c);
    if (!zone)
      return false;
    if (!zones.add(zone)) {
      delete zone;
      return false;
    }
  }
  return true;
}

ICalTimeZone *ICalTimeZoneSource::parse(icalcomponent *vtimezone)
{
  QString name;
  QString xlocation;
  ICalTimeZoneData* data = new ICalTimeZoneData();

  // Read the fixed properties which can only appear once in VTIMEZONE
  icalproperty *p = icalcomponent_get_first_property(vtimezone, ICAL_ANY_PROPERTY);
  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_TZID_PROPERTY:
        name = QString::fromUtf8(icalproperty_get_tzid(p));
        break;

      case ICAL_TZURL_PROPERTY:
        data->d->url = icalproperty_get_tzurl(p);
        break;

      case ICAL_LOCATION_PROPERTY:
        // This isn't mentioned in RFC2445, but libical reads it ...
        data->d->location = QString::fromUtf8(icalproperty_get_location(p));
        break;

      case ICAL_X_PROPERTY: {   // use X-LIC-LOCATION if LOCATION is missing
        const char *xname = icalproperty_get_x_name(p);
        if (xname  &&  !strcmp(xname, "X-LIC-LOCATION"))
            xlocation = QString::fromUtf8(icalproperty_get_x(p));
        break;
      }
      case ICAL_LASTMODIFIED_PROPERTY: {
        icaltimetype t = icalproperty_get_lastmodified(p);
        if (t.is_utc) {
          data->d->lastModified = toQDateTime(t);
        } else {
          kDebug(5800) << "ICalTimeZoneSource::parse(): LAST-MODIFIED not UTC" << endl;
        }
        break;
		}
      default:
        break;
    }
    p = icalcomponent_get_next_property(vtimezone, ICAL_ANY_PROPERTY);
  }

  if (name.isEmpty()) {
    kDebug(5800) << "ICalTimeZoneSource::parse(): TZID missing" << endl;
    delete data;
    return 0;
  }
  if (data->d->location.isEmpty()  &&  !xlocation.isEmpty())
    data->d->location = xlocation;
  kDebug(5800) << "---zoneId: \"" << name << '"' << endl;

  /*
   * Iterate through all time zone rules for this VTIMEZONE,
   * and create a Phase object containing details for each one.
   */
  QList<KTimeZonePhase> phases;
  for (icalcomponent *c = icalcomponent_get_first_component(vtimezone, ICAL_ANY_COMPONENT);
       c;  c = icalcomponent_get_next_component(vtimezone, ICAL_ANY_COMPONENT))
  {
    KTimeZonePhase *phase = 0;
    icalcomponent_kind kind = icalcomponent_isa(c);
    switch (kind) {

      case ICAL_XSTANDARD_COMPONENT:
        kDebug(5800) << "---standard phase: found" << endl;
        phase = ICalTimeZoneSourcePrivate::parsePhase(c, false);
        break;

      case ICAL_XDAYLIGHT_COMPONENT:
        kDebug(5800) << "---daylight phase: found" << endl;
        phase = ICalTimeZoneSourcePrivate::parsePhase(c, true);
        break;

      default:
        kDebug(5800) << "ICalTimeZoneSource::parse(): Unknown component: " << kind << endl;
        break;
    }
    if (phase  &&  phase->isValid())
      phases += *phase;
    delete phase;
  }
  data->setPhases(phases);

  data->d->setComponent( icalcomponent_new_clone(vtimezone) );
  return new ICalTimeZone(this, name, data);
}

ICalTimeZone *ICalTimeZoneSource::parse(icaltimezone *tz)
{
  /* Parse the VTIMEZONE component stored in the icaltimezone structure.
   * This is both easier and provides more complete information than
   * extracting already parsed data from icaltimezone.
   */
  return parse(icaltimezone_get_component(tz));
}

KTimeZonePhase *ICalTimeZoneSourcePrivate::parsePhase(icalcomponent *c, bool daylight)
{
  // Read the observance data for this standard/daylight savings phase
  QList<QByteArray> abbrevs;
  QString   comment;
  int       utcOffset = 0;
  int       prevOffset = 0;
  bool recurs             = false;
  bool found_dtstart      = false;
  bool found_tzoffsetfrom = false;
  bool found_tzoffsetto   = false;
  icaltimetype dtstart = icaltime_null_time();

  // Now do the ical reading.
  icalproperty *p = icalcomponent_get_first_property(c, ICAL_ANY_PROPERTY);
  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_TZNAME_PROPERTY:     // abbreviated name for this time offset
      {
        // TZNAME can appear multiple times in order to provide language
        // translations of the time zone offset name.
#warning Does this cope with multiple language specifications?
        QByteArray tzname = icalproperty_get_tzname(p);
        // Outlook (2000) places "Standard Time" and "Daylight Time" in the TZNAME
        // strings, which is totally useless. So ignore those.
        if (!daylight  &&  tzname == "Standard Time"
        ||  daylight  &&  tzname == "Daylight Time")
          break;
        if (abbrevs.indexOf(tzname))
          abbrevs += tzname;
        break;
      }
      case ICAL_DTSTART_PROPERTY:      // local time at which phase starts
        dtstart = icalproperty_get_dtstart(p);
        found_dtstart = true;
        break;

      case ICAL_TZOFFSETFROM_PROPERTY:    // UTC offset immediately before start of phase
        prevOffset = icalproperty_get_tzoffsetfrom(p);
        found_tzoffsetfrom = true;
        break;

      case ICAL_TZOFFSETTO_PROPERTY:
        utcOffset = icalproperty_get_tzoffsetto(p);
        found_tzoffsetto = true;
        break;

      case ICAL_COMMENT_PROPERTY:
        comment = QString::fromUtf8(icalproperty_get_comment(p));
        break;

      case ICAL_RDATE_PROPERTY:
      case ICAL_RRULE_PROPERTY:
        recurs = true;
        break;

      default:
        kDebug(5800) << "ICalTimeZoneSource::readPhase(): Unknown property: " << kind << endl;
        break;
    }
    p = icalcomponent_get_next_property(c, ICAL_ANY_PROPERTY);
  }

  // Validate the phase data
  if (!found_dtstart || !found_tzoffsetfrom || !found_tzoffsetto) {
    kDebug(5800) << "ICalTimeZoneSource::readPhase(): DTSTART/TZOFFSETFROM/TZOFFSETTO missing" << endl;
    return 0;
  }

  // Convert DTSTART to QDateTime, and from local time to UTC
  QDateTime localStart = toQDateTime(dtstart);   // local time
  dtstart.second -= prevOffset;
  dtstart.is_utc = 1;
  QDateTime utcStart = toQDateTime(icaltime_normalize(dtstart));   // UTC

  QList<QDateTime> times;
  times += utcStart;
kDebug()<<" .. DTSTART: "<<utcStart<<(utcStart.timeSpec()==Qt::UTC?" UTC":" Local")<<endl;
  if (recurs) {
    /* RDATE or RRULE is specified. There should only be one or the other, but
     * it doesn't really matter - the code can cope with both.
     * Note that we had to get DTSTART, TZOFFSETFROM, TZOFFSETTO before reading
     * recurrences.
     */
    Recurrence recur;
    icalproperty *p = icalcomponent_get_first_property(c, ICAL_ANY_PROPERTY);
    while (p) {
      icalproperty_kind kind = icalproperty_isa(p);
      switch (kind) {

        case ICAL_RDATE_PROPERTY:
        {
          icaltimetype t = icalproperty_get_rdate(p).time;
          if (icaltime_is_date(t)) {
            // RDATE with a DATE value inherits the (local) time from DTSTART
            t.hour    = dtstart.hour;
            t.minute  = dtstart.minute;
            t.second  = dtstart.second;
            t.is_date = 0;
            t.is_utc  = 0;    // dtstart is in local time
          }
          // RFC2445 states that RDATE must be in local time,
          // but we support UTC as well to be safe.
          if (!t.is_utc) {
            t.second -= prevOffset;    // convert to UTC
            t.is_utc = 1;
            t = icaltime_normalize(t);
          }
          times += toQDateTime(t);
kDebug()<<" .. RDATE: "<<times.last()<<(times.last().timeSpec()==Qt::UTC?" UTC":" Local")<<endl;
          break;
        }
        case ICAL_RRULE_PROPERTY:
        {
          RecurrenceRule r;
          ICalFormat icf;
          ICalFormatImpl impl(&icf);
          impl.readRecurrence(icalproperty_get_rrule(p), &r);
          r.setStartDt(localStart);
          DateTimeList dts = r.datesInInterval(utcStart, MAX_DATE());
          for ( int i = 0, end = dts.count();  i < end;  ++i) {
            QDateTime utc = dts[i];
	    utc.setTimeSpec(Qt::UTC);
	    times += utc.addSecs(-prevOffset);
kDebug()<<" .. RRULE: "<<times.last()<<endl;
          }
          break;
        }
        default:
          break;
      }
      p = icalcomponent_get_next_property(c, ICAL_ANY_PROPERTY);
    }
    qSortUnique(times);
  }

  return new KTimeZonePhase(times, utcOffset, abbrevs, daylight, comment);
}


}  // namespace KCal
