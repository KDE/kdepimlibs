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

  @brief
  Provides a To-do in the sense of RFC2445.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "todo.h"

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <ksystemtimezone.h>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::Todo::Private
{
  public:
    Private()
      : mPercentComplete( 0 ),
        mHasDueDate( false ),
        mHasStartDate( false ),
        mHasCompletedDate( false )
    {}
    Private( const KCal::Todo::Private &other )
    { init( other ); }

    void init( const KCal::Todo::Private &other );

    KDateTime mDtDue;        // to-do due date (if there is one)
                             // ALSO the first occurrence of a recurring to-do
    KDateTime mDtRecurrence; // next occurrence (for recurring to-dos)
    KDateTime mCompleted;    // to-do completion date (if it has been completed)
    int mPercentComplete;    // to-do percent complete [0,100]
    bool mHasDueDate;        // true if the to-do has a due date
    bool mHasStartDate;      // true if the to-do has a starting date
    bool mHasCompletedDate;  // true if the to-do has a completion date

    /**
      Returns true if the todo got a new date, else false will be returned.
    */
    bool recurTodo( Todo *todo );
};

void KCal::Todo::Private::init( const KCal::Todo::Private &other )
{
  mDtDue = other.mDtDue;
  mDtRecurrence = other.mDtRecurrence;
  mCompleted = other.mCompleted;
  mPercentComplete = other.mPercentComplete;
  mHasDueDate = other.mHasDueDate;
  mHasStartDate = other.mHasStartDate;
  mHasCompletedDate = other.mHasCompletedDate;
}

//@endcond

Todo::Todo()
  : d( new KCal::Todo::Private )
{
}

Todo::Todo( const Todo &other )
  : Incidence( other ),
    d( new KCal::Todo::Private( *other.d ) )
{
}

Todo::~Todo()
{
  delete d;
}

Todo *Todo::clone()
{
  return new Todo( *this );
}

Todo &Todo::operator=( const Todo &other )
{
  Incidence::operator=( other );
  d->init( *other.d );
  return *this;
}

bool Todo::operator==( const Todo &todo ) const
{
  return
    static_cast<const Incidence &>( *this ) == static_cast<const Incidence &>( todo ) &&
    dtDue() == todo.dtDue() &&
    hasDueDate() == todo.hasDueDate() &&
    hasStartDate() == todo.hasStartDate() &&
    completed() == todo.completed() &&
    hasCompletedDate() == todo.hasCompletedDate() &&
    percentComplete() == todo.percentComplete();
}

QByteArray Todo::type() const
{
  return "Todo";
}

void Todo::setDtDue( const KDateTime &dtDue, bool first )
{
  //int diffsecs = d->mDtDue.secsTo(dtDue);

  /*if (mReadOnly) return;
  const Alarm::List& alarms = alarms();
  for (Alarm *alarm = alarms.first(); alarm; alarm = alarms.next()) {
    if (alarm->enabled()) {
      alarm->setTime(alarm->time().addSecs(diffsecs));
    }
  }*/
  if ( recurs() && !first ) {
    d->mDtRecurrence = dtDue;
  } else {
    d->mDtDue = dtDue;
    // TODO: This doesn't seem right...
    recurrence()->setStartDateTime( dtDue );
    recurrence()->setAllDay( allDay() );
  }

  if ( recurs() && dtDue < recurrence()->startDateTime() ) {
    setDtStart( dtDue );
  }

  //kDebug() << "setDtDue says date is" << d->mDtDue.toString();

  /*const Alarm::List& alarms = alarms();
  for (Alarm *alarm = alarms.first(); alarm; alarm = alarms.next())
    alarm->setAlarmStart(d->mDtDue);*/

  updated();
}

KDateTime Todo::dtDue( bool first ) const
{
  if ( !hasDueDate() ) {
    return KDateTime();
  }
  if ( recurs() && !first && d->mDtRecurrence.isValid() ) {
    return d->mDtRecurrence;
  }

  return d->mDtDue;
}

QString Todo::dtDueTimeStr( bool shortfmt, const KDateTime::Spec &spec ) const
{
  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return KGlobal::locale()->formatTime( dtDue( !recurs() ).toTimeSpec( spec ).time(), !shortfmt )
      + timeZone;
  } else {
    return KGlobal::locale()->formatTime( dtDue( !recurs() ).time(), !shortfmt );
  }
}

QString Todo::dtDueDateStr( bool shortfmt, const KDateTime::Spec &spec ) const
{
  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return KGlobal::locale()->formatDate(
      dtDue( !recurs() ).toTimeSpec( spec ).date(),
      ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) )
      + timeZone;
  } else {
    return KGlobal::locale()->formatDate(
      dtDue( !recurs() ).date(),
      ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
  }
}

