/*
  kmime_dateformatter.cpp

  KMime, the KDE Internet mail/usenet news message library.
  Copyright (c) 2001 the KMime authors.
  See file AUTHORS for details

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
  This file is part of the API for handling @ref MIME data and
  defines the DateFormatter class.

  @brief
  Defines the DateFormatter class.

  @authors the KMime authors (see AUTHORS file)
*/

#include "kmime_dateformatter.h"

#include <config-kmime.h>

#include <stdlib.h> // for abs()

#include <QtCore/QTextStream>

#include <kglobal.h>
#include <klocalizedstring.h>
#include <kcalendarsystem.h>
#include <KLocale>

using namespace KMime;

//@cond PRIVATE
int DateFormatter::mDaylight = -1;
//@endcond
DateFormatter::DateFormatter( FormatType ftype )
  : mFormat( ftype ), mTodayOneSecondBeforeMidnight( 0 )
{
}

DateFormatter::~DateFormatter()
{
}

DateFormatter::FormatType DateFormatter::format() const
{
  return mFormat;
}

void DateFormatter::setFormat( FormatType ftype )
{
  mFormat = ftype;
}

QString DateFormatter::dateString( time_t t , const QString &lang ,
                                   bool shortFormat, bool includeSecs ) const
{
  switch ( mFormat ) {
  case Fancy:
    return fancy( t );
    break;
  case Localized:
    return localized( t, shortFormat, includeSecs, lang );
    break;
  case CTime:
    return cTime( t );
    break;
  case Iso:
    return isoDate( t );
    break;
  case Rfc:
    return rfc2822( t );
    break;
  case Custom:
    return custom( t );
    break;
  }
  return QString();
}

QString DateFormatter::dateString( const QDateTime &dt, const QString &lang,
                                   bool shortFormat, bool includeSecs ) const
{
  return dateString( qdateToTimeT( dt ), lang, shortFormat, includeSecs );
}

QString DateFormatter::rfc2822( time_t t ) const
{
  QDateTime tmp;
  QString ret;

  tmp.setTime_t( t );

  ret = tmp.toString( QLatin1String( "ddd, dd MMM yyyy hh:mm:ss " ) );
  ret += QLatin1String( zone( t ) );

  return ret;
}

QString DateFormatter::custom( time_t t ) const
{
  if ( mCustomFormat.isEmpty() ) {
    return QString();
  }

  int z = mCustomFormat.indexOf( QLatin1Char( 'Z' ) );
  QDateTime d;
  QString ret = mCustomFormat;

  d.setTime_t( t );
  if ( z != -1 ) {
    ret.replace( z, 1, QLatin1String( zone( t ) ) );
  }

  ret = d.toString( ret );

  return ret;
}

void DateFormatter::setCustomFormat( const QString &format )
{
  mCustomFormat = format;
  mFormat = Custom;
}

QString DateFormatter::customFormat() const
{
  return mCustomFormat;
}

QByteArray DateFormatter::zone( time_t t ) const
{
#if defined(HAVE_TIMEZONE) || defined(HAVE_TM_GMTOFF)
  struct tm *local = localtime( &t );
#endif

#if defined(HAVE_TIMEZONE)

  //hmm, could make hours & mins static
  int secs = abs( timezone );
  int neg  = ( timezone > 0 ) ? 1 : 0;
  int hours = secs / 3600;
  int mins  = ( secs - hours * 3600 ) / 60;

  // adjust to daylight
  if ( local->tm_isdst > 0 ) {
    mDaylight = 1;
    if ( neg ) {
      --hours;
    } else {
      ++hours;
    }
  } else {
    mDaylight = 0;
  }

#elif defined(HAVE_TM_GMTOFF)

  int secs = abs( local->tm_gmtoff );
  int neg  = ( local->tm_gmtoff < 0 ) ? 1 : 0;
  int hours = secs / 3600;
  int mins  = ( secs - hours * 3600 ) / 60;

  if ( local->tm_isdst > 0 ) {
    mDaylight = 1;
  } else {
    mDaylight = 0;
  }

#else

  QDateTime d1 = QDateTime::fromString( QString::fromLatin1( asctime( gmtime( &t ) ) ) );
  QDateTime d2 = QDateTime::fromString( QString::fromLatin1( asctime( localtime( &t ) ) ) );
  int secs = d1.secsTo( d2 );
  int neg = ( secs < 0 ) ? 1 : 0;
  secs = abs( secs );
  int hours = secs / 3600;
  int mins  = ( secs - hours * 3600 ) / 60;
  // daylight should be already taken care of here

#endif /* HAVE_TIMEZONE */

  QByteArray ret;
  QTextStream s( &ret, QIODevice::WriteOnly );
  s << ( neg ? '-' : '+' )
    << qSetFieldWidth( 2 ) << qSetPadChar( QLatin1Char( '0' ) ) << right << hours << mins;
  //old code: ret.sprintf( "%c%.2d%.2d", (neg) ? '-' : '+', hours, mins );

  return ret;
}

