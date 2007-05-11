/*
    This file is part of libkcal.

    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofe.com>
    Copyright (c) 2006 David Jarvie <software@astrojar.org.uk>

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

#include "recurrencerule.h"

#include <limits.h>
#include <math.h>

#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QStringList>

#include <kdebug.h>
#include <kglobal.h>

using namespace KCal;

// Maximum number of intervals to process
const int LOOP_LIMIT = 10000;

static QString dumpTime( const KDateTime &dt );   // for debugging


/**************************************************************************
 *                               DateHelper                               *
 **************************************************************************/


class DateHelper {
  public:
#ifndef NDEBUG
    static QString dayName( short day );
#endif
    static QDate getNthWeek( int year, int weeknumber, short weekstart = 1 );
    static int weekNumbersInYear( int year, short weekstart = 1 );
    static int getWeekNumber( const QDate &date, short weekstart, int *year = 0 );
    static int getWeekNumberNeg( const QDate &date, short weekstart, int *year = 0 );
};


#ifndef NDEBUG
// TODO: Move to a general library / class, as we need the same in the iCal
//       generator and in the xcal format
QString DateHelper::dayName( short day )
{
  switch ( day ) {
    case 1: return "MO"; break;
    case 2: return "TU"; break;
    case 3: return "WE"; break;
    case 4: return "TH"; break;
    case 5: return "FR"; break;
    case 6: return "SA"; break;
    case 7: return "SU"; break;
    default: return "??";
  }
}
#endif


QDate DateHelper::getNthWeek( int year, int weeknumber, short weekstart )
{
  if ( weeknumber == 0 ) return QDate();
  // Adjust this to the first day of week #1 of the year and add 7*weekno days.
  QDate dt( year, 1, 4 ); // Week #1 is the week that contains Jan 4
  int adjust = -(7 + dt.dayOfWeek() - weekstart) % 7;
  if ( weeknumber > 0 ) {
    dt = dt.addDays( 7 * (weeknumber-1) + adjust );
  } else if ( weeknumber < 0 ) {
    dt = dt.addYears( 1 );
    dt = dt.addDays( 7 * weeknumber + adjust );
  }
  return dt;
}


int DateHelper::getWeekNumber( const QDate &date, short weekstart, int *year )
{
// kDebug(5800) << "Getting week number for " << date << " with weekstart="<<weekstart<<endl;
  if ( year ) *year = date.year();
  QDate dt( date.year(), 1, 4 ); // <= definitely in week #1
  dt = dt.addDays( -(7 + dt.dayOfWeek() - weekstart) % 7 ); // begin of week #1
  QDate dtn( date.year()+1, 1, 4 ); // <= definitely first week of next year
  dtn = dtn.addDays( -(7 + dtn.dayOfWeek() - weekstart) % 7 );

  int daysto = dt.daysTo( date );
  int dayston = dtn.daysTo( date );
  if ( daysto < 0 ) {
    if ( year ) *year = date.year()-1;
    dt = QDate( date.year()-1, 1, 4 );
    dt = dt.addDays( -(7 + dt.dayOfWeek() - weekstart) % 7 ); // begin of week #1
    daysto = dt.daysTo( date );
  } else if ( dayston >= 0 ) {
    // in first week of next year;
    if ( year ) *year = date.year() + 1;
    dt = dtn;
    daysto = dayston;
  }
  return daysto / 7 + 1;
}

int DateHelper::weekNumbersInYear( int year, short weekstart )
{
  QDate dt( year, 1, weekstart );
  QDate dt1( year + 1, 1, weekstart );
  return dt.daysTo( dt1 ) / 7;
}

// Week number from the end of the year
int DateHelper::getWeekNumberNeg( const QDate &date, short weekstart, int *year )
{
  int weekpos = getWeekNumber( date, weekstart, year );
  return weekNumbersInYear( *year, weekstart ) - weekpos - 1;
}





/**************************************************************************
 *                       RecurrenceRule::Constraint                       *
 **************************************************************************/


RecurrenceRule::Constraint::Constraint( KDateTime::Spec spec, int wkst )
  : weekstart( wkst ),
    timespec( spec )
{
  clear();
}

RecurrenceRule::Constraint::Constraint( const KDateTime &dt, PeriodType type, int wkst )
  : weekstart( wkst ),
    timespec( dt.timeSpec() )
{
  readDateTime( dt, type );
}

void RecurrenceRule::Constraint::clear()
{
  year = 0;
  month = 0;
  day = 0;
  hour = -1;
  minute = -1;
  second = -1;
  weekday = 0;
  weekdaynr = 0;
  weeknumber = 0;
  yearday = 0;
  secondOccurrence = false;
}

bool RecurrenceRule::Constraint::matches( const QDate &dt, RecurrenceRule::PeriodType type ) const
{
  // If the event recurs in week 53 or 1, the day might not belong to the same
  // year as the week it is in. E.g. Jan 1, 2005 is in week 53 of year 2004.
  // So we can't simply check the year in that case!
  if ( weeknumber == 0 ) {
    if ( year > 0 && year != dt.year() ) return false;
  } else {
    int y;
    if ( weeknumber > 0 &&
         weeknumber != DateHelper::getWeekNumber( dt, weekstart, &y ) ) return false;
    if ( weeknumber < 0 &&
         weeknumber != DateHelper::getWeekNumberNeg( dt, weekstart, &y ) ) return false;
    if ( year > 0 && year != y ) return false;
  }

  if ( month > 0 && month != dt.month() ) return false;
  if ( day > 0 && day != dt.day() ) return false;
  if ( day < 0 && dt.day() != (dt.daysInMonth() + day + 1 ) ) return false;
  if ( weekday > 0 ) {
    if ( weekday != dt.dayOfWeek() ) return false;
    if ( weekdaynr != 0 ) {
      // If it's a yearly recurrence and a month is given, the position is
      // still in the month, not in the year.
      bool inMonth = (type == rMonthly) || ( type == rYearly && month > 0 );
      // Monthly
      if ( weekdaynr > 0 && inMonth &&
           weekdaynr != (dt.day() - 1)/7 + 1 ) return false;
      if ( weekdaynr < 0 && inMonth &&
           weekdaynr != -((dt.daysInMonth() - dt.day() )/7 + 1 ) )
        return false;
      // Yearly
      if ( weekdaynr > 0 && !inMonth &&
           weekdaynr != (dt.dayOfYear() - 1)/7 + 1 ) return false;
      if ( weekdaynr < 0 && !inMonth &&
           weekdaynr != -((dt.daysInYear() - dt.dayOfYear() )/7 + 1 ) )
        return false;
    }
  }
  if ( yearday > 0 && yearday != dt.dayOfYear() ) return false;
  if ( yearday < 0 && yearday != dt.daysInYear() - dt.dayOfYear() + 1 )
    return false;
  return true;
}

/* Check for a match with the specified date/time.
 * The date/time's time specification must correspond with that of the start date/time.
 */
bool RecurrenceRule::Constraint::matches( const KDateTime &dt, RecurrenceRule::PeriodType type ) const
{
  if ( !matches( dt.date(), type ) ) return false;
  if ( hour >= 0 && (hour != dt.time().hour() || secondOccurrence != dt.isSecondOccurrence()) ) return false;
  if ( minute >= 0 && minute != dt.time().minute() ) return false;
  if ( second >= 0 && second != dt.time().second() ) return false;
  return true;
}

