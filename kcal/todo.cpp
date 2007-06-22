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
  defines the Todo class.

  @authors Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "todo.h"

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::Todo::Private
{
  public:
    bool mHasStartDate;      // true if the to-do has a starting date
    KDateTime mDtDue;        // to-do due date (if there is one)
                             // ALSO the first occurrence of a recurring to-do
    bool mHasDueDate;        // true if the to-do has a due date
    KDateTime mDtRecurrence; // next occurrence (for recurring to-dos)

    int mPercentComplete;    // to-do percent complete [0,100]
    bool mHasCompletedDate;  // true if the to-do has a completion date
    KDateTime mCompleted;    // to-do completion date (if it has been completed)
};
//@endcond

Todo::Todo() : d( new KCal::Todo::Private )
{
  d->mHasDueDate = false;
  d->mHasStartDate = false;
  d->mHasCompletedDate = false;
  d->mPercentComplete = 0;
}

Todo::Todo( const Todo &t ) : Incidence( t ), d( new KCal::Todo::Private )
{
  d->mDtDue = t.d->mDtDue;
  d->mHasDueDate = t.d->mHasDueDate;
  d->mHasStartDate = t.d->mHasStartDate;
  d->mCompleted = t.d->mCompleted;
  d->mHasCompletedDate = t.d->mHasCompletedDate;
  d->mPercentComplete = t.d->mPercentComplete;
  d->mDtRecurrence = t.d->mDtRecurrence;
}

Todo::~Todo()
{
  delete d;
}

Todo *Todo::clone()
{
  return new Todo( *this );
}

bool Todo::operator==( const Todo &t2 ) const
{
  return
    static_cast<const Incidence&>(*this) == static_cast<const Incidence&>(t2) &&
    dtDue() == t2.dtDue() &&
    hasDueDate() == t2.hasDueDate() &&
    hasStartDate() == t2.hasStartDate() &&
    completed() == t2.completed() &&
    hasCompletedDate() == t2.hasCompletedDate() &&
    percentComplete() == t2.percentComplete();
}

void Todo::setDtDue( const KDateTime &dtDue, bool first )
{
  //int diffsecs = d->mDtDue.secsTo(dtDue);

  /*if (mReadOnly) return;
  const Alarm::List& alarms = alarms();
  for (Alarm* alarm = alarms.first(); alarm; alarm = alarms.next()) {
    if (alarm->enabled()) {
      alarm->setTime(alarm->time().addSecs(diffsecs));
    }
  }*/
  if ( doesRecur() && !first ) {
    d->mDtRecurrence = dtDue;
  } else {
    d->mDtDue = dtDue;
    // TODO: This doesn't seem right...
    recurrence()->setStartDateTime( dtDue );
    recurrence()->setFloats( floats() );
  }

  if ( doesRecur() && dtDue < recurrence()->startDateTime() ) {
    setDtStart( dtDue );
  }

  //kDebug(5800) << "setDtDue says date is " << d->mDtDue.toString() << endl;

  /*const Alarm::List& alarms = alarms();
  for (Alarm* alarm = alarms.first(); alarm; alarm = alarms.next())
    alarm->setAlarmStart(d->mDtDue);*/

  updated();
}

KDateTime Todo::dtDue( bool first ) const
{
  if ( doesRecur() && !first && d->mDtRecurrence.isValid() ) {
    return d->mDtRecurrence;
  }

  return d->mDtDue;
}

QString Todo::dtDueTimeStr( bool shortfmt ) const
{
  return KGlobal::locale()->formatTime( dtDue( !doesRecur() ).time(), shortfmt );
}

