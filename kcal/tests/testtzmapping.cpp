/*
  This file is part of the kcal library.
  Copyright (c) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
#include <qtest_kde.h>

#include "testtzmapping.h"
#include "testtzmapping.moc"

QTEST_KDEMAIN( TZMappingTest, NoGUI )

#include "kcal/tzmapping.h"
using namespace KCal;

#include <KDebug>
#include <KTimeZone>

void TZMappingTest::testWinStandardToDisplay()
{
  QCOMPARE( TzMaps::winZoneStandardToDisplay( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TzMaps::winZoneStandardToDisplay( "US Mountain Standard Time" ),
            QString( "Arizona" ) );
  QCOMPARE( TzMaps::winZoneStandardToDisplay( "US mountain time" ),
            QString() );
  QCOMPARE( TzMaps::winZoneStandardToDisplay( "Romance Standard Time" ),
            QString( "Brussels, Copenhagen, Madrid, Paris" ) );
  QCOMPARE( TzMaps::winZoneStandardToDisplay( QString() ),
            QString() );
}

void TZMappingTest::testWinDisplayToStandard()
{
  QCOMPARE( TzMaps::winZoneDisplayToStandard( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TzMaps::winZoneDisplayToStandard( "(GMT-07:00) Arizona" ),
            QString( "US Mountain Standard Time" ) );
  QCOMPARE( TzMaps::winZoneDisplayToStandard( "ariZona" ),
            QString() );
  QCOMPARE( TzMaps::winZoneDisplayToStandard( "Brussels, Copenhagen, Madrid, Paris" ),
            QString( "Romance Standard Time" ) );
  QCOMPARE( TzMaps::winZoneDisplayToStandard( QString() ),
            QString() );
}

void TZMappingTest::testWinToOlson()
{
  QCOMPARE( TzMaps::winZoneToOlson( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TzMaps::winZoneToOlson( "Hawaii" ),
            QString( "Pacific/Honolulu" ) );
  QCOMPARE( TzMaps::winZoneToOlson( "(GMT-09:00) Hawaii" ),
            QString( "Pacific/Honolulu" ) );
  QCOMPARE( TzMaps::winZoneToOlson( "hawaii" ),
            QString() );
  QCOMPARE( TzMaps::winZoneToOlson( "Moscow, St. Petersburg, Volgograd" ),
            QString( "Europe/Moscow" ) );
  QCOMPARE( TzMaps::winZoneToOlson( "Central European Standard Time" ),
            QString( "Europe/Warsaw" ) );
  QCOMPARE( TzMaps::winZoneToOlson( QString() ),
            QString() );
}

void TZMappingTest::testWinToUtcOffset()
{
  QCOMPARE( TzMaps::winZoneToUtcOffset( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TzMaps::winZoneToUtcOffset( "Hawaii" ),
            QString( "UTC-10" ) );
  QCOMPARE( TzMaps::winZoneToUtcOffset( "hawaii" ),
            QString() );
  QCOMPARE( TzMaps::winZoneToUtcOffset( "Moscow, St. Petersburg, Volgograd" ),
            QString( "UTC+3" ) );
  QCOMPARE( TzMaps::winZoneToUtcOffset( QString() ),
            QString() );
}

void TZMappingTest::testUtcOffsetToWin()
{
  QCOMPARE( TzMaps::utcOffsetToWinZone( "UTC+10000" ),
            QString() );
  QCOMPARE( TzMaps::utcOffsetToWinZone( "UTC-10" ),
            QString( "Hawaii" ) );
  QCOMPARE( TzMaps::utcOffsetToWinZone( "utc-10" ),
            QString( "Hawaii" ) );
  QCOMPARE( TzMaps::utcOffsetToWinZone( "UTC+3" ),
            QString( "Moscow, St. Petersburg, Volgograd" ) );
  QCOMPARE( TzMaps::utcOffsetToWinZone( QString() ),
            QString() );
}

void TZMappingTest::testUtcToUtc()
{
  QCOMPARE( TzMaps::winZoneToUtcOffset( TzMaps::utcOffsetToWinZone( QString() ) ),
            QString() );
  QCOMPARE( TzMaps::winZoneToUtcOffset( TzMaps::utcOffsetToWinZone( "UTC+1000" ) ),
            QString() );
  QCOMPARE( TzMaps::winZoneToUtcOffset( TzMaps::utcOffsetToWinZone( "utc-10" ) ),
            QString( "UTC-10" ) );
  QCOMPARE( TzMaps::winZoneToUtcOffset( TzMaps::utcOffsetToWinZone( "UtC" ) ),
            QString( "UTC" ) );
}

void TZMappingTest::testWinToWin()
{
  QCOMPARE( TzMaps::utcOffsetToWinZone( TzMaps::winZoneToUtcOffset( QString() ) ),
            QString() );
  QCOMPARE( TzMaps::utcOffsetToWinZone( TzMaps::winZoneToUtcOffset( "NO-SUCH-ZONE" ) ),
            QString() );
  QCOMPARE( TzMaps::utcOffsetToWinZone( TzMaps::winZoneToUtcOffset( "Hawaii" ) ),
            QString( "Hawaii" ) );
  QCOMPARE( TzMaps::utcOffsetToWinZone( TzMaps::winZoneToUtcOffset( "Central America" ) ),
            QString( "Central Time (US & Canada)" ) );
}

void TZMappingTest::testOlsonToWin()
{
  QCOMPARE( TzMaps::olsonToWinZone( "America/New_York" ),
            QString( "Eastern Time (US & Canada)" ) );
  QCOMPARE( TzMaps::olsonToWinZone( "Australia/Sydney" ),
            QString( "Canberra, Melbourne, Sydney" ) );
  QCOMPARE( TzMaps::olsonToWinZone( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TzMaps::olsonToWinZone( QString() ),
            QString() );
  QCOMPARE( TzMaps::olsonToWinZone( "Europe/London" ),
            QString( "Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London" ) );
  QCOMPARE( TzMaps::olsonToWinZone( "Asia/Kolkata" ),
            QString( "Chennai, Kolkata, Mumbai, New Delhi" ) );
  QCOMPARE( TzMaps::olsonToWinZone( "Pacific/Chatham" ),
            QString() );
}

void TZMappingTest::testOlsonToUtcOffset()
{
  QCOMPARE( TzMaps::olsonToUtcOffset( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TzMaps::olsonToUtcOffset( "America/Argentina/Catamarca" ),
            QString( "UTC-4" ) );
  QCOMPARE( TzMaps::olsonToUtcOffset( "hawaii" ),
            QString() );
  QCOMPARE( TzMaps::olsonToUtcOffset( "America/New_York" ),
            QString( "UTC-5" ) );
  QCOMPARE( TzMaps::olsonToUtcOffset( QString() ),
            QString() );
}

void TZMappingTest::testUtcOffsetToOlson()
{
  QCOMPARE( TzMaps::utcOffsetToOlson( "UTC+10000" ),
            QString() );
  QCOMPARE( TzMaps::utcOffsetToOlson( "UTC-10" ),
            QString( "Pacific/Honolulu" ) );
  QCOMPARE( TzMaps::utcOffsetToOlson( "utc-10" ),
            QString( "Pacific/Honolulu" ) );
  QCOMPARE( TzMaps::utcOffsetToOlson( "utc-4:30" ),
            QString( "America/Caracas" ) );
  QCOMPARE( TzMaps::utcOffsetToOlson( "UTC+3" ),
            QString( "Europe/Moscow" ) );
  QCOMPARE( TzMaps::utcOffsetToOlson( QString() ),
            QString() );
}

static QString abbrevStr( const QList<QByteArray> &abbrevs )
{
  QString str;
  for ( int i = 0, end = abbrevs.count();  i < end;  ++i ) {
    if ( i > 0 ) {
      str += ',';
    }
    str += QString::fromLocal8Bit( abbrevs[i] );
  }
  return str;
}

void TZMappingTest::testAbbrevs()
{
  KTimeZone zonep1( TzMaps::utcOffsetToOlson( "UTC+1" ) );
  KTimeZone zonep2( TzMaps::utcOffsetToOlson( "UTC+2" ) );
  KTimeZone zonep3( TzMaps::utcOffsetToOlson( "UTC+3" ) );
  KTimeZone zonep4( TzMaps::utcOffsetToOlson( "UTC+4" ) );
  KTimeZone zonep5( TzMaps::utcOffsetToOlson( "UTC+5" ) );

  QCOMPARE( abbrevStr( TzMaps::utcOffsetToAbbreviation( "UTC+1" ) ),
            QString( "CET,WAT,WEST" ) );
  QCOMPARE( abbrevStr( TzMaps::utcOffsetToAbbreviation( "UTC+2" ) ),
            QString( "CAT,CEST,EET,IST,SAST" ) );
  QCOMPARE( abbrevStr( TzMaps::utcOffsetToAbbreviation( "UTC+3" ) ),
            QString( "AST,EAT,EEST,MSK" ) );
  QCOMPARE( abbrevStr( TzMaps::utcOffsetToAbbreviation( "UTC+4" ) ),
            QString( "AMT,AST,AZT,GET,MUT,RET,SAMT,SCT" ) );
  QCOMPARE( abbrevStr( TzMaps::utcOffsetToAbbreviation( "UTC+5" ) ),
            QString( "AMST,HMT,PKT,YEKT" ) );
}