bool RecurrenceRule::Constraint::isConsistent( PeriodType /*period*/) const
{
  // TODO: Check for consistency, e.g. byyearday=3 and bymonth=10
  return true;
}

// Return a date/time set to the constraint values, but with those parts less
// significant than the given period type set to 1 (for dates) or 0 (for times).
KDateTime RecurrenceRule::Constraint::intervalDateTime( RecurrenceRule::PeriodType type ) const
{
  QTime t( 0, 0, 0 );
  QDate d( year, (month>0)?month:1, (day>0)?day:1 );
  if ( day < 0 )
    d = d.addDays( d.daysInMonth() + day );
  switch ( type ) {
    case rSecondly:
      t.setHMS( hour, minute, second ); break;
    case rMinutely:
      t.setHMS( hour, minute, 0 ); break;
    case rHourly:
      t.setHMS( hour, 0, 0 ); break;
    case rDaily:
      break;
    case rWeekly:
      d = DateHelper::getNthWeek( year, weeknumber, weekstart ); break;
    case rMonthly:
      d.setYMD( year, month, 1 ); break;
    case rYearly:
      d.setYMD( year, 1, 1 ); break;
    default:
      break;
  }
  KDateTime result( d, t, timespec );
  if ( secondOccurrence )
    result.setSecondOccurrence( true );
  return result;
}


//           Y  M  D | H  Mn S | WD #WD | WN | YD
// required:
//           x       | x  x  x |        |    |
// 0) Trivial: Exact date given, maybe other restrictions
//           x  x  x | x  x  x |        |    |
// 1) Easy case: no weekly restrictions -> at most a loop through possible dates
//           x  +  + | x  x  x |  -  -  |  - |  -
// 2) Year day is given -> date known
//           x       | x  x  x |        |    |  +
// 3) week number is given -> loop through all days of that week. Further
//    restrictions will be applied in the end, when we check all dates for
//    consistency with the constraints
//           x       | x  x  x |        |  + | (-)
// 4) week day is specified ->
//           x       | x  x  x |  x  ?  | (-)| (-)
// 5) All possiblecases have already been treated, so this must be an error!

QList<KDateTime> RecurrenceRule::Constraint::dateTimes( RecurrenceRule::PeriodType type ) const
{
// kDebug(5800) << "              RecurrenceRule::Constraint::dateTimes: " << endl;
  QList<KDateTime> result;
  bool done = false;
  if ( !isConsistent( type ) ) return result;
  // TODO_Recurrence: Handle floating
  QTime tm( hour, minute, second );

  if ( !done && day > 0 && month > 0 ) {
    appendDateTime( QDate( year, month, day ), tm, result );
    done = true;
  }
  if ( !done && day < 0 && month > 0 ) {
    QDate d( year, month, 1 );
    appendDateTime( d.addDays( d.daysInMonth() + day ), tm, result );
    done = true;
  }


  if ( !done && weekday == 0 && weeknumber == 0 && yearday == 0 ) {
    // Easy case: date is given, not restrictions by week or yearday
    uint mstart = (month>0) ? month : 1;
    uint mend = (month <= 0) ? 12 : month;
    for ( uint m = mstart; m <= mend; ++m ) {
      uint dstart, dend;
      if ( day > 0 ) {
        dstart = dend = day;
      } else if ( day < 0 ) {
        QDate date( year, month, 1 );
        dstart = dend = date.daysInMonth() + day + 1;
      } else {
        QDate date( year, month, 1 );
        dstart = 1;
        dend = date.daysInMonth();
      }
      for ( uint d = dstart; d <= dend; ++d ) {
        appendDateTime( QDate( year, m, d ), tm, result );
      }
    }
    done = true;
  }

  // Else: At least one of the week / yearday restrictions was given...
  // If we have a yearday (and of course a year), we know the exact date
  if ( !done && yearday != 0 ) {
    // yearday < 0 means from end of year, so we'll need Jan 1 of the next year
    QDate d( year + ((yearday>0)?0:1), 1, 1 );
    d = d.addDays( yearday - ((yearday>0)?1:0) );
    appendDateTime( d, tm, result );
    done = true;
  }

  // Else: If we have a weeknumber, we have at most 7 possible dates, loop through them
  if ( !done && weeknumber != 0 ) {
    QDate wst( DateHelper::getNthWeek( year, weeknumber, weekstart ) );
    if ( weekday != 0 ) {
      wst = wst.addDays( (7 + weekday - weekstart ) % 7 );
      appendDateTime( wst, tm, result );
    } else {
      for ( int i = 0; i < 7; ++i ) {
        appendDateTime( wst, tm, result );
        wst = wst.addDays( 1 );
      }
    }
    done = true;
  }

  // weekday is given
  if ( !done && weekday != 0 ) {
    QDate dt( year, 1, 1 );
    // If type == yearly and month is given, pos is still in month not year!
    // TODO_Recurrence: Correct handling of n-th  BYDAY...
    int maxloop = 53;
    bool inMonth = ( type == rMonthly) || ( type == rYearly && month > 0 );
    if ( inMonth && month > 0 ) {
      dt = QDate( year, month, 1 );
      maxloop = 5;
    }
    if ( weekdaynr < 0 ) {
      // From end of period (month, year) => relative to begin of next period
      if ( inMonth )
        dt = dt.addMonths( 1 );
      else
        dt = dt.addYears( 1 );
    }
    int adj = ( 7 + weekday - dt.dayOfWeek() ) % 7;
    dt = dt.addDays( adj ); // correct first weekday of the period

    if ( weekdaynr > 0 ) {
      dt = dt.addDays( ( weekdaynr - 1 ) * 7 );
      appendDateTime( dt, tm, result );
    } else if ( weekdaynr < 0 ) {
      dt = dt.addDays( weekdaynr * 7 );
      appendDateTime( dt, tm, result );
    } else {
      // loop through all possible weeks, non-matching will be filtered later
      for ( int i = 0; i < maxloop; ++i ) {
        appendDateTime( dt, tm, result );
        dt = dt.addDays( 7 );
      }
    }
  } // weekday != 0


  // Only use those times that really match all other constraints, too
  QList<KDateTime> valid;
  for ( int i = 0, iend = result.count();  i < iend;  ++i ) {
    if ( matches( result[i], type ) )
      valid.append( result[i] );
  }
  // Don't sort it here, would be unnecessary work. The results from all
  // constraints will be merged to one big list of the interval. Sort that one!
  return valid;
}

void RecurrenceRule::Constraint::appendDateTime( const QDate &date, const QTime &time, QList<KDateTime> &list ) const
{
  KDateTime dt( date, time, timespec );
  if ( dt.isValid() ) {
    if ( secondOccurrence )
      dt.setSecondOccurrence( true );
    list.append( dt );
  }
}


