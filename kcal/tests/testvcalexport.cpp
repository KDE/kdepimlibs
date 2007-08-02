/*
  This file is part of the kcal library.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "kcal/calendarlocal.h"
#include "kcal/vcalformat.h"
#include "kcal/filestorage.h"

#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

using namespace KCal;

int main( int argc, char **argv )
{
  KAboutData aboutData( "testvcalexport", 0,
                        ki18n( "Part of LibKCal's test suite. Checks if export "
                               "to vCalendar still works correctly."), "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add( "verbose", ki18n( "Verbose output" ) );
  options.add( "+input", ki18n( "Name of input file" ) );
  options.add( "+output", ki18n( "Name of output file" ) );
  KCmdLineArgs::addCmdLineOptions( options );

  KComponentData componentData( &aboutData );
  //QCoreApplication app( KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv() );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() != 2 ) {
    args->usage( "Wrong number of arguments." );
  }

  QString input = args->arg( 0 );
  QString output = args->arg( 1 );

  QFileInfo outputFileInfo( output );
  output = outputFileInfo.absoluteFilePath();

  kDebug(5800) << "Input file:" << input;
  kDebug(5800) << "Output file:" << output;

  CalendarLocal cal( KDateTime::UTC );

  if ( !cal.load( input ) ) {
    return 1;
  }
  QString tz = cal.nonKDECustomProperty( "X-LibKCal-Testsuite-OutTZ" );
  if ( !tz.isEmpty() ) {
    cal.setViewTimeZoneId( tz );
  }
  FileStorage storage( &cal, output, new VCalFormat );
  if ( !storage.save() ) {
    return 1;
  }

  return 0;
}

