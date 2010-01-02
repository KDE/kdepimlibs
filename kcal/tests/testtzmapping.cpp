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
  QCOMPARE( TZMaps::winZoneStandardToDisplay( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TZMaps::winZoneStandardToDisplay( "US Mountain Standard Time" ),
            QString( "Arizona" ) );
  QCOMPARE( TZMaps::winZoneStandardToDisplay( "US mountain time" ),
            QString() );
  QCOMPARE( TZMaps::winZoneStandardToDisplay( "Romance Standard Time" ),
            QString( "Brussels, Copenhagen, Madrid, Paris" ) );
  QCOMPARE( TZMaps::winZoneStandardToDisplay( QString() ),
            QString() );
}

void TZMappingTest::testWinDisplayToStandard()
{
  QCOMPARE( TZMaps::winZoneDisplayToStandard( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TZMaps::winZoneDisplayToStandard( "(GMT-07:00) Arizona" ),
            QString( "US Mountain Standard Time" ) );
  QCOMPARE( TZMaps::winZoneDisplayToStandard( "ariZona" ),
            QString() );
  QCOMPARE( TZMaps::winZoneDisplayToStandard( "Brussels, Copenhagen, Madrid, Paris" ),
            QString( "Romance Standard Time" ) );
  QCOMPARE( TZMaps::winZoneDisplayToStandard( QString() ),
            QString() );
}

void TZMappingTest::testWinToOlson()
{
  QCOMPARE( TZMaps::winZoneToOlson( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TZMaps::winZoneToOlson( "Hawaii" ),
            QString( "Pacific/Honolulu" ) );
  QCOMPARE( TZMaps::winZoneToOlson( "(GMT-09:00) Hawaii" ),
            QString( "Pacific/Honolulu" ) );
  QCOMPARE( TZMaps::winZoneToOlson( "hawaii" ),
            QString() );
  QCOMPARE( TZMaps::winZoneToOlson( "Moscow, St. Petersburg, Volgograd" ),
            QString( "Europe/Moscow" ) );
  QCOMPARE( TZMaps::winZoneToOlson( "Central European Standard Time" ),
            QString( "Europe/Warsaw" ) );
  QCOMPARE( TZMaps::winZoneToOlson( QString() ),
            QString() );
}

void TZMappingTest::testWinToUtcOffset()
{
  QCOMPARE( TZMaps::winZoneToUtcOffset( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TZMaps::winZoneToUtcOffset( "Hawaii" ),
            QString( "UTC-10" ) );
  QCOMPARE( TZMaps::winZoneToUtcOffset( "hawaii" ),
            QString() );
  QCOMPARE( TZMaps::winZoneToUtcOffset( "Moscow, St. Petersburg, Volgograd" ),
            QString( "UTC+3" ) );
  QCOMPARE( TZMaps::winZoneToUtcOffset( QString() ),
            QString() );
}

void TZMappingTest::testUtcOffsetToWin()
{
  QCOMPARE( TZMaps::utcOffsetToWinZone( "UTC+10000" ),
            QString() );
  QCOMPARE( TZMaps::utcOffsetToWinZone( "UTC-10" ),
            QString( "Hawaii" ) );
  QCOMPARE( TZMaps::utcOffsetToWinZone( "utc-10" ),
            QString( "Hawaii" ) );
  QCOMPARE( TZMaps::utcOffsetToWinZone( "UTC+3" ),
            QString( "Moscow, St. Petersburg, Volgograd" ) );
  QCOMPARE( TZMaps::utcOffsetToWinZone( QString() ),
            QString() );
}

void TZMappingTest::testUtcToUtc()
{
  QCOMPARE( TZMaps::winZoneToUtcOffset( TZMaps::utcOffsetToWinZone( QString() ) ),
            QString() );
  QCOMPARE( TZMaps::winZoneToUtcOffset( TZMaps::utcOffsetToWinZone( "UTC+1000" ) ),
            QString() );
  QCOMPARE( TZMaps::winZoneToUtcOffset( TZMaps::utcOffsetToWinZone( "utc-10" ) ),
            QString( "UTC-10" ) );
  QCOMPARE( TZMaps::winZoneToUtcOffset( TZMaps::utcOffsetToWinZone( "UtC" ) ),
            QString( "UTC" ) );
}

void TZMappingTest::testWinToWin()
{
  QCOMPARE( TZMaps::utcOffsetToWinZone( TZMaps::winZoneToUtcOffset( QString() ) ),
            QString() );
  QCOMPARE( TZMaps::utcOffsetToWinZone( TZMaps::winZoneToUtcOffset( "NO-SUCH-ZONE" ) ),
            QString() );
  QCOMPARE( TZMaps::utcOffsetToWinZone( TZMaps::winZoneToUtcOffset( "Hawaii" ) ),
            QString( "Hawaii" ) );
  QCOMPARE( TZMaps::utcOffsetToWinZone( TZMaps::winZoneToUtcOffset( "Central America" ) ),
            QString( "Central Time (US & Canada)" ) );
}

void TZMappingTest::testOlsonToWin()
{
  QCOMPARE( TZMaps::olsonToWinZone( "America/New_York" ),
            QString( "Eastern Time (US & Canada)" ) );
  QCOMPARE( TZMaps::olsonToWinZone( "Australia/Sydney" ),
            QString( "Canberra, Melbourne, Sydney" ) );
  QCOMPARE( TZMaps::olsonToWinZone( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TZMaps::olsonToWinZone( QString() ),
            QString() );
  QCOMPARE( TZMaps::olsonToWinZone( "Europe/London" ),
            QString( "Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London" ) );
  QCOMPARE( TZMaps::olsonToWinZone( "Asia/Kolkata" ),
            QString( "Chennai, Kolkata, Mumbai, New Delhi" ) );
  QCOMPARE( TZMaps::olsonToWinZone( "Pacific/Chatham" ),
            QString() );
}

void TZMappingTest::testOlsonToUtcOffset()
{
  QCOMPARE( TZMaps::olsonToUtcOffset( "NO-SUCH-ZONE" ),
            QString() );
  QCOMPARE( TZMaps::olsonToUtcOffset( "America/Argentina/Catamarca" ),
            QString( "UTC-4" ) );
  QCOMPARE( TZMaps::olsonToUtcOffset( "hawaii" ),
            QString() );
  QCOMPARE( TZMaps::olsonToUtcOffset( "America/New_York" ),
            QString( "UTC-5" ) );
  QCOMPARE( TZMaps::olsonToUtcOffset( QString() ),
            QString() );
}

void TZMappingTest::testUtcOffsetToOlson()
{
  QCOMPARE( TZMaps::utcOffsetToOlson( "UTC+10000" ),
            QString() );
  QCOMPARE( TZMaps::utcOffsetToOlson( "UTC-10" ),
            QString( "Pacific/Honolulu" ) );
  QCOMPARE( TZMaps::utcOffsetToOlson( "utc-10" ),
            QString( "Pacific/Honolulu" ) );
  QCOMPARE( TZMaps::utcOffsetToOlson( "utc-4:30" ),
            QString( "America/Caracas" ) );
  QCOMPARE( TZMaps::utcOffsetToOlson( "UTC+3" ),
            QString( "Europe/Moscow" ) );
  QCOMPARE( TZMaps::utcOffsetToOlson( QString() ),
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
  KTimeZone zonep1( TZMaps::utcOffsetToOlson( "UTC+1" ) );
  KTimeZone zonep2( TZMaps::utcOffsetToOlson( "UTC+2" ) );
  KTimeZone zonep3( TZMaps::utcOffsetToOlson( "UTC+3" ) );
  KTimeZone zonep4( TZMaps::utcOffsetToOlson( "UTC+4" ) );
  KTimeZone zonep5( TZMaps::utcOffsetToOlson( "UTC+5" ) );

  QCOMPARE( abbrevStr( TZMaps::utcOffsetToAbbreviation( "UTC+1" ) ),
            QString( "CET,WAT,WEST" ) );
  QCOMPARE( abbrevStr( TZMaps::utcOffsetToAbbreviation( "UTC+2" ) ),
            QString( "CAT,CEST,EET,IST,SAST" ) );
  QCOMPARE( abbrevStr( TZMaps::utcOffsetToAbbreviation( "UTC+3" ) ),
            QString( "AST,EAT,EEST,MSK" ) );
  QCOMPARE( abbrevStr( TZMaps::utcOffsetToAbbreviation( "UTC+4" ) ),
            QString( "AMT,AST,AZT,GET,MUT,RET,SAMT,SCT" ) );
  QCOMPARE( abbrevStr( TZMaps::utcOffsetToAbbreviation( "UTC+5" ) ),
            QString( "AMST,HMT,PKT,YEKT" ) );
}