bool RecurrenceRule::Constraint::increase( RecurrenceRule::PeriodType type, int freq )
{
  // convert the first day of the interval to QDateTime
  KDateTime dt( intervalDateTime( type ) );

  // Now add the intervals
  switch ( type ) {
    case rSecondly:
      dt = dt.addSecs( freq ); break;
    case rMinutely:
      dt = dt.addSecs( 60*freq ); break;
    case rHourly:
      dt = dt.addSecs( 3600 * freq ); break;
    case rDaily:
      dt = dt.addDays( freq ); break;
    case rWeekly:
      dt = dt.addDays( 7*freq ); break;
    case rMonthly:
      dt = dt.addMonths( freq ); break;
    case rYearly:
      dt = dt.addYears( freq ); break;
    default:
      break;
  }
  // Convert back from KDateTime to the Constraint class
  readDateTime( dt, type );

  return true;
}

// Set the constraint's value appropriate to 'type', to the value contained in a date/time.
bool RecurrenceRule::Constraint::readDateTime( const KDateTime &dt, PeriodType type )
{
  clear();
  switch ( type ) {
    // Really fall through! Only weekly needs to be treated differentely!
    case rSecondly:
      second = dt.time().second();
    case rMinutely:
      minute = dt.time().minute();
    case rHourly:
      hour = dt.time().hour();
      secondOccurrence = dt.isSecondOccurrence();
    case rDaily:
      day = dt.date().day();
    case rMonthly:
      month = dt.date().month();
    case rYearly:
      year = dt.date().year();
      break;

    case rWeekly:
      // Determine start day of the current week, calculate the week number from that
      weeknumber = DateHelper::getWeekNumber( dt.date(), weekstart, &year );
      break;
    default:
      break;
  }
  return true;
}



/**************************************************************************
 *                              RecurrenceRule                            *
 **************************************************************************/

RecurrenceRule::RecurrenceRule( )
: mPeriod( rNone ), mFrequency( 0 ), mIsReadOnly( false ),
  mFloating( false ),
  mWeekStart(1)
{
}

RecurrenceRule::RecurrenceRule( const RecurrenceRule &r )
{
  mRRule = r.mRRule;
  mPeriod = r.mPeriod;
  mDateStart = r.mDateStart;
  mDuration = r.mDuration;
  mDateEnd = r.mDateEnd;
  mFrequency = r.mFrequency;

  mIsReadOnly = r.mIsReadOnly;
  mFloating = r.mFloating;

  mBySeconds = r.mBySeconds;
  mByMinutes = r.mByMinutes;
  mByHours = r.mByHours;
  mByDays = r.mByDays;
  mByMonthDays = r.mByMonthDays;
  mByYearDays = r.mByYearDays;
  mByWeekNumbers = r.mByWeekNumbers;
  mByMonths = r.mByMonths;
  mBySetPos = r.mBySetPos;
  mWeekStart = r.mWeekStart;

  setDirty();
}

RecurrenceRule::~RecurrenceRule()
{
}

bool RecurrenceRule::operator==( const RecurrenceRule& r ) const
{
  if ( mPeriod != r.mPeriod ) return false;
  if ( mDateStart != r.mDateStart ) return false;
  if ( mDuration != r.mDuration ) return false;
  if ( mDateEnd != r.mDateEnd ) return false;
  if ( mFrequency != r.mFrequency ) return false;

  if ( mIsReadOnly != r.mIsReadOnly ) return false;
  if ( mFloating != r.mFloating ) return false;

  if ( mBySeconds != r.mBySeconds ) return false;
  if ( mByMinutes != r.mByMinutes ) return false;
  if ( mByHours != r.mByHours ) return false;
  if ( mByDays != r.mByDays ) return false;
  if ( mByMonthDays != r.mByMonthDays ) return false;
  if ( mByYearDays != r.mByYearDays ) return false;
  if ( mByWeekNumbers != r.mByWeekNumbers ) return false;
  if ( mByMonths != r.mByMonths ) return false;
  if ( mBySetPos != r.mBySetPos ) return false;
  if ( mWeekStart != r.mWeekStart ) return false;

  return true;
}

void RecurrenceRule::addObserver( RuleObserver *observer )
{
  if ( !mObservers.contains( observer ) )
    mObservers.append( observer );
}

void RecurrenceRule::removeObserver( RuleObserver *observer )
{
  if ( mObservers.contains( observer ) )
    mObservers.removeAll( observer );
}



void RecurrenceRule::setRecurrenceType( PeriodType period )
{
  if ( isReadOnly() ) return;
  mPeriod = period;
  setDirty();
}


KDateTime RecurrenceRule::endDt( bool *result ) const
{
  if ( result ) *result = false;
  if ( mPeriod == rNone ) return KDateTime();
  if ( mDuration < 0 ) {
    if ( result ) result = false;
    return KDateTime();
  } else if ( mDuration == 0 ) {
    return mDateEnd;
  } else {
    // N occurrences. Check if we have a full cache. If so, return the cached end date.
    if ( ! mCached ) {
      // If not enough occurrences can be found (i.e. inconsistent constraints)
      if ( !buildCache() ) {
        if ( result ) result = false;
        return KDateTime();
      }
    }
    return mCachedDateEnd;
  }
  return KDateTime();
}

void RecurrenceRule::setEndDt( const KDateTime &dateTime )
{
  if ( isReadOnly() ) return;
  mDateEnd = dateTime;
  mDuration = 0; // set to 0 because there is an end date/time
  setDirty();
}

void RecurrenceRule::setDuration(int duration)
{
  if ( isReadOnly() ) return;
  mDuration = duration;
  setDirty();
}

void RecurrenceRule::setFloats( bool floats )
{
  if ( isReadOnly() ) return;
  mFloating = floats;
  setDirty();
}

void RecurrenceRule::clear()
{
  if ( isReadOnly() ) return;
  mPeriod = rNone;
  mBySeconds.clear();
  mByMinutes.clear();
  mByHours.clear();
  mByDays.clear();
  mByMonthDays.clear();
  mByYearDays.clear();
  mByWeekNumbers.clear();
  mByMonths.clear();
  mBySetPos.clear();
  mWeekStart = 1;

  setDirty();
}

void RecurrenceRule::setDirty()
{
  mConstraints.clear();
  buildConstraints();
  mDirty = true;
  mCached = false;
  mCachedDates.clear();
  for ( int i = 0, iend = mObservers.count();  i < iend;  ++i ) {
    if ( mObservers[i] )
      mObservers[i]->recurrenceChanged( this );
  }
}

void RecurrenceRule::setStartDt( const KDateTime &start )
{
  if ( isReadOnly() ) return;
  mDateStart = start;
  setDirty();
}

void RecurrenceRule::setFrequency(int freq)
{
  if ( isReadOnly() || freq <= 0 ) return;
  mFrequency = freq;
  setDirty();
}

void RecurrenceRule::setBySeconds( const QList<int> bySeconds )
{
  if ( isReadOnly() ) return;
  mBySeconds = bySeconds;
  setDirty();
}

void RecurrenceRule::setByMinutes( const QList<int> byMinutes )
{
  if ( isReadOnly() ) return;
  mByMinutes = byMinutes;
  setDirty();
}

void RecurrenceRule::setByHours( const QList<int> byHours )
{
  if ( isReadOnly() ) return;
  mByHours = byHours;
  setDirty();
}


void RecurrenceRule::setByDays( const QList<WDayPos> byDays )
{
  if ( isReadOnly() ) return;
  mByDays = byDays;
  setDirty();
}

