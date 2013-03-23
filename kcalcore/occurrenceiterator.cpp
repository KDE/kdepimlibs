/*
  This file is part of the kcalcore library.

  Copyright (C) 2013 Christian Mollekopf <mollekopf@kolabsys.com>

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
  defines the OccurrenceIterator class.

  @brief
  This class provides an iterator to iterate over all occurrences of incidences.

  @author Christian Mollekopf \<mollekopf@kolabsys.com\>
 */

#include "occurrenceiterator.h"
#include "calendar.h"

#include <KDebug>
#include <QDate>

using namespace KCalCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCalCore::OccurrenceIterator::Private
{
  public:
    Private( OccurrenceIterator *qq )
      : q( qq ),
        occurrenceIt(occurrenceList)
    {
    }

    OccurrenceIterator *q;
    KDateTime start;
    KDateTime end;

    struct Occurrence {
      Occurrence(){};
      Occurrence(const Incidence::Ptr &i, const KDateTime &d): incidence(i), date(d) {}
      Incidence::Ptr incidence;
      KDateTime date;
    };
    QList<Occurrence> occurrenceList;
    QListIterator<Occurrence> occurrenceIt;
    Occurrence current;

    void setupIterator(const Calendar &calendar, const Incidence::List &incidences) {
      foreach (const Incidence::Ptr &inc, incidences) {
        if (inc->hasRecurrenceId()) {
          continue;
        }
        if (inc->recurs()) {
          QHash<KDateTime, Incidence::Ptr> recurrenceIds;
          foreach (const Incidence::Ptr &exception, calendar.instances(inc)) {
            recurrenceIds.insert(exception->recurrenceId(), exception);
          }
          const DateTimeList occurrences = inc->recurrence()->timesInInterval(start, end);
          foreach (const KDateTime &occ, occurrences) {
            if (recurrenceIds.contains(occ)) {
            //TODO exclude exceptions where the start/end is not within (so the occurrence of the recurrence is omitted, but no exception is added
              const KCalCore::Incidence::Ptr &exception = recurrenceIds.value(occ);
              occurrenceList.append(Private::Occurrence(exception, exception->dtStart()));
            } else {
              occurrenceList.append(Private::Occurrence(inc, occ));
            }
          }
        } else {
          //TODO deal with non recurring events?
        }
      }
      occurrenceIt = QListIterator<Private::Occurrence>(occurrenceList);
    }
};
//@endcond

static uint qHash(const KDateTime &dt)
{
  return qHash(dt.toString());
}

static Incidence::List toIncidences(const Event::List &events)
{
  Incidence::List incidences;
  foreach (const Event::Ptr &event, events) {
      incidences << event;
  }
  return incidences;
}

/**
 * Right now there is little point in the iterator, but:
 * With an iterator it should be possible to solve this more memory efficiently and with immediate results at the beginning of the selected timeframe.
 * Either all events are iterated simoulatneously, resulting in occurrences of all events in parallel in the correct time-order, or incidence after incidence, which would be even more efficient.
 *
 * By making this class a friend of calendar, we could also use the internally available data structures.
 */
OccurrenceIterator::OccurrenceIterator( const Calendar &calendar, const KDateTime &start, const KDateTime &end )
  : d( new KCalCore::OccurrenceIterator::Private( this ) )
{
  d->start = start;
  d->end = end;
  //FIXME only takes events into account
  const Incidence::List incidences = toIncidences(calendar.rawEvents(start.date(), end.date(), start.timeSpec()));
  d->setupIterator(calendar, incidences);
}

OccurrenceIterator::OccurrenceIterator(const Calendar& calendar, const Incidence::Ptr& incidence, const KDateTime& start, const KDateTime& end)
  : d( new KCalCore::OccurrenceIterator::Private( this ) )
{
  d->start = start;
  d->end = end;
  d->setupIterator(calendar, Incidence::List() << incidence);
}

OccurrenceIterator::~OccurrenceIterator()
{

}

bool OccurrenceIterator::hasNext() const
{
  return d->occurrenceIt.hasNext();
}

void OccurrenceIterator::next()
{
  d->current = d->occurrenceIt.next();
}

Incidence::Ptr OccurrenceIterator::incidence() const
{
  return d->current.incidence;
}

KDateTime OccurrenceIterator::occurrenceStartDate() const
{
  return d->current.date;
}