time_t DateFormatter::qdateToTimeT( const QDateTime &dt ) const
{
  QDateTime epoch( QDate( 1970, 1, 1 ), QTime( 00, 00, 00 ) );
  time_t t;
  time( &t );

  QDateTime d1 = QDateTime::fromString( QLatin1String( asctime( gmtime( &t ) ) ) );
  QDateTime d2 = QDateTime::fromString( QLatin1String( asctime( localtime( &t ) ) ) );
  time_t drf = epoch.secsTo( dt ) - d1.secsTo( d2 );

  return drf;
}

QString DateFormatter::fancy( time_t t ) const
{
  KLocale *locale = KLocale::global();

  if ( t <= 0 ) {
    return i18nc( "invalid time specified", "unknown" );
  }

  if ( mTodayOneSecondBeforeMidnight < time( 0 ) ) {
    // determine time_t value of today 23:59:59
    const QDateTime today( QDate::currentDate(), QTime( 23, 59, 59 ) );
    mTodayOneSecondBeforeMidnight = today.toTime_t();
  }

  QDateTime old;
  old.setTime_t( t );

  if ( mTodayOneSecondBeforeMidnight >= t ) {
    const time_t diff = mTodayOneSecondBeforeMidnight - t;
    if ( diff < 7 * 24 * 60 * 60 ) {
      if ( diff < 24 * 60 * 60 ) {
        return i18n( "Today %1",
                     locale->formatTime( old.time(), true ) );
      }
      if ( diff < 2 * 24 * 60 * 60 ) {
        return i18n( "Yesterday %1",
                     locale->formatTime( old.time(), true ) );
      }
      for ( int i = 3; i < 8; i++ ) {
        if ( diff < i * 24 * 60 * 60 ) {
          return i18nc( "1. weekday, 2. time", "%1 %2" ,
                        locale->calendar()->weekDayName( old.date() ) ,
                        locale->formatTime( old.time(), true ) );
        }
      }
    }
  }

  return locale->formatDateTime( old );
}

QString DateFormatter::localized( time_t t, bool shortFormat, bool includeSecs,
                                  const QString &lang ) const
{
  QDateTime tmp;
  QString ret;
  KLocale *locale = KLocale::global();

  tmp.setTime_t( t );

  if ( !lang.isEmpty() ) {
    locale = new KLocale( lang, lang, 0 );
    ret = locale->formatDateTime( tmp, ( shortFormat ? KLocale::ShortDate : KLocale::LongDate ), includeSecs );
    delete locale;
  } else {
    ret = locale->formatDateTime( tmp, ( shortFormat ? KLocale::ShortDate : KLocale::LongDate ), includeSecs );
  }

  return ret;
}

QString DateFormatter::cTime( time_t t ) const
{
  return QString::fromLatin1( ctime(  &t ) ).trimmed();
}

QString DateFormatter::isoDate( time_t t ) const
{
  char cstr[64];
  strftime( cstr, 63, "%Y-%m-%d %H:%M:%S", localtime( &t ) );
  return QLatin1String( cstr );
}

void DateFormatter::reset()
{
  mTodayOneSecondBeforeMidnight = 0;
}

QString DateFormatter::formatDate( FormatType ftype, time_t t,
                                   const QString &data, bool shortFormat,
                                   bool includeSecs )
{
  DateFormatter f( ftype );
  if ( ftype == Custom ) {
    f.setCustomFormat( data );
  }
  return f.dateString( t, data, shortFormat, includeSecs );
}

QString DateFormatter::formatCurrentDate( FormatType ftype, const QString &data,
                                          bool shortFormat, bool includeSecs )
{
  DateFormatter f( ftype );
  if ( ftype == Custom ) {
    f.setCustomFormat( data );
  }
  return f.dateString( time( 0 ), data, shortFormat, includeSecs );
}

bool DateFormatter::isDaylight()
{
  if ( mDaylight == -1 ) {
    time_t ntime = time( 0 );
    struct tm *local = localtime( &ntime );
    if ( local->tm_isdst > 0 ) {
      mDaylight = 1;
      return true;
    } else {
      mDaylight = 0;
      return false;
    }
  } else if ( mDaylight != 0 ) {
    return true;
  } else {
    return false;
  }
}