void RecurrenceRule::setByMonthDays( const QList<int> byMonthDays )
{
  if ( isReadOnly() ) return;
  mByMonthDays = byMonthDays;
  setDirty();
}

void RecurrenceRule::setByYearDays( const QList<int> byYearDays )
{
  if ( isReadOnly() ) return;
  mByYearDays = byYearDays;
  setDirty();
}

void RecurrenceRule::setByWeekNumbers( const QList<int> byWeekNumbers )
{
  if ( isReadOnly() ) return;
  mByWeekNumbers = byWeekNumbers;
  setDirty();
}

void RecurrenceRule::setByMonths( const QList<int> byMonths )
{
  if ( isReadOnly() ) return;
  mByMonths = byMonths;
  setDirty();
}

void RecurrenceRule::setBySetPos( const QList<int> bySetPos )
{
  if ( isReadOnly() ) return;
  mBySetPos = bySetPos;
  setDirty();
}

void RecurrenceRule::setWeekStart( short weekStart )
{
  if ( isReadOnly() ) return;
  mWeekStart = weekStart;
  setDirty();
}

void RecurrenceRule::shiftTimes(const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec)
{
  mDateStart = mDateStart.toTimeSpec( oldSpec );
  mDateStart.setTimeSpec( newSpec );
  if ( mDuration == 0 ) {
    mDateEnd = mDateEnd.toTimeSpec( oldSpec );
    mDateEnd.setTimeSpec( newSpec );
  }
  setDirty();
}



// Taken from recurrence.cpp
// int RecurrenceRule::maxIterations() const
// {
//   /* Find the maximum number of iterations which may be needed to reach the
//    * next actual occurrence of a monthly or yearly recurrence.
//    * More than one iteration may be needed if, for example, it's the 29th February,
//    * the 31st day of the month or the 5th Monday, and the month being checked is
//    * February or a 30-day month.
//    * The following recurrences may never occur:
//    * - For rMonthlyDay: if the frequency is a whole number of years.
//    * - For rMonthlyPos: if the frequency is an even whole number of years.
//    * - For rYearlyDay, rYearlyMonth: if the frequeny is a multiple of 4 years.
//    * - For rYearlyPos: if the frequency is an even number of years.
//    * The maximum number of iterations needed, assuming that it does actually occur,
//    * was found empirically.
//    */
//   switch (recurs) {
//     case rMonthlyDay:
//       return (rFreq % 12) ? 6 : 8;
//
//     case rMonthlyPos:
//       if (rFreq % 12 == 0) {
//         // Some of these frequencies may never occur
//         return (rFreq % 84 == 0) ? 364         // frequency = multiple of 7 years
//              : (rFreq % 48 == 0) ? 7           // frequency = multiple of 4 years
//              : (rFreq % 24 == 0) ? 14 : 28;    // frequency = multiple of 2 or 1 year
//       }
//       // All other frequencies will occur sometime
//       if (rFreq > 120)
//         return 364;    // frequencies of > 10 years will hit the date limit first
//       switch (rFreq) {
//         case 23:   return 50;
//         case 46:   return 38;
//         case 56:   return 138;
//         case 66:   return 36;
//         case 89:   return 54;
//         case 112:  return 253;
//         default:   return 25;       // most frequencies will need < 25 iterations
//       }
//
//     case rYearlyMonth:
//     case rYearlyDay:
//       return 8;          // only 29th Feb or day 366 will need more than one iteration
//
//     case rYearlyPos:
//       if (rFreq % 7 == 0)
//         return 364;    // frequencies of a multiple of 7 years will hit the date limit first
//       if (rFreq % 2 == 0) {
//         // Some of these frequencies may never occur
//         return (rFreq % 4 == 0) ? 7 : 14;    // frequency = even number of years
//       }
//       return 28;
//   }
//   return 1;
// }

void RecurrenceRule::buildConstraints()
{
  mConstraints.clear();
  Constraint con( mDateStart.timeSpec() );
  if ( mWeekStart > 0 ) con.weekstart = mWeekStart;
  mConstraints.append( con );

  int c, cend;
  int i, iend;
  Constraint::List tmp;

  #define intConstraint( list, element ) \
  if ( !list.isEmpty() ) { \
    for ( c = 0, cend = mConstraints.count();  c < cend;  ++c ) { \
      for ( i = 0, iend = list.count();  i < iend;  ++i ) { \
        con = mConstraints[c]; \
        con.element = list[i]; \
        tmp.append( con ); \
      } \
    } \
    mConstraints = tmp; \
    tmp.clear(); \
  }

  intConstraint( mBySeconds, second );
  intConstraint( mByMinutes, minute );
  intConstraint( mByHours, hour );
  intConstraint( mByMonthDays, day );
  intConstraint( mByMonths, month );
  intConstraint( mByYearDays, yearday );
  intConstraint( mByWeekNumbers, weeknumber );
  #undef intConstraint

  if ( !mByDays.isEmpty() ) {
    for ( c = 0, cend = mConstraints.count();  c < cend;  ++c ) {
      for ( i = 0, iend = mByDays.count();  i < iend;  ++i ) {
        con = mConstraints[c];
        con.weekday = mByDays[i].day();
        con.weekdaynr = mByDays[i].pos();
        tmp.append( con );
      }
    }
    mConstraints = tmp;
    tmp.clear();
  }

  #define fixConstraint( element, value ) \
  { \
    tmp.clear(); \
    for ( c = 0, cend = mConstraints.count();  c < cend;  ++c ) { \
      con = mConstraints[c]; con.element = value; tmp.append( con ); \
    } \
    mConstraints = tmp; \
  }
  // Now determine missing values from DTSTART. This can speed up things,
  // because we have more restrictions and save some loops.

  // TODO: Does RFC 2445 intend to restrict the weekday in all cases of weekly?
  if ( mPeriod == rWeekly && mByDays.isEmpty() ) {
    fixConstraint( weekday, mDateStart.date().dayOfWeek() );
  }

  // Really fall through in the cases, because all smaller time intervals are
  // constrained from dtstart
  switch ( mPeriod ) {
    case rYearly:
      if ( mByDays.isEmpty() && mByWeekNumbers.isEmpty() && mByYearDays.isEmpty() && mByMonths.isEmpty() ) {
        fixConstraint( month, mDateStart.date().month() );
      }
    case rMonthly:
      if ( mByDays.isEmpty() && mByWeekNumbers.isEmpty() && mByYearDays.isEmpty() && mByMonthDays.isEmpty() ) {
        fixConstraint( day, mDateStart.date().day() );
      }

    case rWeekly:
    case rDaily:
      if ( mByHours.isEmpty() ) {
        fixConstraint( hour, mDateStart.time().hour() );
      }
    case rHourly:
      if ( mByMinutes.isEmpty() ) {
        fixConstraint( minute, mDateStart.time().minute() );
      }
    case rMinutely:
      if ( mBySeconds.isEmpty() ) {
        fixConstraint( second, mDateStart.time().second() );
      }
    case rSecondly:
    default:
      break;
  }
  #undef fixConstraint

  for ( c = 0, cend = mConstraints.count();  c < cend;  ) {
    if ( mConstraints[c].isConsistent( mPeriod ) ) {
      ++c;
    } else {
      mConstraints.removeAt( c );
      --cend;
    }
  }
}

