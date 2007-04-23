/*
    This file is part of the testing framework for the kcal library.

    Copyright (c) 2005 Adriaan de Groot <groot@kde.org>

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

#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "kcal/calendarlocal.h"
#include "kcal/versit/vobject.h"

using namespace KCal;

static const KCmdLineOptions options[] =
{
  {"verbose", "Verbose output", 0},
  KCmdLineLastOption
};

int main(int argc,char **argv)
{
  KAboutData aboutData("testcalendar","Test Calendar","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions( options );

  KComponentData componentData( &aboutData );
  //QCoreApplication app( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv() );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  Q_UNUSED(args)

  CalendarLocal cal( QLatin1String("UTC") );

  QString file = QLatin1String( INPUT );
  if (!cal.load( file ) ) {
    kError() << "Can't load " << file << endl;
    return 1;
  }

  QString uid = QLatin1String("KOrganizer-1345486115.965");
  Event *e = cal.event( uid );
  if (!e) {
    kError() << "No event " << uid << endl;
    return 1;
  }

/*  if (e->hasStartDate()) {
    KDateTime d = e->dtStart();
    kDebug() << "Event starts " << d << endl;
  }
*/

  kDebug() << "Event description " << e->summary() << endl;

  if (e->hasEndDate()) {
    KDateTime d = e->dtEnd();
    kDebug() << "Event ends " << d << endl;
  }


  QString pilotId = e->nonKDECustomProperty( KPilotIdProp );
  if (!pilotId.isEmpty()) {
    kDebug() << "Pilot ID = " << pilotId << endl;
  } else {
    kError() << "No Pilot ID" << endl;
    return 1;
  }

  return 0;
}
