/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002,2006 David Jarvie <software@astrojar.org.uk>
    Copyright (C) 2005 Reinhold Kainhofer <kainhofer@kde.org>

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

#include <limits.h>

#include <QtCore/QList>
#include <QtCore/QBitArray>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "recurrence.h"
#include "recurrencerule.h"

using namespace KCal;


Recurrence::Recurrence()
: mFloating( false ),
  mRecurReadOnly( false ),
  mCachedType( rMax )
{
  mExRules.setAutoDelete( true );
  mRRules.setAutoDelete( true );
}

Recurrence::Recurrence( const Recurrence &r )
: RecurrenceRule::RuleObserver(),
  mRDateTimes( r.mRDateTimes ),
  mRDates( r.mRDates ),
  mExDateTimes( r.mExDateTimes ),
  mExDates( r.mExDates ),
  mStartDateTime( r.mStartDateTime ),
  mFloating( r.mFloating ),
  mRecurReadOnly(r.mRecurReadOnly),
  mCachedType( r.mCachedType )
{
  mExRules.setAutoDelete( true );
  mRRules.setAutoDelete( true );
  int i, end;
  for ( i = 0, end = r.mRRules.count();  i < end;  ++i ) {
    RecurrenceRule *rule = new RecurrenceRule( *r.mRRules[i] );
    mRRules.append( rule );
    rule->addObserver( this );
  }
  for ( i = 0, end = r.mExRules.count();  i < end;  ++i ) {
    RecurrenceRule *rule = new RecurrenceRule( *r.mExRules[i] );
    mExRules.append( rule );
    rule->addObserver( this );
  }
}

Recurrence::~Recurrence()
{
}



bool Recurrence::operator==( const Recurrence& r2 ) const
{
  if ( mStartDateTime != r2.mStartDateTime
  ||   mFloating != r2.mFloating
  ||   mRecurReadOnly != r2.mRecurReadOnly )
    return false;
  if ( mExDates != r2.mExDates ) return false;
  if ( mExDateTimes != r2.mExDateTimes ) return false;
  if ( mRDates != r2.mRDates ) return false;
  if ( mRDateTimes != r2.mRDateTimes ) return false;

// Compare the rrules, exrules! Assume they have the same order... This only
// matters if we have more than one rule (which shouldn't be the default anyway)
  int i;
  int end = mRRules.count();
  if ( end != r2.mRRules.count() ) return false;
  for ( i = 0;  i < end;  ++i ) {
    if ( *mRRules[i] != *r2.mRRules[i] )  return false;
  }
  end = mExRules.count();
  if ( end != r2.mExRules.count() )  return false;
  for ( i = 0;  i < end;  ++i ) {
    if ( *mExRules[i] != *r2.mExRules[i] )  return false;
  }
  return true;
}

void Recurrence::addObserver( RecurrenceObserver *observer )
{
  if ( !mObservers.contains( observer ) )
    mObservers.append( observer );
}

void Recurrence::removeObserver( RecurrenceObserver *observer )
{
  if ( mObservers.contains( observer ) )
    mObservers.removeAll( observer );
}


KDateTime Recurrence::startDateTime() const
{
  return mStartDateTime;
}

void Recurrence::setFloats( bool floats )
{
  if ( mRecurReadOnly ) return;
  if ( floats == mFloating ) return;
  mFloating = floats;

  for ( int i = 0, end = mRRules.count();  i < end;  ++i ) {
    mRRules[i]->setFloats( floats );
  }
  for ( int i = 0, end = mExRules.count();  i < end;  ++i ) {
    mExRules[i]->setFloats( floats );
  }
  updated();
}

RecurrenceRule *Recurrence::defaultRRule( bool create ) const
{
  if ( mRRules.isEmpty() ) {
    if ( !create || mRecurReadOnly ) return 0;
    RecurrenceRule *rrule = new RecurrenceRule();
    rrule->setStartDt( startDateTime() );
    const_cast<KCal::Recurrence*>(this)->addRRule( rrule );
    return rrule;
  } else {
    return mRRules[0];
  }
}

RecurrenceRule *Recurrence::defaultRRuleConst() const
{
  if ( mRRules.isEmpty() ) {
    return 0;
  } else {
    return mRRules[0];
  }
}

void Recurrence::updated()
{
  // recurrenceType() re-calculates the type if it's rMax
  mCachedType = rMax;
  for ( int i = 0, end = mObservers.count();  i < end;  ++i ) {
    if ( mObservers[i] )
      mObservers[i]->recurrenceUpdated( this );
  }
}