// Build and cache a list of all occurrences.
// Only call buildCache() if mDuration > 0.
bool RecurrenceRule::buildCache() const
{
// kDebug(5800) << "         RecurrenceRule::buildCache: " << endl;
  // Build the list of all occurrences of this event (we need that to determine
  // the end date!)
  Constraint interval( getNextValidDateInterval( startDt(), recurrenceType() ) );
  QDateTime next;

  DateTimeList dts = datesForInterval( interval, recurrenceType() );
  // Only use dates after the event has started (start date is only included
  // if it matches)
  int i = dts.findLT( startDt() );
  if ( i >= 0 ) {
    dts.erase( dts.begin(), dts.begin() + i + 1 );
  }


  int loopnr = 0;
  int dtnr = dts.count();
  // some validity checks to avoid infinite loops (i.e. if we have
  // done this loop already 10000 times, bail out )
  while ( loopnr < LOOP_LIMIT && dtnr < mDuration ) {
    interval.increase( recurrenceType(), frequency() );
    // The returned date list is already sorted!
    dts += datesForInterval( interval, recurrenceType() );
    dtnr = dts.count();
    ++loopnr;
  }
  if ( dts.count() > mDuration ) {
    // we have picked up more occurrences than necessary, remove them
    dts.erase( dts.begin() + mDuration, dts.end() );
  }
  mCached = true;
  mCachedDates = dts;

kDebug(5800) << "    Finished Building Cache, cache has " << dts.count() << " entries:" << endl;
// it = dts.begin();
// while ( it != dts.end() ) {
//   kDebug(5800) << "            -=> " << dumpTime(*it) << endl;
//   ++it;
// }
  if ( int(dts.count()) == mDuration ) {
    mCachedDateEnd = dts.last();
    return true;
  } else {
    // The cached date list is incomplete
    mCachedDateEnd = KDateTime();
    mCachedLastDate = interval.intervalDateTime( recurrenceType() );
    return false;
  }
}

bool RecurrenceRule::dateMatchesRules( const KDateTime &kdt ) const
{
  KDateTime dt = kdt.toTimeSpec( mDateStart.timeSpec() );
  for ( int i = 0, iend = mConstraints.count();  i < iend;  ++i ) {
    if ( mConstraints[i].matches( dt, recurrenceType() ) )
      return true;
  }
  return false;
}

bool RecurrenceRule::recursOn( const QDate &qd, const KDateTime::Spec &timeSpec ) const
{
  int i, iend;
  if ( doesFloat() ) {
    // It's a date-only rule, so it has no time specification.
    // Therefore ignore 'timeSpec'.
    if ( qd < startDt().date() )
      return false;
    // Start date is only included if it really matches
    QDate endDate;
    if ( mDuration >= 0 ) {
      endDate =  endDt().date();
      if ( qd > endDate )
        return false;
    }

    // The date must be in an appropriate interval (getNextValidDateInterval),
    // Plus it must match at least one of the constraints
    bool match = false;
    for ( i = 0, iend = mConstraints.count();  i < iend && !match;  ++i ) {
      match = mConstraints[i].matches( qd, recurrenceType() );
    }
    if ( !match )
      return false;

    KDateTime start( qd, QTime( 0, 0, 0 ), mDateStart.timeSpec() );
    Constraint interval( getNextValidDateInterval( start, recurrenceType() ) );
    // Constraint::matches is quite efficient, so first check if it can occur at
    // all before we calculate all actual dates.
    if ( !interval.matches( qd, recurrenceType() ) )
      return false;
    // We really need to obtain the list of dates in this interval, since
    // otherwise BYSETPOS will not work (i.e. the date will match the interval,
    // but BYSETPOS selects only one of these matching dates!
    KDateTime end = start.addDays(1);
    do {
      DateTimeList dts = datesForInterval( interval, recurrenceType() );
      for ( i = 0, iend = dts.count();  i < iend;  ++i ) {
        if ( dts[i].date() >= qd ) {
          return ( dts[i].date() == qd );
        }
      }
      interval.increase( recurrenceType(), frequency() );
    } while ( interval.intervalDateTime( recurrenceType() ) < end );
    return false;
  }

  // It's a date-time rule, so we need to take the time specification into account.
  KDateTime start( qd, QTime(0,0,0), timeSpec );
  KDateTime end = start.addDays(1);
//   kDebug(5800) << "         RecurrenceRule::recursOn(" << qd << ", spec)" << endl;
  if ( end < startDt() )
    return false;
  // Start date is only included if it really matches
  if ( mDuration >= 0 ) {
    KDateTime endRecur = endDt();
    if ( endRecur.isValid() ) {
      if ( start > endRecur ) {
//  kDebug(5800) << "         start>endRecur" << endl;
        return false;
      }
      if ( end > endRecur ) {
//  kDebug(5800) << "         end>endRecur" << endl;
        end = endRecur;    // limit end-of-day time to end of recurrence rule
      }
    }
  }

  // Find the start and end dates in the time spec for the rule
  QDate startDay = start.toTimeSpec( mDateStart.timeSpec() ).date();
  QDate endDay = end.toTimeSpec( mDateStart.timeSpec() ).addSecs( -1 ).date();
  int dayCount = startDay.daysTo( endDay ) + 1;

  // The date must be in an appropriate interval (getNextValidDateInterval),
  // Plus it must match at least one of the constraints
  bool match = false;
  for ( i = 0, iend = mConstraints.count();  i < iend && !match;  ++i ) {
    match = mConstraints[i].matches( startDay, recurrenceType() );
    for ( int d = 1;  d < dayCount && !match;  ++d ) {
      match = mConstraints[i].matches( startDay.addDays( d ), recurrenceType() );
    }
  }
  if ( !match )
    return false;

  Constraint interval( getNextValidDateInterval( start, recurrenceType() ) );
  // Constraint::matches is quite efficient, so first check if it can occur at
  // all before we calculate all actual dates.
  match = false;
  Constraint intervalm = interval;
  do {
    match = intervalm.matches( startDay, recurrenceType() );
    for ( int d = 1;  d < dayCount && !match;  ++d ) {
      match = intervalm.matches( startDay.addDays( d ), recurrenceType() );
    }
    if ( match )
      break;
    intervalm.increase( recurrenceType(), frequency() );
//kDebug(5800)<<"recursOn(): match interval: "<<intervalm.intervalDateTime( recurrenceType() ).dateTime()<<endl;
  } while ( intervalm.intervalDateTime( recurrenceType() ) < end );
  if ( !match )
    return false;

  // We really need to obtain the list of dates in this interval, since
  // otherwise BYSETPOS will not work (i.e. the date will match the interval,
  // but BYSETPOS selects only one of these matching dates!
  do {
    DateTimeList dts = datesForInterval( interval, recurrenceType() );
    int i = dts.findGE( start );
    if ( i >= 0 ) {
      return ( dts[i] < end );
    }
    interval.increase( recurrenceType(), frequency() );
//kDebug(5800)<<"recursOn(): times interval: "<<interval.intervalDateTime( recurrenceType() ).dateTime()<<endl;
  } while ( interval.intervalDateTime( recurrenceType() ) < end );
  return false;
}


