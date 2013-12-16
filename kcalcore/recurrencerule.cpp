/*
  This file is part of the kcalcore library.

  Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofe.com>
  Copyright (c) 2006-2008 David Jarvie <djarvie@kde.org>

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

#include <KDebug>

#include <QtCore/QStringList>
#include <QtCore/QTime>

using namespace KCalCore;

// Maximum number of intervals to process
const int LOOP_LIMIT = 10000;

static QString dumpTime( const KDateTime &dt );   // for debugging

/*=========================================================================
=                                                                         =
= IMPORTANT CODING NOTE:                                                  =
=                                                                         =
= Recurrence handling code is time critical, especially for sub-daily     =
= recurrences. For example, if getNextDate() is called repeatedly to      =
= check all consecutive occurrences over a few years, on a slow machine   =
= this could take many seconds to complete in the worst case. Simple      =
= sub-daily recurrences are optimised by use of mTimedRepetition.         =
=                                                                         =
==========================================================================*/

/**************************************************************************
 *                               DateHelper                               *
 **************************************************************************/
//@cond PRIVATE
class DateHelper
{
  public:
#ifndef NDEBUG
    static QString dayName( short day );
#endif
    static QDate getNthWeek( int year, int weeknumber, short weekstart = 1 );
    static int weekNumbersInYear( int year, short weekstart = 1 );
    static int getWeekNumber( const QDate &date, short weekstart, int *year = 0 );
    static int getWeekNumberNeg( const QDate &date, short weekstart, int *year = 0 );
    // Convert to QDate, allowing for day < 0.
    // month and day must be non-zero.
    static QDate getDate( int year, int month, int day )
    {
      if ( day >= 0 ) {
        return QDate( year, month, day );
      } else {
        if ( ++month > 12 ) {
          month = 1;
          ++year;
        }
        return QDate( year, month, 1 ).addDays( day );
      }
    }
};

#ifndef NDEBUG
// TODO: Move to a general library / class, as we need the same in the iCal
//       generator and in the xcal format
QString DateHelper::dayName( short day )
{
  switch ( day ) {
  case 1:
    return QLatin1String("MO");
  case 2:
    return QLatin1String("TU");
  case 3:
    return QLatin1String("WE");
  case 4:
    return QLatin1String("TH");
  case 5:
    return QLatin1String("FR");
  case 6:
    return QLatin1String("SA");
  case 7:
    return QLatin1String("SU");
  default:
    return QLatin1String("??");
  }
}
#endif