QString Todo::dtDueDateStr( bool shortfmt ) const
{
  return
    KGlobal::locale()->formatDate( dtDue( !doesRecur() ).date(),
                                  ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
}

QString Todo::dtDueStr( bool shortfmt ) const
{
  return
    KGlobal::locale()->formatDateTime( dtDue( !doesRecur() ).dateTime(),
                                       ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
}

bool Todo::hasDueDate() const
{
  return d->mHasDueDate;
}

void Todo::setHasDueDate( bool f )
{
  if ( mReadOnly ) {
    return;
  }
  d->mHasDueDate = f;
  updated();
}

bool Todo::hasStartDate() const
{
  return d->mHasStartDate;
}

void Todo::setHasStartDate( bool f )
{
  if ( mReadOnly ) {
    return;
  }

  if ( doesRecur() && !f ) {
    if ( !comments().filter( "NoStartDate" ).count() ) {
      addComment( "NoStartDate" ); //TODO: --> custom flag?
    }
  } else {
    QString s( "NoStartDate" );
    removeComment( s );
  }
  d->mHasStartDate = f;
  updated();
}

KDateTime Todo::dtStart( bool first ) const
{
  if ( doesRecur() && !first ) {
    return d->mDtRecurrence.addDays( dtDue( first ).daysTo( IncidenceBase::dtStart() ) );
  } else {
    return IncidenceBase::dtStart();
  }
}

void Todo::setDtStart( const KDateTime &dtStart )
{
  // TODO: This doesn't seem right (rfc 2445/6 says, recurrence is calculated from the dtstart...)
  if ( doesRecur() ) {
    recurrence()->setStartDateTime( d->mDtDue );
    recurrence()->setFloats( floats() );
  }
  IncidenceBase::setDtStart( dtStart );
}

QString Todo::dtStartTimeStr( bool shortfmt, bool first ) const
{
  return KGlobal::locale()->formatTime( dtStart( first ).time(), shortfmt );
}

QString Todo::dtStartDateStr( bool shortfmt, bool first ) const
{
  return
    KGlobal::locale()->formatDate( dtStart( first ).date(),
                                   ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
}

QString Todo::dtStartStr( bool shortfmt, bool first ) const
{
  return
    KGlobal::locale()->formatDateTime( dtStart( first ).dateTime(),
                                       ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
}

bool Todo::isCompleted() const
{
  if ( d->mPercentComplete == 100 ) {
    return true;
  } else {
    return false;
  }
}

void Todo::setCompleted( bool completed )
{
  if ( completed ) {
    d->mPercentComplete = 100;
  } else {
    d->mPercentComplete = 0;
    d->mHasCompletedDate = false;
    d->mCompleted = KDateTime();
  }
  updated();
}

KDateTime Todo::completed() const
{
  if ( hasCompletedDate() ) {
    return d->mCompleted;
  } else {
    return KDateTime();
  }
}

QString Todo::completedStr( bool shortfmt ) const
{
  return
    KGlobal::locale()->formatDateTime( d->mCompleted.dateTime(),
                                       ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
}

void Todo::setCompleted( const KDateTime &completed )
{
  if ( !recurTodo() ) {
    d->mHasCompletedDate = true;
    d->mPercentComplete = 100;
    d->mCompleted = completed.toUtc();
  }
  updated();
}

bool Todo::hasCompletedDate() const
{
  return d->mHasCompletedDate;
}

int Todo::percentComplete() const
{
  return d->mPercentComplete;
}

void Todo::setPercentComplete( int percent )
{
  //TODO: (?) assert percent between 0 and 100, inclusive
  d->mPercentComplete = percent;
  if ( percent != 100 ) {
    d->mHasCompletedDate = false;
  }
  updated();
}

void Todo::shiftTimes( const KDateTime::Spec &oldSpec,
                       const KDateTime::Spec &newSpec )
{
  Incidence::shiftTimes( oldSpec, newSpec );
  d->mDtDue = d->mDtDue.toTimeSpec( oldSpec );
  d->mDtDue.setTimeSpec( newSpec );
  if ( doesRecur() ) {
    d->mDtRecurrence = d->mDtRecurrence.toTimeSpec( oldSpec );
    d->mDtRecurrence.setTimeSpec( newSpec );
  }
  if ( d->mHasCompletedDate ) {
    d->mCompleted = d->mCompleted.toTimeSpec( oldSpec );
    d->mCompleted.setTimeSpec( newSpec );
  }
}

void Todo::setDtRecurrence( const KDateTime &dt )
{
  d->mDtRecurrence = dt;
}

KDateTime Todo::dtRecurrence() const
{
  return d->mDtRecurrence.isValid() ? d->mDtRecurrence : d->mDtDue;
}

bool Todo::recursOn( const QDate &date, const KDateTime::Spec &timeSpec ) const
{
  QDate today = QDate::currentDate();
  return
    Incidence::recursOn( date, timeSpec ) &&
    !( date < today && d->mDtRecurrence.date() < today &&
       d->mDtRecurrence > recurrence()->startDateTime() );
}

bool Todo::recurTodo()
{
  if ( doesRecur() ) {
    Recurrence *r = recurrence();
    KDateTime endDateTime = r->endDateTime();
    KDateTime nextDate = r->getNextDateTime( dtDue() );

    if ( ( r->duration() == -1 ||
           ( nextDate.isValid() && endDateTime.isValid() &&
             nextDate <= endDateTime ) ) ) {

      while ( !recursAt( nextDate ) ||
              nextDate <= KDateTime::currentUtcDateTime() ) {

        if ( !nextDate.isValid() || nextDate > endDateTime ) {
          return false;
        }

        nextDate = r->getNextDateTime( nextDate );
      }

      setDtDue( nextDate );
      setCompleted( false );
      setRevision( revision() + 1 );

      return true;
    }
  }

  return false;
}

bool Todo::isOverdue() const
{
  bool inPast = floats() ?
                dtDue().date() < QDate::currentDate() :
                dtDue() < KDateTime::currentUtcDateTime();
  return inPast && !isCompleted();
}

// DEPRECATED methods
void Todo::setDtDue( const QDateTime &dtDue, bool first )
{
  if ( dtStart().isValid() ) {
    setDtDue( KDateTime( dtDue, dtStart().timeSpec() ), first );
  } else {
    setDtDue( KDateTime( dtDue ), first );  // use local time zone
  }
}
void Todo::setCompleted( const QDateTime &completed )
{
  if ( dtStart().isValid() ) {
    setCompleted( KDateTime( completed, dtStart().timeSpec() ) );
  } else {
    setCompleted( KDateTime( completed ) );  // use local time zone
  }
}