bool RecurrenceRule::recursAt( const KDateTime &kdt ) const
{
  // Convert to the time spec used by this recurrence rule
  KDateTime dt( kdt.toTimeSpec( mDateStart.timeSpec() ) );
// kDebug(5800) << "         RecurrenceRule::recursAt: " << dumpTime(dt) << endl;
  if ( doesFloat() )
    return recursOn( dt.date(), dt.timeSpec() );
  if ( dt < startDt() )
    return false;
  // Start date is only included if it really matches
  if ( mDuration >= 0 && dt > endDt() )
    return false;

  // The date must be in an appropriate interval (getNextValidDateInterval),
  // Plus it must match at least one of the constraints
  if ( !dateMatchesRules( dt ) )
    return false;
  // if it recurs every interval, speed things up...
//   if ( mFrequency == 1 && mBySetPos.isEmpty() && mByDays.isEmpty() ) return true;
  Constraint interval( getNextValidDateInterval( dt, recurrenceType() ) );
  // TODO_Recurrence: Does this work with BySetPos???
  if ( interval.matches( dt, recurrenceType() ) )
    return true;

  return false;
}


TimeList RecurrenceRule::recurTimesOn( const QDate &date, const KDateTime::Spec &timeSpec) const
{
// kDebug(5800) << "         RecurrenceRule::recurTimesOn(" << date << ")" << endl;
  TimeList lst;
  if ( doesFloat() )
    return lst;
  KDateTime start( date, QTime(0,0,0), timeSpec );
  KDateTime end = start.addDays(1).addSecs(-1);
  DateTimeList dts = timesInInterval( start, end );   // returns between start and end inclusive
  for ( int i = 0, iend = dts.count();  i < iend;  ++i ) {
    lst += dts[i].toTimeSpec( timeSpec ).time();
  }
  return lst;
}


/** Returns the number of recurrences up to and including the date/time specified. */
int RecurrenceRule::durationTo( const KDateTime &dt ) const
{
  // Convert to the time spec used by this recurrence rule
  KDateTime toDate( dt.toTimeSpec( mDateStart.timeSpec() ) );
// kDebug(5800) << "         RecurrenceRule::durationTo: " << dumpTime(toDate) << endl;
  // Easy cases: either before start, or after all recurrences and we know
  // their number
  if ( toDate < startDt() )
    return 0;
  // Start date is only included if it really matches
  if ( mDuration > 0 && toDate >= endDt() )
    return mDuration;

  return timesInInterval( startDt(), toDate ).count();
}

int RecurrenceRule::durationTo( const QDate &date ) const
{
  return durationTo( KDateTime( date, QTime( 23, 59, 59 ), mDateStart.timeSpec() ) );
}

KDateTime RecurrenceRule::getPreviousDate( const KDateTime& afterDate ) const
{
  // Convert to the time spec used by this recurrence rule
  KDateTime toDate( afterDate.toTimeSpec( mDateStart.timeSpec() ) );
// kDebug(5800) << "         RecurrenceRule::getPreviousDate: " << dumpTime(afterDate) << " = " << dumpTime(toDate) << endl;
  // Invalid starting point, or beyond end of recurrence
  if ( !toDate.isValid() || toDate < startDt() )
    return KDateTime();

  // If we have a cache (duration given), use that
  if ( mDuration > 0 ) {
    if ( !mCached )
      buildCache();
    int i = mCachedDates.findLT( toDate );
    if ( i >= 0 ) {
      return mCachedDates[i];
    }
    return KDateTime();
  }

// kDebug(5800) << "    getPrev date before " << dumpTime(toDate) << endl;
  KDateTime prev = toDate;
  if ( mDuration >= 0 && endDt().isValid() && toDate > endDt() )
    prev = endDt().addSecs( 1 ).toTimeSpec( mDateStart.timeSpec() );

  Constraint interval( getPreviousValidDateInterval( prev, recurrenceType() ) );
// kDebug(5800) << "Previous Valid Date Interval for date " << dumpTime(prev) << ": " << endl;
// interval.dump();
  DateTimeList dts = datesForInterval( interval, recurrenceType() );
  int i = dts.findLT( prev );
  if ( i >= 0 ) {
    return ( dts[i] >= startDt() ) ? dts[i] : KDateTime();
  }

  // Previous interval. As soon as we find an occurrence, we're done.
  while ( interval.intervalDateTime( recurrenceType() ) > startDt() ) {
    interval.increase( recurrenceType(), -frequency() );
// kDebug(5800) << "Decreased interval: " << endl;
// interval.dump();
    // The returned date list is sorted
    DateTimeList dts = datesForInterval( interval, recurrenceType() );
    // The list is sorted, so take the last one.
    if ( !dts.isEmpty() ) {
      prev = dts.last();
      if ( prev.isValid() && prev >= startDt() ) return prev;
      else return KDateTime();
    }
  }
  return KDateTime();
}


KDateTime RecurrenceRule::getNextDate( const KDateTime &preDate ) const
{
  // Convert to the time spec used by this recurrence rule
  KDateTime fromDate( preDate.toTimeSpec( mDateStart.timeSpec() ) );
  // Beyond end of recurrence
  if ( mDuration >= 0 && endDt().isValid() && fromDate >= endDt() )
    return KDateTime();

  // Start date is only included if it really matches
  if ( fromDate < startDt() ) {
    fromDate = startDt().addSecs( -1 );
  }

  if ( mDuration > 0 ) {
    if ( !mCached )
      buildCache();
    int i = mCachedDates.findGT( fromDate );
    if ( i >= 0 ) {
//  kDebug(5800) << "    getNext date after " << fromDate << ", cached date: " << dumpTime(mCachedDates[i]) << endl;
      return mCachedDates[i];
    }
  }

// kDebug(5800) << "    getNext date after " << fromDate << endl;
  KDateTime end = endDt();
  Constraint interval( getNextValidDateInterval( fromDate, recurrenceType() ) );
  DateTimeList dts = datesForInterval( interval, recurrenceType() );
  int i = dts.findGT( fromDate );
  if ( i >= 0 ) {
    return ( mDuration < 0 || dts[i] <= end ) ? dts[i] : KDateTime();
  }
  interval.increase( recurrenceType(), frequency() );
  if ( mDuration >= 0 && interval.intervalDateTime( recurrenceType() ) > end ) {
    return KDateTime();
  }

  // Increase the interval. The first occurrence that we find is the result (if
  // if's before the end date).
    // TODO: some validity checks to avoid infinite loops for contradictory constraints
  int loop = 0;
  do {
    DateTimeList dts = datesForInterval( interval, recurrenceType() );
    if ( dts.count() > 0 ) {
      KDateTime ret( dts[0] );
      if ( mDuration >= 0 && ret > end ) return KDateTime();
      else return ret;
    }
    interval.increase( recurrenceType(), frequency() );
  } while ( ++loop < LOOP_LIMIT
        &&  ( mDuration < 0 || interval.intervalDateTime( recurrenceType() ) < end ));
  return KDateTime();
}