QDate DateHelper::getNthWeek( int year, int weeknumber, short weekstart )
{
  if ( weeknumber == 0 ) {
    return QDate();
  }

  // Adjust this to the first day of week #1 of the year and add 7*weekno days.
  QDate dt( year, 1, 4 ); // Week #1 is the week that contains Jan 4
  int adjust = -( 7 + dt.dayOfWeek() - weekstart ) % 7;
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
  int y = date.year();
  QDate dt( y, 1, 4 ); // <= definitely in week #1
  dt = dt.addDays( -( 7 + dt.dayOfWeek() - weekstart ) % 7 ); // begin of week #1

  int daysto = dt.daysTo( date );
  if ( daysto < 0 ) {
    // in first week of year
    --y;
    dt = QDate( y, 1, 4 );
    dt = dt.addDays( -( 7 + dt.dayOfWeek() - weekstart ) % 7 ); // begin of week #1
    daysto = dt.daysTo( date );
  } else if ( daysto > 355 ) {
    // near the end of the year - check if it's next year
    QDate dtn( y+1, 1, 4 ); // <= definitely first week of next year
    dtn = dtn.addDays( -( 7 + dtn.dayOfWeek() - weekstart ) % 7 );
    int dayston = dtn.daysTo( date );
    if ( dayston >= 0 ) {
      // in first week of next year;
      ++y;
      daysto = dayston;
    }
  }
  if ( year ) {
    *year = y;
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
//@endcond

/**************************************************************************
 *                               WDayPos                                  *
 **************************************************************************/

bool RecurrenceRule::WDayPos::operator==( const RecurrenceRule::WDayPos &pos2 ) const
{
  return mDay == pos2.mDay && mPos == pos2.mPos;
}

bool RecurrenceRule::WDayPos::operator!=( const RecurrenceRule::WDayPos &pos2 ) const
{
  return !operator==( pos2 );
}

/**************************************************************************
 *                               Constraint                               *
 **************************************************************************/
//@cond PRIVATE
class Constraint
{
  public:
    typedef QList<Constraint> List;

    Constraint() {}
    explicit Constraint( KDateTime::Spec, int wkst = 1 );
    Constraint( const KDateTime &dt, RecurrenceRule::PeriodType type, int wkst );
    void clear();
    void setYear( int n )
    {
      year = n;
      useCachedDt = false;
    }
    void setMonth( int n )
    {
      month = n;
      useCachedDt = false;
    }
    void setDay( int n )
    {
      day = n;
      useCachedDt = false;
    }
    void setHour( int n )
    {
      hour = n;
      useCachedDt = false;
    }
    void setMinute( int n )
    {
      minute = n;
      useCachedDt = false;
    }
    void setSecond( int n )
    {
      second = n;
      useCachedDt = false;
    }
    void setWeekday( int n )
    {
      weekday = n;
      useCachedDt = false;
    }
    void setWeekdaynr( int n )
    {
      weekdaynr = n;
      useCachedDt = false;
    }
    void setWeeknumber( int n )
    {
      weeknumber = n;
      useCachedDt = false;
    }
    void setYearday( int n )
    {
      yearday = n;
      useCachedDt = false;
    }
    void setWeekstart( int n )
    {
      weekstart = n;
      useCachedDt = false;
    }
    void setSecondOccurrence( int n )
    {
      secondOccurrence = n;
      useCachedDt = false;
    }

    int year;       // 0 means unspecified
    int month;      // 0 means unspecified
    int day;        // 0 means unspecified
    int hour;       // -1 means unspecified
    int minute;     // -1 means unspecified
    int second;     // -1 means unspecified
    int weekday;    //  0 means unspecified
    int weekdaynr;  // index of weekday in month/year (0=unspecified)
    int weeknumber; //  0 means unspecified
    int yearday;    //  0 means unspecified
    int weekstart;  //  first day of week (1=monday, 7=sunday, 0=unspec.)
    KDateTime::Spec timespec;   // time zone etc. to use
    bool secondOccurrence;  // the time is the second occurrence during daylight savings shift

    bool readDateTime( const KDateTime &dt, RecurrenceRule::PeriodType type );
    bool matches( const QDate &dt, RecurrenceRule::PeriodType type ) const;
    bool matches( const KDateTime &dt, RecurrenceRule::PeriodType type ) const;
    bool merge( const Constraint &interval );
    bool isConsistent() const;
    bool isConsistent( RecurrenceRule::PeriodType period ) const;
    bool increase( RecurrenceRule::PeriodType type, int freq );
    KDateTime intervalDateTime( RecurrenceRule::PeriodType type ) const;
    QList<KDateTime> dateTimes( RecurrenceRule::PeriodType type ) const;
    void appendDateTime( const QDate &date, const QTime &time, QList<KDateTime> &list ) const;
    void dump() const;

  private:
    mutable bool useCachedDt;
    mutable KDateTime cachedDt;
};

Constraint::Constraint( KDateTime::Spec spec, int wkst )
  : weekstart( wkst ),
    timespec( spec )
{
  clear();
}

Constraint::Constraint( const KDateTime &dt, RecurrenceRule::PeriodType type, int wkst )
  : weekstart( wkst ),
    timespec( dt.timeSpec() )
{
  clear();
  readDateTime( dt, type );
}

void Constraint::clear()
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
  useCachedDt = false;
}

bool Constraint::matches( const QDate &dt, RecurrenceRule::PeriodType type ) const
{
  // If the event recurs in week 53 or 1, the day might not belong to the same
  // year as the week it is in. E.g. Jan 1, 2005 is in week 53 of year 2004.
  // So we can't simply check the year in that case!
  if ( weeknumber == 0 ) {
    if ( year > 0 && year != dt.year() ) {
      return false;
    }
  } else {
    int y;
    if ( weeknumber > 0 &&
         weeknumber != DateHelper::getWeekNumber( dt, weekstart, &y ) ) {
      return false;
    }
    if ( weeknumber < 0 &&
         weeknumber != DateHelper::getWeekNumberNeg( dt, weekstart, &y ) ) {
      return false;
    }
    if ( year > 0 && year != y ) {
      return false;
    }
  }

  if ( month > 0 && month != dt.month() ) {
    return false;
  }
  if ( day > 0 && day != dt.day() ) {
    return false;
  }
  if ( day < 0 && dt.day() != ( dt.daysInMonth() + day + 1 ) ) {
    return false;
  }
  if ( weekday > 0 ) {
    if ( weekday != dt.dayOfWeek() ) {
      return false;
    }
    if ( weekdaynr != 0 ) {
      // If it's a yearly recurrence and a month is given, the position is
      // still in the month, not in the year.
      if ( ( type == RecurrenceRule::rMonthly ) ||
           ( type == RecurrenceRule::rYearly && month > 0 ) ) {
        // Monthly
        if ( weekdaynr > 0 &&
             weekdaynr != ( dt.day() - 1 ) / 7 + 1 ) {
          return false;
        }
        if ( weekdaynr < 0 &&
             weekdaynr != -( ( dt.daysInMonth() - dt.day() ) / 7 + 1 ) ) {
          return false;
        }
      } else {
        // Yearly
        if ( weekdaynr > 0 &&
             weekdaynr != ( dt.dayOfYear() - 1 ) / 7 + 1 ) {
          return false;
        }
        if ( weekdaynr < 0 &&
             weekdaynr != -( ( dt.daysInYear() - dt.dayOfYear() ) / 7 + 1 ) ) {
          return false;
        }
      }
    }
  }
  if ( yearday > 0 && yearday != dt.dayOfYear() ) {
    return false;
  }
  if ( yearday < 0 && yearday != dt.daysInYear() - dt.dayOfYear() + 1 ) {
    return false;
  }
  return true;
}

/* Check for a match with the specified date/time.
 * The date/time's time specification must correspond with that of the start date/time.
 */
bool Constraint::matches( const KDateTime &dt, RecurrenceRule::PeriodType type ) const
{
  if ( ( hour >= 0 && ( hour != dt.time().hour() ||
                        secondOccurrence != dt.isSecondOccurrence() ) ) ||
       ( minute >= 0 && minute != dt.time().minute() ) ||
       ( second >= 0 && second != dt.time().second() ) ||
       !matches( dt.date(), type ) ) {
    return false;
  }
  return true;
}

bool Constraint::isConsistent( RecurrenceRule::PeriodType /*period*/) const
{
  // TODO: Check for consistency, e.g. byyearday=3 and bymonth=10
  return true;
}

// Return a date/time set to the constraint values, but with those parts less
// significant than the given period type set to 1 (for dates) or 0 (for times).
KDateTime Constraint::intervalDateTime( RecurrenceRule::PeriodType type ) const
{
  if ( useCachedDt ) {
    return cachedDt;
  }
  QDate d;
  QTime t( 0, 0, 0 );
  bool subdaily = true;
  switch ( type ) {
    case RecurrenceRule::rSecondly:
      t.setHMS( hour, minute, second );
      break;
    case RecurrenceRule::rMinutely:
      t.setHMS( hour, minute, 0 );
      break;
    case RecurrenceRule::rHourly:
      t.setHMS( hour, 0, 0 );
      break;
    case RecurrenceRule::rDaily:
      break;
    case RecurrenceRule::rWeekly:
      d = DateHelper::getNthWeek( year, weeknumber, weekstart );
      subdaily = false;
      break;
    case RecurrenceRule::rMonthly:
      d.setYMD( year, month, 1 );
      subdaily = false;
      break;
    case RecurrenceRule::rYearly:
      d.setYMD( year, 1, 1 );
      subdaily = false;
      break;
    default:
      break;
  }
  if ( subdaily ) {
    d = DateHelper::getDate( year, (month>0)?month:1, day?day:1 );
  }
  cachedDt = KDateTime( d, t, timespec );
  if ( secondOccurrence ) {
    cachedDt.setSecondOccurrence( true );
  }
  useCachedDt = true;
  return cachedDt;
}

bool Constraint::merge( const Constraint &interval )
{
#define mergeConstraint( name, cmparison ) \
  if ( interval.name cmparison ) { \
    if ( !( name cmparison ) ) { \
      name = interval.name; \
    } else if ( name != interval.name ) { \
      return false;\
    } \
  }

  useCachedDt = false;

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
  return true;
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

QList<KDateTime> Constraint::dateTimes( RecurrenceRule::PeriodType type ) const
{
  QList<KDateTime> result;
  bool done = false;
  if ( !isConsistent( type ) ) {
    return result;
  }

  // TODO_Recurrence: Handle all-day
  QTime tm( hour, minute, second );

  if ( !done && day && month > 0 ) {
    appendDateTime( DateHelper::getDate( year, month, day ), tm, result );
    done = true;
  }

  if ( !done && weekday == 0 && weeknumber == 0 && yearday == 0 ) {
    // Easy case: date is given, not restrictions by week or yearday
    uint mstart = ( month > 0 ) ? month : 1;
    uint mend = ( month <= 0 ) ? 12 : month;
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
      uint d = dstart;
      for ( QDate dt( year, m, dstart ); ; dt = dt.addDays( 1 ) ) {
        appendDateTime( dt, tm, result );
        if ( ++d > dend ) {
          break;
        }
      }
    }
    done = true;
  }

  // Else: At least one of the week / yearday restrictions was given...
  // If we have a yearday (and of course a year), we know the exact date
  if ( !done && yearday != 0 ) {
    // yearday < 0 means from end of year, so we'll need Jan 1 of the next year
    QDate d( year + ( ( yearday > 0 ) ? 0 : 1 ), 1, 1 );
    d = d.addDays( yearday - ( ( yearday > 0 ) ? 1 : 0 ) );
    appendDateTime( d, tm, result );
    done = true;
  }

  // Else: If we have a weeknumber, we have at most 7 possible dates, loop through them
  if ( !done && weeknumber != 0 ) {
    QDate wst( DateHelper::getNthWeek( year, weeknumber, weekstart ) );
    if ( weekday != 0 ) {
      wst = wst.addDays( ( 7 + weekday - weekstart ) % 7 );
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
    bool inMonth = ( type == RecurrenceRule::rMonthly ) ||
                   ( type == RecurrenceRule::rYearly && month > 0 );
    if ( inMonth && month > 0 ) {
      dt = QDate( year, month, 1 );
      maxloop = 5;
    }
    if ( weekdaynr < 0 ) {
      // From end of period (month, year) => relative to begin of next period
      if ( inMonth ) {
        dt = dt.addMonths( 1 );
      } else {
        dt = dt.addYears( 1 );
      }
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
    if ( matches( result[i], type ) ) {
      valid.append( result[i] );
    }
  }
  // Don't sort it here, would be unnecessary work. The results from all
  // constraints will be merged to one big list of the interval. Sort that one!
  return valid;
}

void Constraint::appendDateTime( const QDate &date, const QTime &time,
                                 QList<KDateTime> &list ) const
{
  KDateTime dt( date, time, timespec );
  if ( dt.isValid() ) {
    if ( secondOccurrence ) {
      dt.setSecondOccurrence( true );
    }
    list.append( dt );
  }
}

bool Constraint::increase( RecurrenceRule::PeriodType type, int freq )
{
  // convert the first day of the interval to KDateTime
  intervalDateTime( type );

  // Now add the intervals
  switch ( type ) {
    case RecurrenceRule::rSecondly:
      cachedDt = cachedDt.addSecs( freq );
      break;
    case RecurrenceRule::rMinutely:
      cachedDt = cachedDt.addSecs( 60 * freq );
      break;
    case RecurrenceRule::rHourly:
      cachedDt = cachedDt.addSecs( 3600 * freq );
      break;
    case RecurrenceRule::rDaily:
      cachedDt = cachedDt.addDays( freq );
      break;
    case RecurrenceRule::rWeekly:
      cachedDt = cachedDt.addDays( 7 * freq );
      break;
    case RecurrenceRule::rMonthly:
      cachedDt = cachedDt.addMonths( freq );
      break;
    case RecurrenceRule::rYearly:
      cachedDt = cachedDt.addYears( freq );
      break;
    default:
      break;
  }
  // Convert back from KDateTime to the Constraint class
  readDateTime( cachedDt, type );
  useCachedDt = true;   // readDateTime() resets this

  return true;
}

// Set the constraint's value appropriate to 'type', to the value contained in a date/time.
bool Constraint::readDateTime( const KDateTime &dt, RecurrenceRule::PeriodType type )
{
  switch ( type ) {
    // Really fall through! Only weekly needs to be treated differently!
  case RecurrenceRule::rSecondly:
    second = dt.time().second();
  case RecurrenceRule::rMinutely:
    minute = dt.time().minute();
  case RecurrenceRule::rHourly:
    hour = dt.time().hour();
    secondOccurrence = dt.isSecondOccurrence();
  case RecurrenceRule::rDaily:
    day = dt.date().day();
  case RecurrenceRule::rMonthly:
    month = dt.date().month();
  case RecurrenceRule::rYearly:
    year = dt.date().year();
    break;
  case RecurrenceRule::rWeekly:
    // Determine start day of the current week, calculate the week number from that
    weeknumber = DateHelper::getWeekNumber( dt.date(), weekstart, &year );
    break;
  default:
    break;
  }
  useCachedDt = false;
  return true;
}
//@endcond

/**************************************************************************
 *                        RecurrenceRule::Private                         *
 **************************************************************************/

//@cond PRIVATE
class KCalCore::RecurrenceRule::Private
{
  public:
    Private( RecurrenceRule *parent )
      : mParent( parent ),
        mPeriod( rNone ),
        mFrequency( 0 ),
        mDuration( -1 ),
        mWeekStart( 1 ),
        mIsReadOnly( false ),
        mAllDay( false )
    {
        setDirty();
    }

    Private( RecurrenceRule *parent, const Private &p );

    Private &operator=( const Private &other );
    bool operator==( const Private &other ) const;
    void clear();
    void setDirty();
    void buildConstraints();
    bool buildCache() const;
    Constraint getNextValidDateInterval( const KDateTime &preDate, PeriodType type ) const;
    Constraint getPreviousValidDateInterval( const KDateTime &afterDate, PeriodType type ) const;
    DateTimeList datesForInterval( const Constraint &interval, PeriodType type ) const;

    RecurrenceRule *mParent;
    QString mRRule;            // RRULE string
    PeriodType mPeriod;
    KDateTime mDateStart;      // start of recurrence (but mDateStart is not an occurrence
                               // unless it matches the rule)
    uint mFrequency;
    /** how often it recurs:
           < 0 means no end date,
           0 means an explicit end date,
           positive values give the number of occurrences */
    int mDuration;
    KDateTime mDateEnd;

    QList<int> mBySeconds;     // values: second 0-59
    QList<int> mByMinutes;     // values: minute 0-59
    QList<int> mByHours;       // values: hour 0-23

    QList<WDayPos> mByDays;   // n-th weekday of the month or year
    QList<int> mByMonthDays;   // values: day -31 to -1 and 1-31
    QList<int> mByYearDays;    // values: day -366 to -1 and 1-366
    QList<int> mByWeekNumbers; // values: week -53 to -1 and 1-53
    QList<int> mByMonths;      // values: month 1-12
    QList<int> mBySetPos;      // values: position -366 to -1 and 1-366
    short mWeekStart;               // first day of the week (1=Monday, 7=Sunday)

    Constraint::List mConstraints;
    QList<RuleObserver*> mObservers;

    // Cache for duration
    mutable DateTimeList mCachedDates;
    mutable KDateTime mCachedDateEnd;
    mutable KDateTime mCachedLastDate;   // when mCachedDateEnd invalid, last date checked
    mutable bool mCached;

    bool mIsReadOnly;
    bool mAllDay;
    bool mNoByRules;        // no BySeconds, ByMinutes, ... rules exist
    uint mTimedRepetition;  // repeats at a regular number of seconds interval, or 0
};

RecurrenceRule::Private::Private( RecurrenceRule *parent, const Private &p )
  : mParent( parent ),
    mRRule( p.mRRule ),
    mPeriod( p.mPeriod ),
    mDateStart( p.mDateStart ),
    mFrequency( p.mFrequency ),
    mDuration( p.mDuration ),
    mDateEnd( p.mDateEnd ),

    mBySeconds( p.mBySeconds ),
    mByMinutes( p.mByMinutes ),
    mByHours( p.mByHours ),
    mByDays( p.mByDays ),
    mByMonthDays( p.mByMonthDays ),
    mByYearDays( p.mByYearDays ),
    mByWeekNumbers( p.mByWeekNumbers ),
    mByMonths( p.mByMonths ),
    mBySetPos( p.mBySetPos ),
    mWeekStart( p.mWeekStart ),

    mIsReadOnly( p.mIsReadOnly ),
    mAllDay( p.mAllDay ),
    mNoByRules( p.mNoByRules )
{
    setDirty();
}

RecurrenceRule::Private &RecurrenceRule::Private::operator=( const Private &p )
{
  // check for self assignment
  if ( &p == this ) {
    return *this;
  }

  mRRule = p.mRRule;
  mPeriod = p.mPeriod;
  mDateStart = p.mDateStart;
  mFrequency = p.mFrequency;
  mDuration = p.mDuration;
  mDateEnd = p.mDateEnd;

  mBySeconds = p.mBySeconds;
  mByMinutes = p.mByMinutes;
  mByHours = p.mByHours;
  mByDays = p.mByDays;
  mByMonthDays = p.mByMonthDays;
  mByYearDays = p.mByYearDays;
  mByWeekNumbers = p.mByWeekNumbers;
  mByMonths = p.mByMonths;
  mBySetPos = p.mBySetPos;
  mWeekStart = p.mWeekStart;

  mIsReadOnly = p.mIsReadOnly;
  mAllDay = p.mAllDay;
  mNoByRules = p.mNoByRules;

  setDirty();

  return *this;
}

bool RecurrenceRule::Private::operator==( const Private &r ) const
{
  return
    mPeriod == r.mPeriod &&
    ( ( mDateStart == r.mDateStart ) ||
      ( !mDateStart.isValid() && !r.mDateStart.isValid() ) ) &&
    mDuration == r.mDuration &&
    ( ( mDateEnd == r.mDateEnd ) ||
      ( !mDateEnd.isValid() && !r.mDateEnd.isValid() ) ) &&
    mFrequency == r.mFrequency &&
    mIsReadOnly == r.mIsReadOnly &&
    mAllDay == r.mAllDay &&
    mBySeconds == r.mBySeconds &&
    mByMinutes == r.mByMinutes &&
    mByHours == r.mByHours &&
    mByDays == r.mByDays &&
    mByMonthDays == r.mByMonthDays &&
    mByYearDays == r.mByYearDays &&
    mByWeekNumbers == r.mByWeekNumbers &&
    mByMonths == r.mByMonths &&
    mBySetPos == r.mBySetPos &&
    mWeekStart == r.mWeekStart &&
    mNoByRules == r.mNoByRules;
}

void RecurrenceRule::Private::clear()
{
  if ( mIsReadOnly ) {
    return;
  }
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
  mNoByRules = false;

  setDirty();
}

void RecurrenceRule::Private::setDirty()
{
  buildConstraints();
  mCached = false;
  mCachedDates.clear();
  for ( int i = 0, iend = mObservers.count();  i < iend;  ++i ) {
    if ( mObservers[i] ) {
      mObservers[i]->recurrenceChanged( mParent );
    }
  }
}
//@endcond

/**************************************************************************
 *                              RecurrenceRule                            *
 **************************************************************************/

RecurrenceRule::RecurrenceRule()
  : d( new Private( this ) )
{
}

RecurrenceRule::RecurrenceRule( const RecurrenceRule &r )
  : d( new Private( this, *r.d ) )
{
}

RecurrenceRule::~RecurrenceRule()
{
  delete d;
}

bool RecurrenceRule::operator==( const RecurrenceRule &r ) const
{
  return *d == *r.d;
}

RecurrenceRule &RecurrenceRule::operator=( const RecurrenceRule &r )
{
  // check for self assignment
  if ( &r == this ) {
    return *this;
  }

  *d = *r.d;

  return *this;
}

void RecurrenceRule::addObserver( RuleObserver *observer )
{
  if ( !d->mObservers.contains( observer ) ) {
    d->mObservers.append( observer );
  }
}

void RecurrenceRule::removeObserver( RuleObserver *observer )
{
  if ( d->mObservers.contains( observer ) ) {
    d->mObservers.removeAll( observer );
  }
}

void RecurrenceRule::setRecurrenceType( PeriodType period )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mPeriod = period;
  d->setDirty();
}

KDateTime RecurrenceRule::endDt( bool *result ) const
{
  if ( result ) {
    *result = false;
  }
  if ( d->mPeriod == rNone ) {
    return KDateTime();
  }
  if ( d->mDuration < 0 ) {
    return KDateTime();
  }
  if ( d->mDuration == 0 ) {
    if ( result ) {
      *result = true;
    }
    return d->mDateEnd;
  }

  // N occurrences. Check if we have a full cache. If so, return the cached end date.
  if ( !d->mCached ) {
    // If not enough occurrences can be found (i.e. inconsistent constraints)
    if ( !d->buildCache() ) {
      return KDateTime();
    }
  }
  if ( result ) {
    *result = true;
  }
  return d->mCachedDateEnd;
}

void RecurrenceRule::setEndDt( const KDateTime &dateTime )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mDateEnd = dateTime;
  d->mDuration = 0; // set to 0 because there is an end date/time
  d->setDirty();
}

void RecurrenceRule::setDuration( int duration )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mDuration = duration;
  d->setDirty();
}