bool Recurrence::doesRecur() const
{
  return !mRRules.isEmpty() || !mRDates.isEmpty() || !mRDateTimes.isEmpty();
}

ushort Recurrence::recurrenceType() const
{
  if ( mCachedType == rMax ) {
    mCachedType = recurrenceType( defaultRRuleConst() );
  }
  return mCachedType;
}

ushort Recurrence::recurrenceType( const RecurrenceRule *rrule )
{
  if ( !rrule ) return rNone;
  RecurrenceRule::PeriodType type = rrule->recurrenceType();

  // BYSETPOS, BYWEEKNUMBER and BYSECOND were not supported in old versions
  if ( !rrule->bySetPos().isEmpty() )
    return rOther;
  if ( !rrule->bySeconds().isEmpty() )
    return rOther;
  if ( !rrule->byWeekNumbers().isEmpty() )
    return rOther;

  // It wasn't possible to set BYMINUTES, BYHOUR etc. by the old code. So if
  // it's set, it's none of the old types
  if ( !rrule->byMinutes().isEmpty() )
    return rOther;
  if ( !rrule->byHours().isEmpty() )
    return rOther;

  // Possible combinations were:
  // BYDAY: with WEEKLY, MONTHLY, YEARLY
  // BYMONTHDAY: with MONTHLY, YEARLY
  // BYMONTH: with YEARLY
  // BYYEARDAY: with YEARLY
  if ( !rrule->byYearDays().isEmpty() && type != RecurrenceRule::rYearly )
    return rOther;
  if ( !rrule->byMonths().isEmpty() && type != RecurrenceRule::rYearly )
    return rOther;
  if ( !rrule->byDays().isEmpty() ) {
    if ( type != RecurrenceRule::rYearly && type != RecurrenceRule::rMonthly &&
         type != RecurrenceRule::rWeekly )
      return rOther;
  }

  switch ( type ) {
    case RecurrenceRule::rNone:     return rNone;
    case RecurrenceRule::rMinutely: return rMinutely;
    case RecurrenceRule::rHourly:   return rHourly;
    case RecurrenceRule::rDaily:    return rDaily;
    case RecurrenceRule::rWeekly:   return rWeekly;
    case RecurrenceRule::rMonthly: {
        if ( rrule->byDays().isEmpty() ) return rMonthlyDay;
        else if ( rrule->byMonthDays().isEmpty() ) return rMonthlyPos;
        else return rOther; // both position and date specified
      }
    case RecurrenceRule::rYearly: {
        // Possible combinations:
        //   rYearlyMonth: [BYMONTH &] BYMONTHDAY
        //   rYearlyDay: BYYEARDAY
        //   rYearlyPos: [BYMONTH &] BYDAY
        if ( !rrule->byDays().isEmpty() ) {
          // can only by rYearlyPos
          if ( rrule->byMonthDays().isEmpty() && rrule->byYearDays().isEmpty() )
            return rYearlyPos;
          else return rOther;
        } else if ( !rrule->byYearDays().isEmpty() ) {
          // Can only be rYearlyDay
          if ( rrule->byMonths().isEmpty() && rrule->byMonthDays().isEmpty() )
            return rYearlyDay;
          else return rOther;
        } else {
          return rYearlyMonth;
        }
        break;
      }
     default: return rOther;
  }
  return rOther;
}

bool Recurrence::recursOn(const QDate &qd, const KDateTime::Spec &timeSpec) const
{
  // Don't waste time if date is before the start of the recurrence
  if ( KDateTime( qd, QTime(23,59,59), timeSpec ) < mStartDateTime )
    return false;

  int i, end;
  TimeList tms;
  // First handle dates. Exrules override
  if ( mExDates.containsSorted( qd ) )
    return false;
  // For all-day events a matching exrule excludes the whole day
  // since exclusions take precedence over inclusions, we know it can't occur on that day.
  if ( doesFloat() ) {
    for ( i = 0, end = mExRules.count();  i < end;  ++i ) {
      if ( mExRules[i]->recursOn( qd, timeSpec ) )
        return false;
    }
  }

  if ( mRDates.containsSorted( qd ) )
    return true;

  // Check if it might recur today at all.
  bool recurs = ( startDate() == qd );
  for ( i = 0, end = mRDateTimes.count();  i < end && !recurs;  ++i ) {
    recurs = ( mRDateTimes[i].toTimeSpec( timeSpec ).date() == qd );
  }
  for ( i = 0, end = mRRules.count();  i < end && !recurs;  ++i ) {
    recurs = mRRules[i]->recursOn( qd, timeSpec );
  }
  // If the event wouldn't recur at all, simply return false, don't check ex*
  if ( !recurs )
    return false;

  // Check if there are any times for this day excluded, either by exdate or exrule:
  bool exon = false;
  for ( i = 0, end = mExDateTimes.count();  i < end && !exon;  ++i ) {
    exon = ( mExDateTimes[i].toTimeSpec( timeSpec ).date() == qd );
  }
  if ( !doesFloat() ) {     // we have already checked floating times above
    for ( i = 0, end = mExRules.count();  i < end && !exon;  ++i ) {
      exon = mExRules[i]->recursOn( qd, timeSpec );
    }
  }

  if ( !exon ) {
    // Simple case, nothing on that day excluded, return the value from before
    return recurs;
  } else {
    // Harder part: I don't think there is any way other than to calculate the
    // whole list of items for that day.
//TODO: consider whether it would be more efficient to call
//      Rule::recurTimesOn() instead of Rule::recursOn() from the start
    TimeList timesForDay( recurTimesOn( qd, timeSpec ) );
    return !timesForDay.isEmpty();
  }
}