DateTimeList RecurrenceRule::timesInInterval( const KDateTime &start, const KDateTime &end ) const
{
// kDebug(5800) << "         RecurrenceRule::timesInInterval: " << dumpTime(start) << " - " << dumpTime(end) << endl;
  DateTimeList result;
  if ( end < startDt() )
    return result;    // before start of recurrence
  KDateTime enddt = end;
  if ( mDuration >= 0 ) {
    KDateTime endRecur = endDt();
    if ( endRecur.isValid() ) {
      if ( start >= endRecur )
        return result;    // beyond end of recurrence
      if ( end > endRecur )
        enddt = endRecur;    // limit end time to end of recurrence rule
    }
  }

  KDateTime st = start;
  bool done = false;
  if ( mDuration > 0 ) {
    if ( !mCached )
      buildCache();
    if ( mCachedDateEnd.isValid() && start >= mCachedDateEnd )
      return result;    // beyond end of recurrence
    int i = mCachedDates.findGE( start );
    if ( i >= 0 ) {
      int iend = mCachedDates.findGT( enddt, i );
      if ( iend < 0 )
        iend = mCachedDates.count();
      else
        done = true;
      while ( i < iend ) {
//  kDebug(5800) << "    cached date: " << dumpTime(mCachedDates[i] ) << endl;
        result += mCachedDates[i++];
      }
    }
    if ( mCachedDateEnd.isValid() )
      done = true;
    else if ( !result.isEmpty() ) {
      result += KDateTime();    // indicate that the returned list is incomplete
      done = true;
    }
    if (done)
      return result;
    // We don't have any result yet, but we reached the end of the incomplete cache
    st = mCachedLastDate.addSecs(1);
  }

// kDebug(5800) << "    getNext date after " << dumpTime(st) << endl;
  Constraint interval( getNextValidDateInterval( st, recurrenceType() ) );
  int loop = 0;
  do {
    DateTimeList dts = datesForInterval( interval, recurrenceType() );
    int i = 0;
    int iend = dts.count();
    if ( loop == 0 ) {
      i = dts.findGE( st );
      if ( i < 0 )
        i = iend;
    }
    int j = dts.findGT( enddt, i );
    if ( j >= 0 ) {
      iend = j;
      loop = LOOP_LIMIT;
    }
    while ( i < iend ) {
      result += dts[i++];
    }
    // Increase the interval.
    interval.increase( recurrenceType(), frequency() );
  } while ( ++loop < LOOP_LIMIT
        &&  interval.intervalDateTime( recurrenceType() ) < end );
  return result;
}


// Find the date/time of the occurrence at or before a date/time, for a given period type.
// Return a constraint whose value appropriate to 'type', is set to the value contained in the date/time.
RecurrenceRule::Constraint RecurrenceRule::getPreviousValidDateInterval( const KDateTime &dt, PeriodType type ) const
{
// kDebug(5800) << "       (o) getPreviousValidDateInterval before " << dumpTime(dt) << ", type=" << type << endl;
  long periods = 0;
  KDateTime start = startDt();
  KDateTime nextValid( start );
  int modifier = 1;
  KDateTime toDate( dt.toTimeSpec( start.timeSpec() ) );
  // for super-daily recurrences, don't care about the time part

  // Find the #intervals since the dtstart and round to the next multiple of
  // the frequency
  switch ( type ) {
    // Really fall through for sub-daily, since the calculations only differ
    // by the factor 60 and 60*60! Same for weekly and daily (factor 7)
    case rHourly:   modifier *= 60;
    case rMinutely: modifier *= 60;
    case rSecondly:
        periods = static_cast<int>( start.secsTo_long( toDate ) / modifier );
        // round it down to the next lower multiple of frequency():
        periods = ( periods / frequency() ) * frequency();
        nextValid = start.addSecs( modifier * periods );
        break;

    case rWeekly:
        toDate = toDate.addDays( -(7 + toDate.date().dayOfWeek() - mWeekStart) % 7 );
        start = start.addDays( -(7 + start.date().dayOfWeek() - mWeekStart) % 7 );
        modifier *= 7;
    case rDaily:
        periods = start.daysTo( toDate ) / modifier;
        // round it down to the next lower multiple of frequency():
        periods = ( periods / frequency() ) * frequency();
        nextValid = start.addDays( modifier * periods );
        break;

    case rMonthly: {
        periods = 12*( toDate.date().year() - start.date().year() ) +
             ( toDate.date().month() - start.date().month() );
        // round it down to the next lower multiple of frequency():
        periods = ( periods / frequency() ) * frequency();
        // set the day to the first day of the month, so we don't have problems
        // with non-existent days like Feb 30 or April 31
        start.setDate( QDate( start.date().year(), start.date().month(), 1 ) );
        nextValid.setDate( start.date().addMonths( periods ) );
        break; }
    case rYearly:
        periods = ( toDate.date().year() - start.date().year() );
        // round it down to the next lower multiple of frequency():
        periods = ( periods / frequency() ) * frequency();
        nextValid.setDate( start.date().addYears( periods ) );
        break;
    default:
        break;
  }
// kDebug(5800) << "    ~~~> date in previous interval is: : " << dumpTime(nextValid) << endl;

  return Constraint( nextValid, type, mWeekStart );
}

// Find the date/time of the next occurrence at or after a date/time, for a given period type.
// Return a constraint whose value appropriate to 'type', is set to the value contained in the date/time.
RecurrenceRule::Constraint RecurrenceRule::getNextValidDateInterval( const KDateTime &dt, PeriodType type ) const
{
  // TODO: Simplify this!
  long periods = 0;
  KDateTime start = startDt();
  KDateTime nextValid( start );
  int modifier = 1;
  KDateTime toDate( dt.toTimeSpec( start.timeSpec() ) );
  // for super-daily recurrences, don't care about the time part

  // Find the #intervals since the dtstart and round to the next multiple of
  // the frequency
  switch ( type ) {
    // Really fall through for sub-daily, since the calculations only differ
    // by the factor 60 and 60*60! Same for weekly and daily (factor 7)
    case rHourly:   modifier *= 60;
    case rMinutely: modifier *= 60;
    case rSecondly:
        periods = static_cast<int>( start.secsTo_long( toDate ) / modifier );
        periods = qMax( 0L, periods );
        if ( periods > 0 )
          periods += ( frequency() - 1 - ( (periods - 1) % frequency() ) );
        nextValid = start.addSecs( modifier * periods );
        break;

    case rWeekly:
        // correct both start date and current date to start of week
        toDate = toDate.addDays( -(7 + toDate.date().dayOfWeek() - mWeekStart) % 7 );
        start = start.addDays( -(7 + start.date().dayOfWeek() - mWeekStart) % 7 );
        modifier *= 7;
    case rDaily:
        periods = start.daysTo( toDate ) / modifier;
        periods = qMax( 0L, periods );
        if ( periods > 0 )
          periods += (frequency() - 1 - ( (periods - 1) % frequency() ) );
        nextValid = start.addDays( modifier * periods );
        break;

    case rMonthly: {
        periods = 12*( toDate.date().year() - start.date().year() ) +
             ( toDate.date().month() - start.date().month() );
        periods = qMax( 0L, periods);
        if ( periods > 0 )
          periods += (frequency() - 1 - ( (periods - 1) % frequency() ) );
        // set the day to the first day of the month, so we don't have problems
        // with non-existent days like Feb 30 or April 31
        start.setDate( QDate( start.date().year(), start.date().month(), 1 ) );
        nextValid.setDate( start.date().addMonths( periods ) );
        break; }
    case rYearly:
        periods = ( toDate.date().year() - start.date().year() );
        periods = qMax( 0L, periods);
        if ( periods > 0 )
          periods += ( frequency() - 1 - ( (periods - 1) % frequency() ) );
        nextValid.setDate( start.date().addYears( periods ) );
        break;
    default:
        break;
  }
//  kDebug(5800) << "    ~~~> date in next interval is: : " << dumpTime(nextValid) << endl;

  return Constraint( nextValid, type, mWeekStart );
}

