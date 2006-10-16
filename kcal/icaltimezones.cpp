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
#include <QVector>
#include <QSet>
#include <QFile>
#include <QTextStream>

#include <kdatetime.h>
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

static icaltimetype writeLocalICalDateTime( const QDateTime &utc, int offset )
{
  QDateTime local = utc.addSecs(offset);
  icaltimetype t = icaltime_null_time();
  t.year    = local.date().year();
  t.month   = local.date().month();
  t.day     = local.date().day();
  t.hour    = local.time().hour();
  t.minute  = local.time().minute();
  t.second  = local.time().second();
  t.is_date = 0;
  t.zone    = 0;
  t.is_utc  = 0;
  return t;
}


namespace KCal {


/******************************************************************************/

class ICalTimeZonesPrivate
{
public:
  ICalTimeZonesPrivate() : zones(new ICalTimeZones::ZoneMap())  {}
  ~ICalTimeZonesPrivate()  { clear();  delete zones; }
  void clear();
  static ICalTimeZone *utc();

  ICalTimeZones::ZoneMap *zones;
  QSet<ICalTimeZone*> nonconstZones;   // member zones owned by ICalTimeZones
};

void ICalTimeZonesPrivate::clear()
{
  // Delete all zones actually owned by this collection.
  for (ICalTimeZones::ZoneMap::ConstIterator it = zones->begin(), end = zones->end();  it != end;  ++it) {
    if (nonconstZones.contains(const_cast<ICalTimeZone*>(it.value())))   // only delete zones actually owned
      delete it.value();
  }
  zones->clear();
  nonconstZones.clear();
}

ICalTimeZone *ICalTimeZonesPrivate::utc()
{
  static ICalTimeZone *utcZone = 0;
  if ( !utcZone ) {
    ICalTimeZoneSource tzs;
    utcZone = tzs.parse( icaltimezone_get_utc_timezone() );
  }
  return utcZone;
}


ICalTimeZones::ICalTimeZones()
  : d(new ICalTimeZonesPrivate)
{
}

ICalTimeZones::~ICalTimeZones()
{
  delete d;
}

const ICalTimeZones::ZoneMap ICalTimeZones::zones() const
{
  return *d->zones;
}

bool ICalTimeZones::add(ICalTimeZone *zone)
{
  if (!zone)
    return false;
  if (d->zones->find(zone->name()) != d->zones->end())
    return false;    // name already exists
  d->zones->insert(zone->name(), zone);
  d->nonconstZones.insert(zone);
  return true;
}

bool ICalTimeZones::addConst(const ICalTimeZone *zone)
{
  if (!zone)
    return false;
  if (d->zones->find(zone->name()) != d->zones->end())
    return false;    // name already exists
  d->zones->insert(zone->name(), zone);
  return true;
}

const ICalTimeZone *ICalTimeZones::detach(const ICalTimeZone *zone)
{
  if (zone) {
    for (ZoneMap::Iterator it = d->zones->begin(), end = d->zones->end();  it != end;  ++it) {
      if (it.value() == zone) {
        d->zones->erase(it);
        d->nonconstZones.remove(const_cast<ICalTimeZone*>(zone));
        return (zone == utc()) ? 0 : zone;
      }
    }
  }
  return 0;
}

const ICalTimeZone *ICalTimeZones::detach(const QString &name)
{
  if (!name.isEmpty()) {
    ZoneMap::Iterator it = d->zones->find(name);
    if (it != d->zones->end()) {
      const ICalTimeZone *zone = it.value();
      d->zones->erase(it);
      d->nonconstZones.remove(const_cast<ICalTimeZone*>(zone));
      return (zone == utc()) ? 0 : zone;
    }
  }
  return 0;
}

void ICalTimeZones::clear()
{
  d->clear();
}

const ICalTimeZone *ICalTimeZones::zone(const QString &name) const
{
  if (!name.isEmpty()) {
    ZoneMap::ConstIterator it = d->zones->find(name);
    if (it != d->zones->end())
      return it.value();
  }
  return 0;    // error
}

const ICalTimeZone *ICalTimeZones::utc()
{
  return ICalTimeZonesPrivate::utc();
}


/******************************************************************************/

ICalTimeZone::ICalTimeZone(ICalTimeZoneSource *source, const QString &name, ICalTimeZoneData *data)
  : KTimeZone(source, name)
{
  setData(data);
}

ICalTimeZone::ICalTimeZone(const KTimeZone &tz)
  : KTimeZone(0, tz.name(), tz.countryCode(), tz.latitude(), tz.longitude(), tz.comment())
{
  if (tz.data(true))
    setData( new ICalTimeZoneData(*tz.data(), tz) );
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

bool ICalTimeZone::hasTransitions() const
{
    return true;
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
  d->icalComponent = icalcomponent_new_clone( rhs.d->icalComponent );
}

ICalTimeZoneData::ICalTimeZoneData(const KTimeZoneData &rhs, const KTimeZone &tz)
  : KTimeZoneData(rhs),
    d(new ICalTimeZoneDataPrivate())
{
  if (dynamic_cast<const KSystemTimeZone*>(&tz)) {
    icaltimezone *itz = icaltimezone_get_builtin_timezone( tz.name().toUtf8() );
    d->icalComponent = icalcomponent_new_clone( icaltimezone_get_component( itz ) );
  }
  else {
    // Write the time zone data into an iCal component
    icalcomponent *comp = icalcomponent_new(ICAL_VTIMEZONE_COMPONENT);
#warning tzid or location?
    icalcomponent_add_property(comp, icalproperty_new_tzid( tz.name().toUtf8() ));
//    icalcomponent_add_property(comp, icalproperty_new_tzurl( ??.toUtf8() ));
//    icalcomponent_add_property(comp, icalproperty_new_location( ??.toUtf8() ));

    // Compile an ordered list of transitions so that we can know the phases
    // which occur before and after each transition.
    const QList<KTimeZone::Transition> transits = transitions();
    QVector<bool> transitionsDone(transits.count());
    transitionsDone.fill(false);

    // Go through the list of transitions and create an iCal component for each
    // distinct combination of phase after and UTC offset before the transition.
    bool found;
    icaldatetimeperiodtype dtperiod;
    dtperiod.period = icalperiodtype_null_period();
    do {
      found = false;
      for (int i = 0, end = transits.count();  i < end;  ++i) {
        if ( transitionsDone[i] )
          continue;
        found = true;
        int preOffset = (i > 0) ? transits[i-1].phase().utcOffset() : rhs.previousUtcOffset();
        KTimeZone::Phase phase = transits[i].phase();
        icalcomponent *phcomp = icalcomponent_new( phase.isDst() ?
                                   ICAL_XDAYLIGHT_COMPONENT : ICAL_XSTANDARD_COMPONENT );
        QList<QByteArray> abbrevs = phase.abbreviations();
        for (int a = 0, aend = abbrevs.count();  a < aend;  ++a) {
          icalcomponent_add_property(phcomp, icalproperty_new_tzname( static_cast<const char*>(abbrevs[a]) ));
        }
        if ( !phase.comment().isEmpty() )
          icalcomponent_add_property(phcomp, icalproperty_new_comment( phase.comment().toUtf8() ));
        icalcomponent_add_property(phcomp, icalproperty_new_tzoffsetfrom( preOffset ));
        icalcomponent_add_property(phcomp, icalproperty_new_tzoffsetto( phase.utcOffset() ));
        icalcomponent_add_property(phcomp, icalproperty_new_dtstart(
                          writeLocalICalDateTime( transits[i].time(), preOffset ) ));
        transitionsDone[i] = true;

        while (++i < end) {
          if (!transitionsDone[i] 
          &&  transits[i].phase() == phase
          &&  transits[i-1].phase().utcOffset() == preOffset) {
            dtperiod.time = writeLocalICalDateTime( transits[i].time(), preOffset );
            icalcomponent_add_property(phcomp, icalproperty_new_rdate( dtperiod ));
            transitionsDone[i] = true;
          }
        }
        icalcomponent_add_component(comp, phcomp);
      }
    } while (found);

    d->icalComponent = comp;
  }
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
  icalcomponent *c = icalcomponent_new_clone( d->icalComponent );
  if ( !icaltimezone_set_component( icaltz, c ) ) {
    icalcomponent_free( c );
    icaltimezone_free( icaltz, 1 );
    return 0;
  }
  return icaltz;
}

bool ICalTimeZoneData::hasTransitions() const
{
    return true;
}


/******************************************************************************/

class ICalTimeZoneSourcePrivate
{
  public:
    static QList<QDateTime> parsePhase(icalcomponent*, bool daylight, int &prevOffset, KTimeZone::Phase&);
};


ICalTimeZoneSource::ICalTimeZoneSource()
{
}

ICalTimeZoneSource::~ICalTimeZoneSource()
{
}

bool ICalTimeZoneSource::parse(const QString &fileName, ICalTimeZones &zones)
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

bool ICalTimeZoneSource::parse(icalcomponent *calendar, ICalTimeZones &zones)
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
#warning Should location be used for name rather than tzid?
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
  //kDebug(5800) << "---zoneId: \"" << name << '"' << endl;

  /*
   * Iterate through all time zone rules for this VTIMEZONE,
   * and create a Phase object containing details for each one.
   */
  int prevOffset = 0;
  QList<KTimeZone::Transition> transitions;
  QDateTime earliest;
  QList<KTimeZone::Phase> phases;
  for (icalcomponent *c = icalcomponent_get_first_component(vtimezone, ICAL_ANY_COMPONENT);
       c;  c = icalcomponent_get_next_component(vtimezone, ICAL_ANY_COMPONENT))
  {
    int prevoff;
    KTimeZone::Phase phase;
    QList<QDateTime> times;
    icalcomponent_kind kind = icalcomponent_isa(c);
    switch (kind) {

      case ICAL_XSTANDARD_COMPONENT:
        //kDebug(5800) << "---standard phase: found" << endl;
        times = ICalTimeZoneSourcePrivate::parsePhase(c, false, prevoff, phase);
        break;

      case ICAL_XDAYLIGHT_COMPONENT:
        //kDebug(5800) << "---daylight phase: found" << endl;
        times = ICalTimeZoneSourcePrivate::parsePhase(c, true, prevoff, phase);
        break;

      default:
        kDebug(5800) << "ICalTimeZoneSource::parse(): Unknown component: " << kind << endl;
        break;
    }
    int tcount = times.count();
    if (tcount) {
      phases += phase;
      for (int t = 0;  t < tcount;  ++t)
        transitions += KTimeZone::Transition(times[t], phase);
      if (!earliest.isValid()  ||  times[0] < earliest) {
        prevOffset = prevoff;
        earliest = times[0];
      }
    }
  }
  data->setPhases(phases, prevOffset);
  // Remove any "duplicate" transitions, i.e. those where two consecutive
  // transitions have the same phase.
  qSort(transitions);
  for (int t = 1, tend = transitions.count();  t < tend; ) {
    if (transitions[t].phase() == transitions[t-1].phase()) {
      transitions.removeAt(t);
      --tend;
    } else {
      ++t;
    }
  }
  data->setTransitions(transitions);

  data->d->setComponent( icalcomponent_new_clone(vtimezone) );
  kDebug(5800) << "ICalTimeZoneSource::parse(): VTIMEZONE " << name << endl;
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

QList<QDateTime> ICalTimeZoneSourcePrivate::parsePhase(icalcomponent *c, bool daylight, int &prevOffset,
                                      KTimeZone::Phase &phase)
{
  QList<QDateTime> transitions;

  // Read the observance data for this standard/daylight savings phase
  QList<QByteArray> abbrevs;
  QString comment;
  prevOffset = 0;
  int  utcOffset = 0;
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
        if (!abbrevs.contains(tzname))
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
    return transitions;
  }

  // Convert DTSTART to QDateTime, and from local time to UTC
  QDateTime localStart = toQDateTime(dtstart);   // local time
  dtstart.second -= prevOffset;
  dtstart.is_utc = 1;
  QDateTime utcStart = toQDateTime(icaltime_normalize(dtstart));   // UTC

  transitions += utcStart;
  if (recurs) {
    /* RDATE or RRULE is specified. There should only be one or the other, but
     * it doesn't really matter - the code can cope with both.
     * Note that we had to get DTSTART, TZOFFSETFROM, TZOFFSETTO before reading
     * recurrences.
     */
    KDateTime klocalStart(localStart, KDateTime::Spec::ClockTime);
    KDateTime maxTime(MAX_DATE(), KDateTime::Spec::ClockTime);
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
          transitions += toQDateTime(t);
          break;
        }
        case ICAL_RRULE_PROPERTY:
        {
          RecurrenceRule r;
          ICalFormat icf;
          ICalFormatImpl impl(&icf);
          impl.readRecurrence(icalproperty_get_rrule(p), &r);
          r.setStartDt(klocalStart);
          // The end date time specified in an RRULE should be in UTC.
          // Convert to local time to avoid timesInInterval() getting things wrong.
          if (r.duration() == 0) {
            KDateTime end(r.endDt());
            if (end.timeSpec() == KDateTime::Spec::UTC) {
              end.setTimeSpec(KDateTime::Spec::ClockTime);
              r.setEndDt( end.addSecs(prevOffset) );
            }
          }
          DateTimeList dts = r.timesInInterval(klocalStart, maxTime);
          for ( int i = 0, end = dts.count();  i < end;  ++i) {
            QDateTime utc = dts[i].dateTime();
            utc.setTimeSpec(Qt::UTC);
            transitions += utc.addSecs(-prevOffset);
          }
          break;
        }
        default:
          break;
      }
      p = icalcomponent_get_next_property(c, ICAL_ANY_PROPERTY);
    }
    qSortUnique(transitions);
  }

  phase = KTimeZone::Phase(utcOffset, abbrevs, daylight, comment);
  return transitions;
}


}  // namespace KCal