bool Recurrence::recursAt( const KDateTime &dt ) const
{
  // Convert to recurrence's time zone for date comparisons, and for more efficient time comparisons
  KDateTime dtrecur = dt.toTimeSpec( mStartDateTime.timeSpec() );

  // if it's excluded anyway, don't bother to check if it recurs at all.
  if ( mExDateTimes.containsSorted( dtrecur )) return false;
  if ( mExDates.containsSorted( dtrecur.date() )) return false;
  int i, end;
  for ( i = 0, end = mExRules.count();  i < end;  ++i ) {
    if ( mExRules[i]->recursAt( dtrecur ) ) return false;
  }

  // Check explicit recurrences, then rrules.
  if ( startDateTime() == dtrecur || mRDateTimes.containsSorted( dtrecur ) )
    return true;
  for ( i = 0, end = mRRules.count();  i < end;  ++i ) {
    if ( mRRules[i]->recursAt( dtrecur ) ) return true;
  }

  return false;
}

/** Calculates the cumulative end of the whole recurrence (rdates and rrules).
    If any rrule is infinite, or the recurrence doesn't have any rrules or
    rdates, an invalid date is returned. */
KDateTime Recurrence::endDateTime() const
{
  DateTimeList dts;
  dts << startDateTime();
  if ( !mRDates.isEmpty() ) {
    dts << KDateTime( mRDates.last(), QTime( 0, 0, 0 ), mStartDateTime.timeSpec() );
  }
  if ( !mRDateTimes.isEmpty() ) {
    dts << mRDateTimes.last();
  }
  for ( int i = 0, end = mRRules.count();  i < end;  ++i ) {
    KDateTime rl( mRRules[i]->endDt() );
    // if any of the rules is infinite, the whole recurrence is
    if ( !rl.isValid() ) return KDateTime();
    dts << rl;
  }
  dts.sortUnique();
  if ( dts.isEmpty() ) return KDateTime();
  else return dts.last();
}

/** Calculates the cumulative end of the whole recurrence (rdates and rrules).
    If any rrule is infinite, or the recurrence doesn't have any rrules or
    rdates, an invalid date is returned. */
QDate Recurrence::endDate() const
{
  KDateTime end( endDateTime() );
  if ( end.isValid() ) { return end.date(); }
  else return QDate();
}

void Recurrence::setEndDate( const QDate &date )
{
  KDateTime dt( date, mStartDateTime.time(), mStartDateTime.timeSpec() );
  if ( doesFloat() )
    dt.setTime( QTime( 23, 59, 59 ) );
  setEndDateTime( dt );
}

void Recurrence::setEndDateTime( const KDateTime &dateTime )
{
  if ( mRecurReadOnly ) return;
  RecurrenceRule *rrule = defaultRRule( true );
  if ( !rrule ) return;
  rrule->setEndDt( dateTime );
  updated();
}

int Recurrence::duration() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->duration();
  else return 0;
}

int Recurrence::durationTo( const KDateTime &datetime ) const
{
  // Emulate old behavior: This is just an interface to the first rule!
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( !rrule ) return 0;
  else return rrule->durationTo( datetime );
}

int Recurrence::durationTo( const QDate &date ) const
{
  return durationTo( KDateTime( date, QTime( 23, 59, 59 ), mStartDateTime.timeSpec() ) );
}

void Recurrence::setDuration( int duration )
{
  if ( mRecurReadOnly ) return;
  RecurrenceRule *rrule = defaultRRule( true );
  if ( !rrule ) return;
  rrule->setDuration( duration );
  updated();
}