void RecurrenceRule::setAllDay( bool allDay )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mAllDay = allDay;
  d->setDirty();
}

void RecurrenceRule::clear()
{
  d->clear();
}

void RecurrenceRule::setDirty()
{
  d->setDirty();
}

void RecurrenceRule::setStartDt( const KDateTime &start )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mDateStart = start;
  d->setDirty();
}

void RecurrenceRule::setFrequency( int freq )
{
  if ( isReadOnly() || freq <= 0 ) {
    return;
  }
  d->mFrequency = freq;
  d->setDirty();
}

void RecurrenceRule::setBySeconds( const QList<int> &bySeconds )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mBySeconds = bySeconds;
  d->setDirty();
}

void RecurrenceRule::setByMinutes( const QList<int> &byMinutes )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mByMinutes = byMinutes;
  d->setDirty();
}

void RecurrenceRule::setByHours( const QList<int> &byHours )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mByHours = byHours;
  d->setDirty();
}

void RecurrenceRule::setByDays( const QList<WDayPos> &byDays )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mByDays = byDays;
  d->setDirty();
}

void RecurrenceRule::setByMonthDays( const QList<int> &byMonthDays )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mByMonthDays = byMonthDays;
  d->setDirty();
}

void RecurrenceRule::setByYearDays( const QList<int> &byYearDays )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mByYearDays = byYearDays;
  d->setDirty();
}