QString Todo::dtDueStr( bool shortfmt, const KDateTime::Spec &spec ) const
{
  if ( allDay() ) {
    return dtDueDateStr( shortfmt, spec );
  }

  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return KGlobal::locale()->formatDateTime(
      dtDue( !recurs() ).toTimeSpec( spec ).dateTime(),
      ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) )
      + timeZone;
  } else {
    return  KGlobal::locale()->formatDateTime(
      dtDue( !recurs() ).dateTime(),
      ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
  }
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

  if ( recurs() && !f ) {
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

KDateTime Todo::dtStart() const
{
  return dtStart( false );
}

KDateTime Todo::dtStart( bool first ) const
{
  if ( !hasStartDate() ) {
    return KDateTime();
  }
  if ( recurs() && !first ) {
    return d->mDtRecurrence.addDays( dtDue( first ).daysTo( IncidenceBase::dtStart() ) );
  } else {
    return IncidenceBase::dtStart();
  }
}

void Todo::setDtStart( const KDateTime &dtStart )
{
  // TODO: This doesn't seem right (rfc 2445/6 says, recurrence is calculated from the dtstart...)
  if ( recurs() ) {
    recurrence()->setStartDateTime( d->mDtDue );
    recurrence()->setAllDay( allDay() );
  }
  IncidenceBase::setDtStart( dtStart );
}

QString Todo::dtStartTimeStr( bool shortfmt, bool first, const KDateTime::Spec &spec ) const
{
  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return KGlobal::locale()->formatTime( dtStart( first ).toTimeSpec( spec ).time(), !shortfmt )
      + timeZone;
  } else {
    return KGlobal::locale()->formatTime( dtStart( first ).time(), !shortfmt );
  }
}

QString Todo::dtStartTimeStr( bool shortfmt, const KDateTime::Spec &spec ) const
{
  return Incidence::dtStartTimeStr( shortfmt, spec );
}

QString Todo::dtStartDateStr( bool shortfmt, bool first, const KDateTime::Spec &spec ) const
{
  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return
      KGlobal::locale()->formatDate( dtStart( first ).toTimeSpec( spec ).date(),
                                   ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) )
      + timeZone;
  } else {
    return
      KGlobal::locale()->formatDate( dtStart( first ).date(),
                                   ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
  }
}

QString Todo::dtStartDateStr( bool shortfmt, const KDateTime::Spec &spec ) const
{
  return Incidence::dtStartDateStr( shortfmt, spec );
}

QString Todo::dtStartStr( bool shortfmt, bool first, const KDateTime::Spec &spec ) const
{
  if ( allDay() ) {
    return dtStartDateStr( shortfmt, spec );
  }

  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return
      KGlobal::locale()->formatDateTime( dtStart( first ).toTimeSpec( spec ).dateTime(),
                                       ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) )
      + timeZone;
  } else {
    return
      KGlobal::locale()->formatDateTime( dtStart( first ).dateTime(),
                                       ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
  }
}

QString Todo::dtStartStr( bool shortfmt, const KDateTime::Spec &spec ) const
{
  return Incidence::dtStartStr( shortfmt, spec );
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
  if ( !d->recurTodo( this ) ) {
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
  if ( recurs() ) {
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

bool Todo::isOverdue() const
{
  if ( !dtDue().isValid() ) {
    return false; // if it's never due, it can't be overdue
  }

  bool inPast = allDay() ?
                dtDue().date() < QDate::currentDate() :
                dtDue() < KDateTime::currentUtcDateTime();
  return inPast && !isCompleted();
}

KDateTime Todo::endDateRecurrenceBase() const
{
  return dtDue();
}

//@cond PRIVATE
bool Todo::Private::recurTodo( Todo *todo )
{
  if ( todo->recurs() ) {
    Recurrence *r = todo->recurrence();
    KDateTime endDateTime = r->endDateTime();
    KDateTime nextDate = r->getNextDateTime( todo->dtDue() );

    if ( ( r->duration() == -1 ||
           ( nextDate.isValid() && endDateTime.isValid() &&
             nextDate <= endDateTime ) ) ) {

      while ( !todo->recursAt( nextDate ) ||
              nextDate <= KDateTime::currentUtcDateTime() ) {

        if ( !nextDate.isValid() ||
	        ( nextDate > endDateTime && r->duration() != -1 ) ) {

          return false;
        }

        nextDate = r->getNextDateTime( nextDate );
      }

      todo->setDtDue( nextDate );
      todo->setCompleted( false );
      todo->setRevision( todo->revision() + 1 );

      return true;
    }
  }

  return false;
}
//@endcond