void Recurrence::shiftTimes(const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec)
{
  if ( mRecurReadOnly ) return;
  mStartDateTime = mStartDateTime.toTimeSpec( oldSpec );
  mStartDateTime.setTimeSpec( newSpec );

  int i, end;
  for ( i = 0, end = mRDateTimes.count();  i < end;  ++i ) {
    mRDateTimes[i] = mRDateTimes[i].toTimeSpec( oldSpec );
    mRDateTimes[i].setTimeSpec( newSpec );
  }
  for ( i = 0, end = mExDateTimes.count();  i < end;  ++i ) {
    mExDateTimes[i] = mExDateTimes[i].toTimeSpec( oldSpec );
    mExDateTimes[i].setTimeSpec( newSpec );
  }
  for ( i = 0, end = mRRules.count();  i < end;  ++i )
    mRRules[i]->shiftTimes( oldSpec, newSpec );
  for ( i = 0, end = mExRules.count();  i < end;  ++i )
    mExRules[i]->shiftTimes( oldSpec, newSpec );
}

void Recurrence::unsetRecurs()
{
  if ( mRecurReadOnly ) return;
  mRRules.clear();
  updated();
}

void Recurrence::clear()
{
  if ( mRecurReadOnly ) return;
  mRRules.clear();
  mExRules.clear();
  mRDates.clear();
  mRDateTimes.clear();
  mExDates.clear();
  mExDateTimes.clear();
  mCachedType = rMax;
  updated();
}

void Recurrence::setStartDateTime( const KDateTime &start )
{
  if ( mRecurReadOnly ) return;
  mStartDateTime = start;
  if ( start.isDateOnly() )
    setFloats( true );
  else {
    setFloats( false );   // set all RRULEs and EXRULEs

    int i, end;
    for ( i = 0, end = mRRules.count();  i < end;  ++i ) {
      mRRules[i]->setStartDt( start );
    }
    for ( i = 0, end = mExRules.count();  i < end;  ++i ) {
      mExRules[i]->setStartDt( start );
    }
  }
  updated();
}

int Recurrence::frequency() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->frequency();
  else return 0;
}

// Emulate the old behaviour. Make this methods just an interface to the
// first rrule
void Recurrence::setFrequency( int freq )
{
  if ( mRecurReadOnly || freq <= 0 ) return;
  RecurrenceRule *rrule = defaultRRule( true );
  if ( rrule )
    rrule->setFrequency( freq );
  updated();
}


// WEEKLY

int Recurrence::weekStart() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->weekStart();
  else return 1;
}

// Emulate the old behavior
QBitArray Recurrence::days() const
{
  QBitArray days( 7 );
  days.fill( 0 );
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) {
    QList<RecurrenceRule::WDayPos> bydays = rrule->byDays();
    for (int i = 0; i < bydays.size(); ++i) {
      if( bydays.at(i).pos() == 0)
        days.setBit( bydays.at(i).day() - 1 );
    }
  }
  return days;
}


// MONTHLY

// Emulate the old behavior
QList<int> Recurrence::monthDays() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->byMonthDays();
  else return QList<int>();
}

// Emulate the old behavior
QList<RecurrenceRule::WDayPos> Recurrence::monthPositions() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->byDays();
  else return QList<RecurrenceRule::WDayPos>();
}


// YEARLY

QList<int> Recurrence::yearDays() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->byYearDays();
  else return QList<int>();
}

QList<int> Recurrence::yearDates() const
{
  return monthDays();
}

QList<int> Recurrence::yearMonths() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->byMonths();
  else return QList<int>();
}

QList<RecurrenceRule::WDayPos> Recurrence::yearPositions() const
{
  return monthPositions();
}



RecurrenceRule *Recurrence::setNewRecurrenceType( RecurrenceRule::PeriodType type, int freq )
{
  if ( mRecurReadOnly || freq <= 0 ) return 0;
  mRRules.clear();
  updated();
  RecurrenceRule *rrule = defaultRRule( true );
  if ( !rrule ) return 0;
  rrule->setRecurrenceType( type );
  rrule->setFrequency( freq );
  rrule->setDuration( -1 );
  return rrule;
}

void Recurrence::setMinutely( int _rFreq )
{
  if ( setNewRecurrenceType( RecurrenceRule::rMinutely, _rFreq ) )
    updated();
}

void Recurrence::setHourly( int _rFreq )
{
  if ( setNewRecurrenceType( RecurrenceRule::rHourly, _rFreq ) )
    updated();
}