void RecurrenceRule::setByWeekNumbers( const QList<int> &byWeekNumbers )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mByWeekNumbers = byWeekNumbers;
  d->setDirty();
}

void RecurrenceRule::setByMonths( const QList<int> &byMonths )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mByMonths = byMonths;
  d->setDirty();
}

void RecurrenceRule::setBySetPos( const QList<int> &bySetPos )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mBySetPos = bySetPos;
  d->setDirty();
}

void RecurrenceRule::setWeekStart( short weekStart )
{
  if ( isReadOnly() ) {
    return;
  }
  d->mWeekStart = weekStart;
  d->setDirty();
}

void RecurrenceRule::shiftTimes( const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec )
{
  d->mDateStart = d->mDateStart.toTimeSpec( oldSpec );
  d->mDateStart.setTimeSpec( newSpec );
  if ( d->mDuration == 0 ) {
    d->mDateEnd = d->mDateEnd.toTimeSpec( oldSpec );
    d->mDateEnd.setTimeSpec( newSpec );
  }
  d->setDirty();
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

//@cond PRIVATE
void RecurrenceRule::Private::buildConstraints()
{
  mTimedRepetition = 0;
  mNoByRules = mBySetPos.isEmpty();
  mConstraints.clear();
  Constraint con( mDateStart.timeSpec() );
  if ( mWeekStart > 0 ) {
    con.setWeekstart( mWeekStart );
  }
  mConstraints.append( con );

  int c, cend;
  int i, iend;
  Constraint::List tmp;

  #define intConstraint( list, setElement ) \
  if ( !list.isEmpty() ) { \
    mNoByRules = false; \
    iend = list.count(); \
    if ( iend == 1 ) { \
      for ( c = 0, cend = mConstraints.count();  c < cend;  ++c ) { \
        mConstraints[c].setElement( list[0] ); \
      } \
    } else { \
      for ( c = 0, cend = mConstraints.count();  c < cend;  ++c ) { \
        for ( i = 0;  i < iend;  ++i ) { \
          con = mConstraints[c]; \
          con.setElement( list[i] ); \
          tmp.append( con ); \
        } \
      } \
      mConstraints = tmp; \
      tmp.clear(); \
    } \
  }

  intConstraint( mBySeconds, setSecond );
  intConstraint( mByMinutes, setMinute );
  intConstraint( mByHours, setHour );
  intConstraint( mByMonthDays, setDay );
  intConstraint( mByMonths, setMonth );
  intConstraint( mByYearDays, setYearday );
  intConstraint( mByWeekNumbers, setWeeknumber );
  #undef intConstraint

  if ( !mByDays.isEmpty() ) {
    mNoByRules = false;
    for ( c = 0, cend = mConstraints.count();  c < cend;  ++c ) {
      for ( i = 0, iend = mByDays.count();  i < iend;  ++i ) {
        con = mConstraints[c];
        con.setWeekday( mByDays[i].day() );
        con.setWeekdaynr( mByDays[i].pos() );
        tmp.append( con );
      }
    }
    mConstraints = tmp;
    tmp.clear();
  }

  #define fixConstraint( setElement, value ) \
  { \
    for ( c = 0, cend = mConstraints.count();  c < cend;  ++c ) { \
      mConstraints[c].setElement( value );                        \
    } \
  }
  // Now determine missing values from DTSTART. This can speed up things,
  // because we have more restrictions and save some loops.

  // TODO: Does RFC 2445 intend to restrict the weekday in all cases of weekly?
  if ( mPeriod == rWeekly && mByDays.isEmpty() ) {
    fixConstraint( setWeekday, mDateStart.date().dayOfWeek() );
  }

  // Really fall through in the cases, because all smaller time intervals are
  // constrained from dtstart
  switch ( mPeriod ) {
  case rYearly:
    if ( mByDays.isEmpty() && mByWeekNumbers.isEmpty() &&
         mByYearDays.isEmpty() && mByMonths.isEmpty() ) {
      fixConstraint( setMonth, mDateStart.date().month() );
    }
  case rMonthly:
    if ( mByDays.isEmpty() && mByWeekNumbers.isEmpty() &&
         mByYearDays.isEmpty() && mByMonthDays.isEmpty() ) {
      fixConstraint( setDay, mDateStart.date().day() );
    }
  case rWeekly:
  case rDaily:
    if ( mByHours.isEmpty() ) {
      fixConstraint( setHour, mDateStart.time().hour() );
    }
  case rHourly:
    if ( mByMinutes.isEmpty() ) {
      fixConstraint( setMinute, mDateStart.time().minute() );
    }
  case rMinutely:
    if ( mBySeconds.isEmpty() ) {
      fixConstraint( setSecond, mDateStart.time().second() );
    }
  case rSecondly:
  default:
    break;
  }
  #undef fixConstraint

  if ( mNoByRules ) {
    switch ( mPeriod ) {
      case rHourly:
        mTimedRepetition = mFrequency * 3600;
        break;
      case rMinutely:
        mTimedRepetition = mFrequency * 60;
        break;
      case rSecondly:
        mTimedRepetition = mFrequency;
        break;
      default:
        break;
    }
  } else {
    for ( c = 0, cend = mConstraints.count(); c < cend; ) {
      if ( mConstraints[c].isConsistent( mPeriod ) ) {
        ++c;
      } else {
        mConstraints.removeAt( c );
        --cend;
      }
    }
  }
}

// Build and cache a list of all occurrences.
// Only call buildCache() if mDuration > 0.
bool RecurrenceRule::Private::buildCache() const
{
  Q_ASSERT( mDuration > 0 );
  // Build the list of all occurrences of this event (we need that to determine
  // the end date!)
  Constraint interval( getNextValidDateInterval( mDateStart, mPeriod ) );
  QDateTime next;

  DateTimeList dts = datesForInterval( interval, mPeriod );
  // Only use dates after the event has started (start date is only included
  // if it matches)
  int i = dts.findLT( mDateStart );
  if ( i >= 0 ) {
    dts.erase( dts.begin(), dts.begin() + i + 1 );
  }

  // some validity checks to avoid infinite loops (i.e. if we have
  // done this loop already 10000 times, bail out )
  for ( int loopnr = 0; loopnr < LOOP_LIMIT && dts.count() < mDuration; ++loopnr ) {
    interval.increase( mPeriod, mFrequency );
    // The returned date list is already sorted!
    dts += datesForInterval( interval, mPeriod );
  }
  if ( dts.count() > mDuration ) {
    // we have picked up more occurrences than necessary, remove them
    dts.erase( dts.begin() + mDuration, dts.end() );
  }
  mCached = true;
  mCachedDates = dts;

// it = dts.begin();
// while ( it != dts.end() ) {
//   kDebug() << "            -=>" << dumpTime(*it);
//   ++it;
// }
  if ( int( dts.count() ) == mDuration ) {
    mCachedDateEnd = dts.last();
    return true;
  } else {
    // The cached date list is incomplete
    mCachedDateEnd = KDateTime();
    mCachedLastDate = interval.intervalDateTime( mPeriod );
    return false;
  }
}
//@endcond

bool RecurrenceRule::dateMatchesRules( const KDateTime &kdt ) const
{
  KDateTime dt = kdt.toTimeSpec( d->mDateStart.timeSpec() );
  for ( int i = 0, iend = d->mConstraints.count();  i < iend;  ++i ) {
    if ( d->mConstraints[i].matches( dt, recurrenceType() ) ) {
      return true;
    }
  }
  return false;
}

bool RecurrenceRule::recursOn( const QDate &qd, const KDateTime::Spec &timeSpec ) const
{
  int i, iend;

  if ( !qd.isValid() || !d->mDateStart.isValid() ) {
    // There can't be recurrences on invalid dates
    return false;
  }

  if ( allDay() ) {
    // It's a date-only rule, so it has no time specification.
    // Therefore ignore 'timeSpec'.
    if ( qd < d->mDateStart.date() ) {
      return false;
    }
    // Start date is only included if it really matches
    QDate endDate;
    if ( d->mDuration >= 0 ) {
      endDate =  endDt().date();
      if ( qd > endDate ) {
        return false;
      }
    }

    // The date must be in an appropriate interval (getNextValidDateInterval),
    // Plus it must match at least one of the constraints
    bool match = false;
    for ( i = 0, iend = d->mConstraints.count();  i < iend && !match;  ++i ) {
      match = d->mConstraints[i].matches( qd, recurrenceType() );
    }
    if ( !match ) {
      return false;
    }

    KDateTime start( qd, QTime( 0, 0, 0 ), d->mDateStart.timeSpec() );
    Constraint interval( d->getNextValidDateInterval( start, recurrenceType() ) );
    // Constraint::matches is quite efficient, so first check if it can occur at
    // all before we calculate all actual dates.
    if ( !interval.matches( qd, recurrenceType() ) ) {
      return false;
    }
    // We really need to obtain the list of dates in this interval, since
    // otherwise BYSETPOS will not work (i.e. the date will match the interval,
    // but BYSETPOS selects only one of these matching dates!
    KDateTime end = start.addDays( 1 );
    do {
      DateTimeList dts = d->datesForInterval( interval, recurrenceType() );
      for ( i = 0, iend = dts.count();  i < iend;  ++i ) {
        if ( dts[i].date() >= qd ) {
          return dts[i].date() == qd;
        }
      }
      interval.increase( recurrenceType(), frequency() );
    } while ( interval.intervalDateTime( recurrenceType() ) < end );
    return false;
  }

  // It's a date-time rule, so we need to take the time specification into account.
  KDateTime start( qd, QTime( 0, 0, 0 ), timeSpec );
  KDateTime end = start.addDays( 1 ).toTimeSpec( d->mDateStart.timeSpec() );
  start = start.toTimeSpec( d->mDateStart.timeSpec() );
  if ( end < d->mDateStart ) {
    return false;
  }
  if ( start < d->mDateStart ) {
    start = d->mDateStart;
  }

  // Start date is only included if it really matches
  if ( d->mDuration >= 0 ) {
    KDateTime endRecur = endDt();
    if ( endRecur.isValid() ) {
      if ( start > endRecur ) {
        return false;
      }
      if ( end > endRecur ) {
        end = endRecur;    // limit end-of-day time to end of recurrence rule
      }
    }
  }

  if ( d->mTimedRepetition ) {
    // It's a simple sub-daily recurrence with no constraints
    int n = static_cast<int>( ( d->mDateStart.secsTo_long( start ) - 1 ) % d->mTimedRepetition );
    return start.addSecs( d->mTimedRepetition - n ) < end;
  }

  // Find the start and end dates in the time spec for the rule
  QDate startDay = start.date();
  QDate endDay = end.addSecs( -1 ).date();
  int dayCount = startDay.daysTo( endDay ) + 1;

  // The date must be in an appropriate interval (getNextValidDateInterval),
  // Plus it must match at least one of the constraints
  bool match = false;
  for ( i = 0, iend = d->mConstraints.count();  i < iend && !match;  ++i ) {
    match = d->mConstraints[i].matches( startDay, recurrenceType() );
    for ( int day = 1;  day < dayCount && !match;  ++day ) {
      match = d->mConstraints[i].matches( startDay.addDays( day ), recurrenceType() );
    }
  }
  if ( !match ) {
    return false;
  }

  Constraint interval( d->getNextValidDateInterval( start, recurrenceType() ) );
  // Constraint::matches is quite efficient, so first check if it can occur at
  // all before we calculate all actual dates.
  match = false;
  Constraint intervalm = interval;
  do {
    match = intervalm.matches( startDay, recurrenceType() );
    for ( int day = 1;  day < dayCount && !match;  ++day ) {
      match = intervalm.matches( startDay.addDays( day ), recurrenceType() );
    }
    if ( match ) {
      break;
    }
    intervalm.increase( recurrenceType(), frequency() );
  } while ( intervalm.intervalDateTime( recurrenceType() ) < end );
  if ( !match ) {
    return false;
  }

  // We really need to obtain the list of dates in this interval, since
  // otherwise BYSETPOS will not work (i.e. the date will match the interval,
  // but BYSETPOS selects only one of these matching dates!
  do {
    DateTimeList dts = d->datesForInterval( interval, recurrenceType() );
    int i = dts.findGE( start );
    if ( i >= 0 ) {
      return dts[i] <= end;
    }
    interval.increase( recurrenceType(), frequency() );
  } while ( interval.intervalDateTime( recurrenceType() ) < end );

  return false;
}

bool RecurrenceRule::recursAt( const KDateTime &kdt ) const
{
  // Convert to the time spec used by this recurrence rule
  KDateTime dt( kdt.toTimeSpec( d->mDateStart.timeSpec() ) );

  if ( allDay() ) {
    return recursOn( dt.date(), dt.timeSpec() );
  }
  if ( dt < d->mDateStart ) {
    return false;
  }
  // Start date is only included if it really matches
  if ( d->mDuration >= 0 && dt > endDt() ) {
    return false;
  }

  if ( d->mTimedRepetition ) {
    // It's a simple sub-daily recurrence with no constraints
    return !( d->mDateStart.secsTo_long( dt ) % d->mTimedRepetition );
  }

  // The date must be in an appropriate interval (getNextValidDateInterval),
  // Plus it must match at least one of the constraints
  if ( !dateMatchesRules( dt ) ) {
    return false;
  }
  // if it recurs every interval, speed things up...
//   if ( d->mFrequency == 1 && d->mBySetPos.isEmpty() && d->mByDays.isEmpty() ) return true;
  Constraint interval( d->getNextValidDateInterval( dt, recurrenceType() ) );
  // TODO_Recurrence: Does this work with BySetPos???
  if ( interval.matches( dt, recurrenceType() ) ) {
    return true;
  }
  return false;
}

TimeList RecurrenceRule::recurTimesOn( const QDate &date, const KDateTime::Spec &timeSpec ) const
{
  TimeList lst;
  if ( allDay() ) {
    return lst;
  }
  KDateTime start( date, QTime( 0, 0, 0 ), timeSpec );
  KDateTime end = start.addDays( 1 ).addSecs( -1 );
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
  KDateTime toDate( dt.toTimeSpec( d->mDateStart.timeSpec() ) );
  // Easy cases:
  // either before start, or after all recurrences and we know their number
  if ( toDate < d->mDateStart ) {
    return 0;
  }
  // Start date is only included if it really matches
  if ( d->mDuration > 0 && toDate >= endDt() ) {
    return d->mDuration;
  }

  if ( d->mTimedRepetition ) {
    // It's a simple sub-daily recurrence with no constraints
    return static_cast<int>( d->mDateStart.secsTo_long( toDate ) / d->mTimedRepetition );
  }

  return timesInInterval( d->mDateStart, toDate ).count();
}

int RecurrenceRule::durationTo( const QDate &date ) const
{
  return durationTo( KDateTime( date, QTime( 23, 59, 59 ), d->mDateStart.timeSpec() ) );
}

KDateTime RecurrenceRule::getPreviousDate( const KDateTime &afterDate ) const
{
  // Convert to the time spec used by this recurrence rule
  KDateTime toDate( afterDate.toTimeSpec( d->mDateStart.timeSpec() ) );

  // Invalid starting point, or beyond end of recurrence
  if ( !toDate.isValid() || toDate < d->mDateStart ) {
    return KDateTime();
  }

  if ( d->mTimedRepetition ) {
    // It's a simple sub-daily recurrence with no constraints
    KDateTime prev = toDate;
    if ( d->mDuration >= 0 && endDt().isValid() && toDate > endDt() ) {
      prev = endDt().addSecs( 1 ).toTimeSpec( d->mDateStart.timeSpec() );
    }
    int n = static_cast<int>( ( d->mDateStart.secsTo_long( prev ) - 1 ) % d->mTimedRepetition );
    if ( n < 0 ) {
      return KDateTime();  // before recurrence start
    }
    prev = prev.addSecs( -n - 1 );
    return prev >= d->mDateStart ? prev : KDateTime();
  }

  // If we have a cache (duration given), use that
  if ( d->mDuration > 0 ) {
    if ( !d->mCached ) {
      d->buildCache();
    }
    int i = d->mCachedDates.findLT( toDate );
    if ( i >= 0 ) {
      return d->mCachedDates[i];
    }
    return KDateTime();
  }

  KDateTime prev = toDate;
  if ( d->mDuration >= 0 && endDt().isValid() && toDate > endDt() ) {
    prev = endDt().addSecs( 1 ).toTimeSpec( d->mDateStart.timeSpec() );
  }

  Constraint interval( d->getPreviousValidDateInterval( prev, recurrenceType() ) );
  DateTimeList dts = d->datesForInterval( interval, recurrenceType() );
  int i = dts.findLT( prev );
  if ( i >= 0 ) {
    return ( dts[i] >= d->mDateStart ) ? dts[i] : KDateTime();
  }

  // Previous interval. As soon as we find an occurrence, we're done.
  while ( interval.intervalDateTime( recurrenceType() ) > d->mDateStart ) {
    interval.increase( recurrenceType(), -int( frequency() ) );
    // The returned date list is sorted
    DateTimeList dts = d->datesForInterval( interval, recurrenceType() );
    // The list is sorted, so take the last one.
    if ( !dts.isEmpty() ) {
      prev = dts.last();
      if ( prev.isValid() && prev >= d->mDateStart ) {
        return prev;
      } else {
        return KDateTime();
      }
    }
  }
  return KDateTime();
}

KDateTime RecurrenceRule::getNextDate( const KDateTime &preDate ) const
{
  // Convert to the time spec used by this recurrence rule
  KDateTime fromDate( preDate.toTimeSpec( d->mDateStart.timeSpec() ) );
  // Beyond end of recurrence
  if ( d->mDuration >= 0 && endDt().isValid() && fromDate >= endDt() ) {
    return KDateTime();
  }

  // Start date is only included if it really matches
  if ( fromDate < d->mDateStart ) {
    fromDate = d->mDateStart.addSecs( -1 );
  }

  if ( d->mTimedRepetition ) {
    // It's a simple sub-daily recurrence with no constraints
    int n = static_cast<int>( ( d->mDateStart.secsTo_long( fromDate ) + 1 ) % d->mTimedRepetition );
    KDateTime next = fromDate.addSecs( d->mTimedRepetition - n + 1 );
    return d->mDuration < 0 || !endDt().isValid() || next <= endDt() ? next : KDateTime();
  }

  if ( d->mDuration > 0 ) {
    if ( !d->mCached ) {
      d->buildCache();
    }
    int i = d->mCachedDates.findGT( fromDate );
    if ( i >= 0 ) {
      return d->mCachedDates[i];
    }
  }

  KDateTime end = endDt();
  Constraint interval( d->getNextValidDateInterval( fromDate, recurrenceType() ) );
  DateTimeList dts = d->datesForInterval( interval, recurrenceType() );
  int i = dts.findGT( fromDate );
  if ( i >= 0 ) {
    return ( d->mDuration < 0 || dts[i] <= end ) ? dts[i] : KDateTime();
  }
  interval.increase( recurrenceType(), frequency() );
  if ( d->mDuration >= 0 && interval.intervalDateTime( recurrenceType() ) > end ) {
    return KDateTime();
  }

  // Increase the interval. The first occurrence that we find is the result (if
  // if's before the end date).
  // TODO: some validity checks to avoid infinite loops for contradictory constraints
  int loop = 0;
  do {
    DateTimeList dts = d->datesForInterval( interval, recurrenceType() );
    if ( dts.count() > 0 ) {
      KDateTime ret( dts[0] );
      if ( d->mDuration >= 0 && ret > end ) {
        return KDateTime();
      } else {
        return ret;
      }
    }
    interval.increase( recurrenceType(), frequency() );
  } while ( ++loop < LOOP_LIMIT &&
            ( d->mDuration < 0 || interval.intervalDateTime( recurrenceType() ) < end ) );
  return KDateTime();
}

DateTimeList RecurrenceRule::timesInInterval( const KDateTime &dtStart,
                                              const KDateTime &dtEnd ) const
{
  const KDateTime start = dtStart.toTimeSpec( d->mDateStart.timeSpec() );
  const KDateTime end = dtEnd.toTimeSpec( d->mDateStart.timeSpec() );
  DateTimeList result;
  if ( end < d->mDateStart ) {
    return result;    // before start of recurrence
  }
  KDateTime enddt = end;
  if ( d->mDuration >= 0 ) {
    const KDateTime endRecur = endDt();
    if ( endRecur.isValid() ) {
      if ( start > endRecur ) {
        return result;    // beyond end of recurrence
      }
      if ( end >= endRecur ) {
        enddt = endRecur;    // limit end time to end of recurrence rule
      }
    }
  }

  if ( d->mTimedRepetition ) {
    // It's a simple sub-daily recurrence with no constraints

    //Seconds to add to interval start, to get first occurrence which is within interval
    qint64 offsetFromNextOccurrence;
    if ( d->mDateStart < start ) {
      offsetFromNextOccurrence =
        d->mTimedRepetition - ( d->mDateStart.secsTo_long( start ) % d->mTimedRepetition );
    } else {
      offsetFromNextOccurrence = -( d->mDateStart.secsTo_long( start ) % d->mTimedRepetition );
    }
    KDateTime dt = start.addSecs( offsetFromNextOccurrence );
    if ( dt <= enddt ) {
      int numberOfOccurrencesWithinInterval =
        static_cast<int>( dt.secsTo_long( enddt ) / d->mTimedRepetition ) + 1;
      // limit n by a sane value else we can "explode".
      numberOfOccurrencesWithinInterval = qMin( numberOfOccurrencesWithinInterval, LOOP_LIMIT );
      for ( int i = 0;
            i < numberOfOccurrencesWithinInterval;
            dt = dt.addSecs( d->mTimedRepetition ), ++i ) {
        result += dt;
      }
    }
    return result;
  }

  KDateTime st = start;
  bool done = false;
  if ( d->mDuration > 0 ) {
    if ( !d->mCached ) {
      d->buildCache();
    }
    if ( d->mCachedDateEnd.isValid() && start > d->mCachedDateEnd ) {
      return result;    // beyond end of recurrence
    }
    int i = d->mCachedDates.findGE( start );
    if ( i >= 0 ) {
      int iend = d->mCachedDates.findGT( enddt, i );
      if ( iend < 0 ) {
        iend = d->mCachedDates.count();
      } else {
        done = true;
      }
      while ( i < iend ) {
        result += d->mCachedDates[i++];
      }
    }
    if ( d->mCachedDateEnd.isValid() ) {
      done = true;
    } else if ( !result.isEmpty() ) {
      result += KDateTime();    // indicate that the returned list is incomplete
      done = true;
    }
    if ( done ) {
      return result;
    }
    // We don't have any result yet, but we reached the end of the incomplete cache
    st = d->mCachedLastDate.addSecs( 1 );
  }

  Constraint interval( d->getNextValidDateInterval( st, recurrenceType() ) );
  int loop = 0;
  do {
    DateTimeList dts = d->datesForInterval( interval, recurrenceType() );
    int i = 0;
    int iend = dts.count();
    if ( loop == 0 ) {
      i = dts.findGE( st );
      if ( i < 0 ) {
        i = iend;
      }
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
  } while ( ++loop < LOOP_LIMIT &&
            interval.intervalDateTime( recurrenceType() ) < end );
  return result;
}

//@cond PRIVATE
// Find the date/time of the occurrence at or before a date/time,
// for a given period type.
// Return a constraint whose value appropriate to 'type', is set to
// the value contained in the date/time.
Constraint RecurrenceRule::Private::getPreviousValidDateInterval( const KDateTime &dt,
                                                                  PeriodType type ) const
{
  long periods = 0;
  KDateTime start = mDateStart;
  KDateTime nextValid( start );
  int modifier = 1;
  KDateTime toDate( dt.toTimeSpec( start.timeSpec() ) );
  // for super-daily recurrences, don't care about the time part

  // Find the #intervals since the dtstart and round to the next multiple of
  // the frequency
  switch ( type ) {
    // Really fall through for sub-daily, since the calculations only differ
    // by the factor 60 and 60*60! Same for weekly and daily (factor 7)
  case rHourly:
    modifier *= 60;
  case rMinutely:
    modifier *= 60;
  case rSecondly:
    periods = static_cast<int>( start.secsTo_long( toDate ) / modifier );
    // round it down to the next lower multiple of frequency:
    if ( mFrequency > 0 ) {
      periods = ( periods / mFrequency ) * mFrequency;
    }
    nextValid = start.addSecs( modifier * periods );
    break;
  case rWeekly:
    toDate = toDate.addDays( -( 7 + toDate.date().dayOfWeek() - mWeekStart ) % 7 );
    start = start.addDays( -( 7 + start.date().dayOfWeek() - mWeekStart ) % 7 );
    modifier *= 7;
  case rDaily:
    periods = start.daysTo( toDate ) / modifier;
    // round it down to the next lower multiple of frequency:
    if ( mFrequency > 0 ) {
      periods = ( periods / mFrequency ) * mFrequency;
    }
    nextValid = start.addDays( modifier * periods );
    break;
  case rMonthly:
  {
    periods = 12 * ( toDate.date().year() - start.date().year() ) +
              ( toDate.date().month() - start.date().month() );
    // round it down to the next lower multiple of frequency:
    if ( mFrequency > 0 ) {
      periods = ( periods / mFrequency ) * mFrequency;
    }
    // set the day to the first day of the month, so we don't have problems
    // with non-existent days like Feb 30 or April 31
    start.setDate( QDate( start.date().year(), start.date().month(), 1 ) );
    nextValid.setDate( start.date().addMonths( periods ) );
    break; }
  case rYearly:
    periods = ( toDate.date().year() - start.date().year() );
    // round it down to the next lower multiple of frequency:
    if ( mFrequency > 0 ) {
      periods = ( periods / mFrequency ) * mFrequency;
    }
    nextValid.setDate( start.date().addYears( periods ) );
    break;
  default:
    break;
  }

  return Constraint( nextValid, type, mWeekStart );
}

// Find the date/time of the next occurrence at or after a date/time,
// for a given period type.
// Return a constraint whose value appropriate to 'type', is set to the
// value contained in the date/time.
Constraint RecurrenceRule::Private::getNextValidDateInterval( const KDateTime &dt,
                                                              PeriodType type ) const
{
  // TODO: Simplify this!
  long periods = 0;
  KDateTime start = mDateStart;
  KDateTime nextValid( start );
  int modifier = 1;
  KDateTime toDate( dt.toTimeSpec( start.timeSpec() ) );
  // for super-daily recurrences, don't care about the time part

  // Find the #intervals since the dtstart and round to the next multiple of
  // the frequency
  switch ( type ) {
    // Really fall through for sub-daily, since the calculations only differ
    // by the factor 60 and 60*60! Same for weekly and daily (factor 7)
  case rHourly:
    modifier *= 60;
  case rMinutely:
    modifier *= 60;
  case rSecondly:
    periods = static_cast<int>( start.secsTo_long( toDate ) / modifier );
    periods = qMax( 0L, periods );
    if ( periods > 0 && mFrequency > 0 ) {
      periods += ( mFrequency - 1 - ( ( periods - 1 ) % mFrequency ) );
    }
    nextValid = start.addSecs( modifier * periods );
    break;
  case rWeekly:
    // correct both start date and current date to start of week
    toDate = toDate.addDays( -( 7 + toDate.date().dayOfWeek() - mWeekStart ) % 7 );
    start = start.addDays( -( 7 + start.date().dayOfWeek() - mWeekStart ) % 7 );
    modifier *= 7;
  case rDaily:
    periods = start.daysTo( toDate ) / modifier;
    periods = qMax( 0L, periods );
    if ( periods > 0 && mFrequency > 0 ) {
      periods += ( mFrequency - 1 - ( ( periods - 1 ) % mFrequency ) );
    }
    nextValid = start.addDays( modifier * periods );
    break;
  case rMonthly:
  {
    periods = 12 * ( toDate.date().year() - start.date().year() ) +
              ( toDate.date().month() - start.date().month() );
    periods = qMax( 0L, periods );
    if ( periods > 0 && mFrequency > 0 ) {
      periods += ( mFrequency - 1 - ( ( periods - 1 ) % mFrequency ) );
    }
    // set the day to the first day of the month, so we don't have problems
    // with non-existent days like Feb 30 or April 31
    start.setDate( QDate( start.date().year(), start.date().month(), 1 ) );
    nextValid.setDate( start.date().addMonths( periods ) );
    break;
  }
  case rYearly:
    periods = ( toDate.date().year() - start.date().year() );
    periods = qMax( 0L, periods );
    if ( periods > 0 && mFrequency > 0 ) {
      periods += ( mFrequency - 1 - ( ( periods - 1 ) % mFrequency ) );
    }
    nextValid.setDate( start.date().addYears( periods ) );
    break;
  default:
    break;
  }

  return Constraint( nextValid, type, mWeekStart );
}

DateTimeList RecurrenceRule::Private::datesForInterval( const Constraint &interval,
                                                        PeriodType type ) const
{
  /* -) Loop through constraints,
     -) merge interval with each constraint
     -) if merged constraint is not consistent => ignore that constraint
     -) if complete => add that one date to the date list
     -) Loop through all missing fields => For each add the resulting
  */
  DateTimeList lst;
  for ( int i = 0, iend = mConstraints.count();  i < iend;  ++i ) {
    Constraint merged( interval );
    if ( merged.merge( mConstraints[i] ) ) {
      // If the information is incomplete, we can't use this constraint
      if ( merged.year > 0 && merged.hour >= 0 && merged.minute >= 0 && merged.second >= 0 ) {
        // We have a valid constraint, so get all datetimes that match it andd
        // append it to all date/times of this interval
        QList<KDateTime> lstnew = merged.dateTimes( type );
        lst += lstnew;
      }
    }
  }
  // Sort it so we can apply the BySetPos. Also some logic relies on this being sorted
  lst.sortUnique();

/*if ( lst.isEmpty() ) {
  kDebug() << "         No Dates in Interval";
} else {
  kDebug() << "         Dates:";
  for ( int i = 0, iend = lst.count();  i < iend;  ++i ) {
    kDebug()<< "              -)" << dumpTime(lst[i]);
  }
  kDebug() << "       ---------------------";
}*/
  if ( !mBySetPos.isEmpty() ) {
    DateTimeList tmplst = lst;
    lst.clear();
    for ( int i = 0, iend = mBySetPos.count();  i < iend;  ++i ) {
      int pos = mBySetPos[i];
      if ( pos > 0 ) {
        --pos;
      }
      if ( pos < 0 ) {
        pos += tmplst.count();
      }
      if ( pos >= 0 && pos < tmplst.count() ) {
        lst.append( tmplst[pos] );
      }
    }
    lst.sortUnique();
  }

  return lst;
}
//@endcond

void RecurrenceRule::dump() const
{
#ifndef NDEBUG
  kDebug();
  if ( !d->mRRule.isEmpty() ) {
    kDebug() << "   RRULE=" << d->mRRule;
  }
  kDebug() << "   Read-Only:" << isReadOnly();

  kDebug() << "   Period type:" << int( recurrenceType() ) << ", frequency:" << frequency();
  kDebug() << "   #occurrences:" << duration();
  kDebug() << "   start date:" << dumpTime( startDt() )
           << ", end date:" << dumpTime( endDt() );

#define dumpByIntList(list,label) \
  if ( !list.isEmpty() ) {\
    QStringList lst;\
    for ( int i = 0, iend = list.count();  i < iend;  ++i ) {\
      lst.append( QString::number( list[i] ) );\
    }\
    kDebug() << "  " << label << lst.join( QLatin1String(", ") );\
  }
  dumpByIntList( d->mBySeconds, QLatin1String("BySeconds:  ") );
  dumpByIntList( d->mByMinutes, QLatin1String("ByMinutes:  ") );
  dumpByIntList( d->mByHours, QLatin1String("ByHours:    ") );
  if ( !d->mByDays.isEmpty() ) {
    QStringList lst;
    for ( int i = 0, iend = d->mByDays.count();  i < iend;  ++i ) {\
      lst.append( ( d->mByDays[i].pos() ? QString::number( d->mByDays[i].pos() ) : QLatin1String("") ) +
                   DateHelper::dayName( d->mByDays[i].day() ) );
    }
    kDebug() << "   ByDays:    " << lst.join( QLatin1String(", ") );
  }
  dumpByIntList( d->mByMonthDays, QLatin1String("ByMonthDays:") );
  dumpByIntList( d->mByYearDays, QLatin1String("ByYearDays: ") );
  dumpByIntList( d->mByWeekNumbers, QLatin1String("ByWeekNr:   ") );
  dumpByIntList( d->mByMonths, QLatin1String("ByMonths:   ") );
  dumpByIntList( d->mBySetPos, QLatin1String("BySetPos:   ") );
  #undef dumpByIntList

  kDebug() << "   Week start:" << DateHelper::dayName( d->mWeekStart ); //krazy:exclude=kdebug

  kDebug() << "   Constraints:";
  // dump constraints
  for ( int i = 0, iend = d->mConstraints.count();  i < iend;  ++i ) {
    d->mConstraints[i].dump();
  }
#endif
}

//@cond PRIVATE
void Constraint::dump() const
{
  kDebug() << "     ~> Y=" << year
           << ", M=" << month
           << ", D=" << day
           << ", H=" << hour
           << ", m=" << minute
           << ", S=" << second
           << ", wd=" << weekday
           << ",#wd=" << weekdaynr
           << ", #w=" << weeknumber
           << ", yd=" << yearday;
}
//@endcond

QString dumpTime( const KDateTime &dt )
{
#ifndef NDEBUG
  if ( !dt.isValid() ) {
    return QString();
  }
  QString result;
  if ( dt.isDateOnly() ) {
    result = dt.toString( QLatin1String("%a %Y-%m-%d %:Z") );
  } else {
    result = dt.toString( QLatin1String("%a %Y-%m-%d %H:%M:%S %:Z") );
    if ( dt.isSecondOccurrence() ) {
      result += QLatin1String( " (2nd)" );
    }
  }
  if ( dt.timeSpec() == KDateTime::Spec::ClockTime() ) {
    result += QLatin1String( "Clock" );
  }
  return result;
#else
  Q_UNUSED( dt );
  return QString();
#endif
}

KDateTime RecurrenceRule::startDt() const
{
  return d->mDateStart;
}

RecurrenceRule::PeriodType RecurrenceRule::recurrenceType() const
{
  return d->mPeriod;
}

uint RecurrenceRule::frequency() const
{
  return d->mFrequency;
}

int RecurrenceRule::duration() const
{
  return d->mDuration;
}

QString RecurrenceRule::rrule() const
{
  return d->mRRule;
}

void RecurrenceRule::setRRule( const QString &rrule )
{
  d->mRRule = rrule;
}

bool RecurrenceRule::isReadOnly() const
{
  return d->mIsReadOnly;
}

void RecurrenceRule::setReadOnly( bool readOnly )
{
  d->mIsReadOnly = readOnly;
}

bool RecurrenceRule::recurs() const
{
  return d->mPeriod != rNone;
}

bool RecurrenceRule::allDay() const
{
  return d->mAllDay;
}

const QList<int> &RecurrenceRule::bySeconds() const
{
  return d->mBySeconds;
}

const QList<int> &RecurrenceRule::byMinutes() const
{
  return d->mByMinutes;
}

const QList<int> &RecurrenceRule::byHours() const
{
  return d->mByHours;
}

const QList<RecurrenceRule::WDayPos> &RecurrenceRule::byDays() const
{
  return d->mByDays;
}

const QList<int> &RecurrenceRule::byMonthDays() const
{
  return d->mByMonthDays;
}

const QList<int> &RecurrenceRule::byYearDays() const
{
  return d->mByYearDays;
}

const QList<int> &RecurrenceRule::byWeekNumbers() const
{
  return d->mByWeekNumbers;
}

const QList<int> &RecurrenceRule::byMonths() const
{
  return d->mByMonths;
}

const QList<int> &RecurrenceRule::bySetPos() const
{
  return d->mBySetPos;
}

short RecurrenceRule::weekStart() const
{
  return d->mWeekStart;
}

RecurrenceRule::RuleObserver::~RuleObserver()
{
}

RecurrenceRule::WDayPos::WDayPos( int ps, short dy )
  : mDay( dy ), mPos( ps )
{
}

void RecurrenceRule::WDayPos::setDay( short dy )
{
  mDay = dy;
}

short RecurrenceRule::WDayPos::day() const
{
  return mDay;
}

void RecurrenceRule::WDayPos::setPos( int ps )
{
  mPos = ps;
}

int RecurrenceRule::WDayPos::pos() const
{
  return mPos;
}


QDataStream& operator<<( QDataStream &out, const Constraint &c )
{
  out << c.year << c.month << c.day << c.hour << c.minute << c.second
      << c.weekday << c.weekdaynr << c.weeknumber << c.yearday << c.weekstart
      << c.timespec << c.secondOccurrence;

  return out;
}

QDataStream& operator>>( QDataStream &in, Constraint &c  )
{
  in >> c.year >> c.month >> c.day >> c.hour >> c.minute >> c.second
     >> c.weekday >> c.weekdaynr >> c.weeknumber >> c.yearday >> c.weekstart
     >> c.timespec >> c.secondOccurrence;
  return in;
}

KCALCORE_EXPORT QDataStream& KCalCore::operator<<( QDataStream &out, const KCalCore::RecurrenceRule::WDayPos &w )
{
  out << w.mDay << w.mPos;
  return out;
}

KCALCORE_EXPORT QDataStream& KCalCore::operator>>( QDataStream &in, KCalCore::RecurrenceRule::WDayPos &w  )
{
  in >> w.mDay >> w.mPos;
  return in;
}

KCALCORE_EXPORT QDataStream& KCalCore::operator<<( QDataStream &out, const KCalCore::RecurrenceRule *r )
{
  if (!r)
    return out;

  RecurrenceRule::Private *d = r->d;
  out << d->mRRule << static_cast<quint32>(d->mPeriod) << d->mDateStart << d->mFrequency << d->mDuration << d->mDateEnd
      << d->mBySeconds << d->mByMinutes << d->mByHours << d->mByDays << d->mByMonthDays
      << d->mByYearDays << d->mByWeekNumbers << d->mByMonths << d->mBySetPos
      << d->mWeekStart << d->mConstraints << d->mAllDay << d->mNoByRules << d->mTimedRepetition
      << d->mIsReadOnly;

  return out;
}


KCALCORE_EXPORT QDataStream& KCalCore::operator>>( QDataStream &in, const KCalCore::RecurrenceRule *r )
{
  if (!r)
    return in;

  RecurrenceRule::Private *d = r->d;
  quint32 period;
  in >> d->mRRule >> period >> d->mDateStart >> d->mFrequency >> d->mDuration >> d->mDateEnd
      >> d->mBySeconds >> d->mByMinutes >> d->mByHours >> d->mByDays >> d->mByMonthDays
      >> d->mByYearDays >> d->mByWeekNumbers >> d->mByMonths >> d->mBySetPos
      >> d->mWeekStart >> d->mConstraints >> d->mAllDay >> d->mNoByRules >> d->mTimedRepetition
      >> d->mIsReadOnly;

  d->mPeriod = static_cast<RecurrenceRule::PeriodType>( period );

  return in;
}