bool RecurrenceRule::mergeIntervalConstraint( Constraint *merged,
          const Constraint &conit, const Constraint &interval ) const
{
  Constraint result( interval );

#define mergeConstraint( name, cmparison ) \
  if ( conit.name cmparison ) { \
    if ( !(result.name cmparison) || result.name == conit.name ) { \
      result.name = conit.name; \
    } else return false;\
  }

  mergeConstraint( year, > 0 );
  mergeConstraint( month, > 0 );
  mergeConstraint( day, != 0 );
  mergeConstraint( hour, >= 0 );
  mergeConstraint( minute, >= 0 );
  mergeConstraint( second, >= 0 );

  mergeConstraint( weekday, != 0 );
  mergeConstraint( weekdaynr, != 0 );
  mergeConstraint( weeknumber, != 0 );
  mergeConstraint( yearday, != 0 );

  #undef mergeConstraint
  if ( merged ) *merged = result;
  return true;
}


DateTimeList RecurrenceRule::datesForInterval( const Constraint &interval, PeriodType type ) const
{
  /* -) Loop through constraints,
     -) merge interval with each constraint
     -) if merged constraint is not consistent => ignore that constraint
     -) if complete => add that one date to the date list
     -) Loop through all missing fields => For each add the resulting
  */
// kDebug(5800) << "         RecurrenceRule::datesForInterval: " << endl;
// interval.dump();
  DateTimeList lst;
  for ( int i = 0, iend = mConstraints.count();  i < iend;  ++i ) {
    Constraint merged( mDateStart.timeSpec() );
    bool mergeok = mergeIntervalConstraint( &merged, mConstraints[i], interval );
    // If the information is incomplete, we can't use this constraint
    if ( merged.year <= 0 || merged.hour < 0 || merged.minute < 0 || merged.second < 0 )
      mergeok = false;
    if ( mergeok ) {
// kDebug(5800) << "      -) merged constraint: " << endl;
// merged.dump();
      // We have a valid constraint, so get all datetimes that match it andd
      // append it to all date/times of this interval
      QList<KDateTime> lstnew = merged.dateTimes( type );
      lst += lstnew;
    }
  }
  // Sort it so we can apply the BySetPos. Also some logic relies on this being sorted
  lst.sortUnique();


/*if ( lst.isEmpty() ) {
  kDebug(5800) << "         No Dates in Interval " << endl;
} else {
  kDebug(5800) << "         Dates: " << endl;
  for ( int i = 0, iend = lst.count();  i < iend;  ++i ) {
    kDebug(5800)<< "              -) " << dumpTime(lst[i]) << endl;
  }
  kDebug(5800) << "       ---------------------" << endl;
}*/
  if ( !mBySetPos.isEmpty() ) {
    DateTimeList tmplst = lst;
    lst.clear();
    for ( int i = 0, iend = mBySetPos.count();  i < iend;  ++i ) {
      int pos = mBySetPos[i];
      if ( pos > 0 ) --pos;
      if ( pos < 0 ) pos += tmplst.count();
      if ( pos >= 0 && pos < tmplst.count() ) {
        lst.append( tmplst[pos] );
      }
    }
    lst.sortUnique();
  }

  return lst;
}


void RecurrenceRule::dump() const
{
#ifndef NDEBUG
  kDebug(5800) << "RecurrenceRule::dump():" << endl;
  if ( !mRRule.isEmpty() )
    kDebug(5800) << "   RRULE=" << mRRule << endl;
  kDebug(5800) << "   Read-Only: " << isReadOnly() <<
                   ", dirty: " << mDirty << endl;

  kDebug(5800) << "   Period type: " << recurrenceType() << ", frequency: " << frequency() << endl;
  kDebug(5800) << "   #occurrences: " << duration() << endl;
  kDebug(5800) << "   start date: " << dumpTime(startDt()) << ", end date: " << dumpTime(endDt()) << endl;


#define dumpByIntList(list,label) \
  if ( !list.isEmpty() ) {\
    QStringList lst;\
    for ( int i = 0, iend = list.count();  i < iend;  ++i ) {\
      lst.append( QString::number( list[i] ) );\
    }\
    kDebug(5800) << "   " << label << lst.join(", ") << endl;\
  }
  dumpByIntList( mBySeconds,    "BySeconds:  " );
  dumpByIntList( mByMinutes,    "ByMinutes:  " );
  dumpByIntList( mByHours,      "ByHours:    " );
  if ( !mByDays.isEmpty() ) {
    QStringList lst;
    for ( int i = 0, iend = mByDays.count();  i < iend;  ++i ) {\
      lst.append( ( mByDays[i].pos() ? QString::number( mByDays[i].pos() ) : "" ) +
                   DateHelper::dayName( mByDays[i].day() ) );
    }
    kDebug(5800) << "   ByDays:     " << lst.join(", ") << endl;
  }
  dumpByIntList( mByMonthDays,  "ByMonthDays:" );
  dumpByIntList( mByYearDays,   "ByYearDays: " );
  dumpByIntList( mByWeekNumbers,"ByWeekNr:   " );
  dumpByIntList( mByMonths,     "ByMonths:   " );
  dumpByIntList( mBySetPos,     "BySetPos:   " );
  #undef dumpByIntList

  kDebug(5800) << "   Week start: " << DateHelper::dayName( mWeekStart ) << endl;

  kDebug(5800) << "   Constraints:" << endl;
  // dump constraints
  for ( int i = 0, iend = mConstraints.count();  i < iend;  ++i ) {
    mConstraints[i].dump();
  }
#endif
}

void RecurrenceRule::Constraint::dump() const
{
  kDebug(5800) << "     ~> Y="<<year<<", M="<<month<<", D="<<day<<", H="<<hour<<", m="<<minute<<", S="<<second<<", wd="<<weekday<<",#wd="<<weekdaynr<<", #w="<<weeknumber<<", yd="<<yearday<<endl;
}

QString dumpTime( const KDateTime &dt )
{
  if ( !dt.isValid() )
    return QString();
  QString result;
  if ( dt.isDateOnly() ) {
    result = dt.toString( "%a %Y-%m-%d %:Z" );
  } else {
    result = dt.toString( "%a %Y-%m-%d %H:%M:%S %:Z" );
    if ( dt.isSecondOccurrence() )
      result += QLatin1String(" (2nd)");
  }
  if ( dt.timeSpec() == KDateTime::Spec::ClockTime)
    result += QLatin1String("Clock");
  return result;
}