void Recurrence::setDaily( int _rFreq )
{
  if ( setNewRecurrenceType( RecurrenceRule::rDaily, _rFreq ) )
    updated();
}

void Recurrence::setWeekly( int freq, int weekStart )
{
  RecurrenceRule *rrule = setNewRecurrenceType( RecurrenceRule::rWeekly, freq );
  if ( !rrule ) return;
  rrule->setWeekStart( weekStart );
  updated();
}

void Recurrence::setWeekly( int freq, const QBitArray &days, int weekStart )
{
  setWeekly( freq, weekStart );
  addMonthlyPos( 0, days );
}

void Recurrence::addWeeklyDays( const QBitArray &days )
{
  addMonthlyPos( 0, days );
}

void Recurrence::setMonthly( int freq )
{
  if ( setNewRecurrenceType( RecurrenceRule::rMonthly, freq ) )
    updated();
}

void Recurrence::addMonthlyPos( short pos, const QBitArray &days )
{
  // Allow 53 for yearly!
  if ( mRecurReadOnly || pos > 53 || pos < -53 ) return;
  RecurrenceRule *rrule = defaultRRule( false );
  if ( !rrule ) return;
  bool changed = false;
  QList<RecurrenceRule::WDayPos> positions = rrule->byDays();

  for ( int i = 0; i < 7; ++i ) {
    if ( days.testBit(i) ) {
      RecurrenceRule::WDayPos p( pos, i + 1 );
      if ( !positions.contains( p ) ) {
        changed = true;
        positions.append( p );
      }
    }
  }
  if ( changed ) {
    rrule->setByDays( positions );
    updated();
  }
}


void Recurrence::addMonthlyPos( short pos, ushort day )
{
  // Allow 53 for yearly!
  if ( mRecurReadOnly || pos > 53 || pos < -53 ) return;
  RecurrenceRule *rrule = defaultRRule( false );
  if ( !rrule ) return;
  QList<RecurrenceRule::WDayPos> positions = rrule->byDays();

  RecurrenceRule::WDayPos p( pos, day );
  if ( !positions.contains( p ) ) {
    positions.append( p );
    rrule->setByDays( positions );
    updated();
  }
}


void Recurrence::addMonthlyDate( short day )
{
  if ( mRecurReadOnly || day > 31 || day < -31 ) return;
  RecurrenceRule *rrule = defaultRRule( true );
  if ( !rrule ) return;

  QList<int> monthDays = rrule->byMonthDays();
  if ( !monthDays.contains( day ) ) {
    monthDays.append( day );
    rrule->setByMonthDays( monthDays );
    updated();
  }
}

void Recurrence::setYearly( int freq )
{
  if ( setNewRecurrenceType( RecurrenceRule::rYearly, freq ) )
    updated();
}


// Daynumber within year
void Recurrence::addYearlyDay( int day )
{
  RecurrenceRule *rrule = defaultRRule( false ); // It must already exist!
  if ( !rrule ) return;

  QList<int> days = rrule->byYearDays();
  if ( !days.contains( day ) ) {
    days << day;
    rrule->setByYearDays( days );
    updated();
  }
}

// day part of date within year
void Recurrence::addYearlyDate( int day )
{
  addMonthlyDate( day );
}

// day part of date within year, given as position (n-th weekday)
void Recurrence::addYearlyPos( short pos, const QBitArray &days )
{
  addMonthlyPos( pos, days );
}


// month part of date within year
void Recurrence::addYearlyMonth( short month )
{
  if ( mRecurReadOnly || month < 1 || month > 12 ) return;
  RecurrenceRule *rrule = defaultRRule( false );
  if ( !rrule ) return;

  QList<int> months = rrule->byMonths();
  if ( !months.contains(month) ) {
    months << month;
    rrule->setByMonths( months );
    updated();
  }
}


