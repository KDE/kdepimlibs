/*
    This file is part of the kcal library.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "calendarlocal.h"
#include "icalformat.h"
#include "qtopiaformat.h"

#include <kaboutdata.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>

#include <iostream>

using namespace KCal;

int main(int argc,char **argv)
{
  KAboutData aboutData("convertqtopia", 0,ki18n("Qtopia calendar file converter"),"0.1");
  aboutData.addAuthor(ki18n("Cornelius Schumacher"), KLocalizedString(), "schumacher@kde.org");

  KCmdLineArgs::init(argc,argv,&aboutData);

  KCmdLineOptions options;
  options.add("q");
  options.add("qtopia2icalendar", ki18n("Convert Qtopia calendar file to iCalendar"));
  options.add("i");
  options.add("icalendar2qtopia", ki18n("Convert iCalendar to iCalendar"));
  options.add("o");
  options.add("output <file>", ki18n("Output file"));
  options.add("+input", ki18n("Input file"));
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  bool sourceQtopia = false;
  bool sourceIcalendar = false;

  if ( args->isSet( "qtopia2icalendar" ) ) {
    sourceQtopia = true;
  }

  if ( args->isSet( "icalendar2qtopia" ) ) {
    sourceIcalendar = true;
  }

  if ( sourceQtopia && sourceIcalendar ) {
    KCmdLineArgs::usageError(
        i18n("Please specify only one of the conversion options.") );
  }
  if ( !sourceQtopia && !sourceIcalendar ) {
    KCmdLineArgs::usageError(
        i18n("You have to specify one conversion option.") );
  }

  if ( args->count() != 1 ) {
    KCmdLineArgs::usageError( i18n("Error: No input file.") );
  }

  QString inputFile = args->arg( 0 );

  QString outputFile;
  if ( args->isSet("output") ) outputFile = args->getOption( "output" );

  kDebug(5800) << "Input File: '" << inputFile << "'" << endl;
  kDebug(5800) << "Output File: '" << outputFile << "'" << endl;

  if ( sourceQtopia ) {
    CalendarLocal cal( QLatin1String( "UTC" ) );
    
    QtopiaFormat qtopiaFormat;
    qtopiaFormat.load( &cal, inputFile );

    ICalFormat icalendarFormat;
    if ( outputFile.isEmpty() ) {
      QString out = icalendarFormat.toString( &cal );
      std::cout << out.toLocal8Bit().constData() << std::endl;
    } else {
      bool success = icalendarFormat.save( &cal, outputFile );
      if ( !success ) {
        std::cerr << i18n( "Error saving to '%1'.", outputFile ).toLocal8Bit().constData()
                  << std::endl;
        return 1;
      }
    }
  }
  
  if ( sourceIcalendar ) {
    std::cerr << "Not implemented yet." << std::endl;
    return 1;
  }
}
