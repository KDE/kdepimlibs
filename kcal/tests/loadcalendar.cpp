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

#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <QtCore/QByteArray>

#include "kcal/calendarlocal.h"

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

  Q_UNUSED( args );

  CalendarLocal cal( KDateTime::UTC );

  cal.load("cal");

  KCal::Todo::List todoList;
  KCal::Todo::List::ConstIterator todo;

  // Build dictionary to look up Task object from Todo uid.  Each task is a
  // QListViewItem, and is initially added with the view as the parent.
  todoList = cal.rawTodos();
  kDebug() << (*todoList.begin())->uid() << endl;
  QString result=(*todoList.begin())->customProperty(QByteArray("karm"),QByteArray("totalTaskTime"));
  kDebug() << result << endl;
  if (result != QString("a,b"))
  {
    kDebug() << "The string a,b was expected, but given was " << result << endl;
    return 1;
  }
  else
  {
    kDebug() << "Test passed" << endl;
  }
}