TimeList Recurrence::recurTimesOn( const QDate &date, const KDateTime::Spec &timeSpec ) const
{
// kDebug(5800) << "recurTimesOn(" << date << ")" << endl;
  int i, end;
  TimeList times;
  // The whole day is excepted
  if ( mExDates.containsSorted( date ) ) return times;
  // EXRULE takes precedence over RDATE entries, so for floating events,
  // a matching excule also excludes the whole day automatically
  if ( doesFloat() ) {
    for ( i = 0, end = mExRules.count();  i < end;  ++i ) {
      if ( mExRules[i]->recursOn( date, timeSpec ) )
        return times;
    }
  }

  KDateTime dt = startDateTime().toTimeSpec( timeSpec );
  if ( dt.date() == date )
    times << dt.time();
  bool foundDate = false;
  for ( i = 0, end = mRDateTimes.count();  i < end;  ++i ) {
    dt = mRDateTimes[i].toTimeSpec( timeSpec );
    if ( dt.date() == date ) {
      times << dt.time();
      foundDate = true;
    } else if (foundDate) break; // <= Assume that the rdatetime list is sorted
  }
  for ( i = 0, end = mRRules.count();  i < end;  ++i ) {
    times += mRRules[i]->recurTimesOn( date, timeSpec );
  }
  times.sortUnique();

  foundDate = false;
  TimeList extimes;
  for ( i = 0, end = mExDateTimes.count();  i < end;  ++i ) {
    dt = mExDateTimes[i].toTimeSpec( timeSpec );
    if ( dt.date() == date ) {
      extimes << dt.time();
      foundDate = true;
    } else if (foundDate) break;
  }
  if ( !doesFloat() ) {     // we have already checked floating times above
    for ( i = 0, end = mExRules.count();  i < end;  ++i ) {
      extimes += mExRules[i]->recurTimesOn( date, timeSpec );
    }
  }
  extimes.sortUnique();

  int st = 0;
  for ( i = 0, end = extimes.count();  i < end;  ++i ) {
    int j = times.removeSorted( extimes[i], st );
    if ( j >= 0 )
      st = j;
  }
  return times;
}

DateTimeList Recurrence::timesInInterval( const KDateTime &start, const KDateTime &end ) const
{
  int i, count;
  DateTimeList times;
  for ( i = 0, count = mRRules.count();  i < count;  ++i ) {
    times += mRRules[i]->timesInInterval( start, end );
  }
  times += mRDateTimes;
  KDateTime kdt( startDateTime() );
  for ( i = 0, count = mRDates.count();  i < count;  ++i ) {
    kdt.setDate( mRDates[i] );
    times += kdt;
  }
  times.sortUnique();

  // Remove excluded times
  int idt = 0;
  int enddt = times.count();
  for ( i = 0, count = mExDates.count();  i < count && idt < enddt;  ++i ) {
    while ( idt < enddt && times[idt].date() < mExDates[i] ) ++idt;
    while ( idt < enddt && times[idt].date() == mExDates[i] ) {
      times.removeAt(idt);
      --enddt;
    }
  }
  DateTimeList extimes;
  for ( i = 0, count = mExRules.count();  i < count;  ++i ) {
    extimes += mExRules[i]->timesInInterval( start, end );
  }
  extimes += mExDateTimes;
  extimes.sortUnique();

  int st = 0;
  for ( i = 0, count = extimes.count();  i < count;  ++i ) {
    int j = times.removeSorted( extimes[i], st );
    if ( j >= 0 )
      st = j;
  }

  return times;
}

KDateTime Recurrence::getNextDateTime( const KDateTime &preDateTime ) const
{
kDebug(5800) << " Recurrence::getNextDateTime after " << preDateTime.dateTime() << endl;
  KDateTime nextDT = preDateTime;
  // prevent infinite loops, e.g. when an exrule extinguishes an rrule (e.g.
  // the exrule is identical to the rrule). If an occurrence is found, break
  // out of the loop by returning that KDateTime
// TODO_Recurrence: Is a loop counter of 1000 really okay? I mean for secondly
// recurrence, an exdate might exclude more than 1000 intervals!
  int loop = 0;
  while ( loop < 1000 ) {
    // Outline of the algo:
    //   1) Find the next date/time after preDateTime when the event could recur
    //     1.0) Add the start date if it's after preDateTime
    //     1.1) Use the next occurrence from the explicit RDATE lists
    //     1.2) Add the next recurrence for each of the RRULEs
    //   2) Take the earliest recurrence of these = KDateTime nextDT
    //   3) If that date/time is not excluded, either explicitly by an EXDATE or
    //      by an EXRULE, return nextDT as the next date/time of the recurrence
    //   4) If it's excluded, start all at 1), but starting at nextDT (instead
    //      of preDateTime). Loop at most 1000 times.
    ++loop;
    // First, get the next recurrence from the RDate lists
    DateTimeList dates;
    if ( nextDT < startDateTime() ) dates << startDateTime();
    int end;
    // Assume that the rdatetime list is sorted
    int i = mRDateTimes.findGT( nextDT );
    if ( i >= 0 ) {
      dates << mRDateTimes[i];
    }

/*kDebug(5800) << "    nextDT: " << nextDT << ", startDT: " << startDateTime() << endl;
kDebug(5800) << "   getNextDateTime: found " << dates.count() << " RDATES and DTSTART in loop " << loop << endl;*/
    KDateTime kdt( startDateTime() );
    for ( i = 0, end = mRDates.count();  i < end;  ++i ) {
      kdt.setDate( mRDates[i] );
      if ( kdt > nextDT ) {
        dates << kdt;
        break;
      }
    }

    // Add the next occurrences from all RRULEs.
    for ( i = 0, end = mRRules.count();  i < end;  ++i ) {
      KDateTime dt = mRRules[i]->getNextDate( nextDT );
      if ( dt.isValid() ) dates << dt;
    }

    // Take the first of these (all others can't be used later on)
    dates.sortUnique();
// kDebug(5800) << "   getNextDateTime: found " << dates.count() << " dates in loop " << loop << endl;

    if ( dates.isEmpty() ) return KDateTime();
    nextDT = dates.first();

    // Check if that date/time is excluded explicitly or by an exrule:
    if ( !mExDates.containsSorted( nextDT.date() ) && !mExDateTimes.containsSorted( nextDT ) ) {
// kDebug(5800) << "   NextDT" << nextDT << " not excluded by EXDATE " << endl;
      bool allowed = true;
      for ( i = 0, end = mExRules.count();  i < end;  ++i ) {
        allowed = allowed && !( mExRules[i]->recursAt( nextDT ) );
      }
// kDebug(5800) << "   NextDT " << nextDT << ", allowed=" << allowed << endl;
      if ( allowed ) return nextDT;
    }
  }

  // Couldn't find a valid occurrences in 1000 loops, something is wrong!
  return KDateTime();
}

KDateTime Recurrence::getPreviousDateTime( const KDateTime &afterDateTime ) const
{
  KDateTime prevDT = afterDateTime;
  // prevent infinite loops, e.g. when an exrule extinguishes an rrule (e.g.
  // the exrule is identical to the rrule). If an occurrence is found, break
  // out of the loop by returning that KDateTime
  int loop = 0;
  while ( loop < 1000 ) {
    // Outline of the algo:
    //   1) Find the next date/time after preDateTime when the event could recur
    //     1.1) Use the next occurrence from the explicit RDATE lists
    //     1.2) Add the next recurrence for each of the RRULEs
    //   2) Take the earliest recurrence of these = KDateTime nextDT
    //   3) If that date/time is not excluded, either explicitly by an EXDATE or
    //      by an EXRULE, return nextDT as the next date/time of the recurrence
    //   4) If it's excluded, start all at 1), but starting at nextDT (instead
    //      of preDateTime). Loop at most 1000 times.
    ++loop;
    // First, get the next recurrence from the RDate lists
    DateTimeList dates;
    if ( prevDT > startDateTime() ) dates << startDateTime();

    int i = mRDateTimes.findLT( prevDT );
    if ( i >= 0 ) {
      dates << mRDateTimes[i];
    }

    KDateTime kdt( startDateTime() );
    for ( i = mRDates.count();  --i >= 0;  ) {
      kdt.setDate( mRDates[i] );
      if ( kdt < prevDT ) {
        dates << kdt;
        break;
      }
    }

    // Add the previous occurrences from all RRULEs.
    int end;
    for ( i = 0, end = mRRules.count();  i < end;  ++i ) {
      KDateTime dt = mRRules[i]->getPreviousDate( prevDT );
      if ( dt.isValid() ) dates << dt;
    }
kDebug(5800) << "   getPreviousDateTime: found " << dates.count() << " dates in loop " << loop << endl;

    // Take the last of these (all others can't be used later on)
    dates.sortUnique();
    if ( dates.isEmpty() ) return KDateTime();
    prevDT = dates.last();

    // Check if that date/time is excluded explicitly or by an exrule:
    if ( !mExDates.containsSorted( prevDT.date() ) && !mExDateTimes.containsSorted( prevDT ) ) {
      bool allowed = true;
      for ( i = 0, end = mExRules.count();  i < end;  ++i ) {
        allowed = allowed && !( mExRules[i]->recursAt( prevDT ) );
      }
      if ( allowed ) return prevDT;
    }
  }

  // Couldn't find a valid occurrences in 1000 loops, something is wrong!
  return KDateTime();
}


/***************************** PROTECTED FUNCTIONS ***************************/


RecurrenceRule::List Recurrence::rRules() const
{
  return mRRules;
}

void Recurrence::addRRule( RecurrenceRule *rrule )
{
  if ( mRecurReadOnly || !rrule ) return;
  rrule->setFloats( mFloating );
  mRRules.append( rrule );
  rrule->addObserver( this );
  updated();
}

void Recurrence::removeRRule( RecurrenceRule *rrule )
{
  if (mRecurReadOnly) return;
  mRRules.removeAll( rrule );
  rrule->removeObserver( this );
  updated();
}

RecurrenceRule::List Recurrence::exRules() const
{
  return mExRules;
}

void Recurrence::addExRule( RecurrenceRule *exrule )
{
  if ( mRecurReadOnly || !exrule ) return;
  exrule->setFloats( mFloating );
  mExRules.append( exrule );
  exrule->addObserver( this );
  updated();
}

void Recurrence::removeExRule( RecurrenceRule *exrule )
{
  if (mRecurReadOnly) return;
  mExRules.removeAll( exrule );
  exrule->removeObserver( this );
  updated();
}


DateTimeList Recurrence::rDateTimes() const
{
  return mRDateTimes;
}

void Recurrence::setRDateTimes( const DateTimeList &rdates )
{
  if ( mRecurReadOnly ) return;
  mRDateTimes = rdates;
  mRDateTimes.sortUnique();
  updated();
}

void Recurrence::addRDateTime( const KDateTime &rdate )
{
  if ( mRecurReadOnly ) return;
  mRDateTimes.insertSorted( rdate );
  updated();
}


DateList Recurrence::rDates() const
{
  return mRDates;
}

void Recurrence::setRDates( const DateList &rdates )
{
  if ( mRecurReadOnly ) return;
  mRDates = rdates;
  mRDates.sortUnique();
  updated();
}

void Recurrence::addRDate( const QDate &rdate )
{
  if ( mRecurReadOnly ) return;
  mRDates.insertSorted( rdate );
  updated();
}


DateTimeList Recurrence::exDateTimes() const
{
  return mExDateTimes;
}

void Recurrence::setExDateTimes( const DateTimeList &exdates )
{
  if ( mRecurReadOnly ) return;
  mExDateTimes = exdates;
  mExDateTimes.sortUnique();
}

void Recurrence::addExDateTime( const KDateTime &exdate )
{
  if ( mRecurReadOnly ) return;
  mExDateTimes.insertSorted( exdate );
  updated();
}


DateList Recurrence::exDates() const
{
  return mExDates;
}

void Recurrence::setExDates( const DateList &exdates )
{
  if ( mRecurReadOnly ) return;
  mExDates = exdates;
  mExDates.sortUnique();
  updated();
}

void Recurrence::addExDate( const QDate &exdate )
{
  if ( mRecurReadOnly ) return;
  mExDates.insertSorted( exdate );
  updated();
}

void Recurrence::recurrenceChanged( RecurrenceRule * )
{
  updated();
}


// %%%%%%%%%%%%%%%%%% end:Recurrencerule %%%%%%%%%%%%%%%%%%

void Recurrence::dump() const
{
  kDebug(5800) << "Recurrence::dump():" << endl;

  int i;
  int count = mRRules.count();
  kDebug(5800) << "  -) " << count << " RRULEs: " << endl;
  for ( i = 0;  i < count;  ++i ) {
    kDebug(5800) << "    -) RecurrenceRule : " << endl;
    mRRules[i]->dump();
  }
  count = mExRules.count();
  kDebug(5800) << "  -) " << count << " EXRULEs: " << endl;
  for ( i = 0;  i < count;  ++i ) {
    kDebug(5800) << "    -) ExceptionRule : " << endl;
    mExRules[i]->dump();
  }


  count = mRDates.count();
  kDebug(5800) << endl << "  -) " << count << " Recurrence Dates: " << endl;
  for ( i = 0;  i < count;  ++i ) {
    kDebug(5800) << "     " << mRDates[i] << endl;
  }
  count = mRDateTimes.count();
  kDebug(5800) << endl << "  -) " << count << " Recurrence Date/Times: " << endl;
  for ( i = 0;  i < count;  ++i ) {
    kDebug(5800) << "     " << mRDateTimes[i].dateTime() << endl;
  }
  count = mExDates.count();
  kDebug(5800) << endl << "  -) " << count << " Exceptions Dates: " << endl;
  for ( i = 0;  i < count;  ++i ) {
    kDebug(5800) << "     " << mExDates[i] << endl;
  }
  count = mExDateTimes.count();
  kDebug(5800) << endl << "  -) " << count << " Exception Date/Times: " << endl;
  for ( i = 0;  i < count;  ++i ) {
    kDebug(5800) << "     " << mExDateTimes[i].dateTime() << endl;
  }
}
