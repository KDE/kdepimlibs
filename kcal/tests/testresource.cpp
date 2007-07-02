/* This file is part of the KDE project
   Copyright (C) 2004 Till Adam <adam@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

#include <kurl.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kinputdialog.h>

#include "kabc/stdaddressbook.h"
#include "kresources/factory.h"

#include "kcal/calendarresources.h"
#include "kcal/resourcecalendar.h"
#include "kcal/icalformat.h"
#include "kcal/event.h"

#include "testresource.h"
#include "testincidencegenerator.h"

int main(int argc, char *argv[])
{
  // Use another directory than the real one, just to keep things clean
  // KDEHOME needs to be writable though, for a ksycoca database
  setenv( "KDEHOME", QFile::encodeName( QDir::homePath() + "/.kde-testresource" ), true );
  setenv( "KDE_FORK_SLAVES", "yes", true ); // simpler, for the final cleanup

  KAboutData aboutData("testresource", 0, ki18n("Part of LibKCal's test suite."), "0.1");
  KCmdLineArgs::init(argc, argv, &aboutData);

  KCmdLineOptions options;
  options.add("resource <type>", ki18n("The resource to test"));
  options.add("configfile <file>", ki18n("Location of a config file for the resource"));
  KCmdLineArgs::addCmdLineOptions( options );

  KComponentData componentData( &aboutData );
  //QCoreApplication app( KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv() );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString type = QString();
  if ( !args->getOption( "resource" ).isEmpty() )
    type = args->getOption( "resource" );
  KConfigGroup *config = 0;
  if ( !args->getOption( "configfile" ).isEmpty() ) {
    KConfig _config( KUrl( args->getOption( "configfile" ) ).url() );
    config = new KConfigGroup( &_config, "FRED" ); //TODO: replace FRED with a real group name
  }
  KCal::TestResource test( type, config );
  test.setup();
  test.runAll();
  test.cleanup();
  kDebug() << "All tests OK." << endl;
  return 0;
}

namespace KCal {

TestResource::TestResource( const QString &type, KConfigGroup *config )
 :m_resource_type( type ), m_config( config ), m_res( 0 )
{}

void TestResource::setup()
{
  CalendarResourceManager *manager = new CalendarResourceManager( "calendar" );
  manager->readConfig();

  if ( m_resource_type.isNull() ) {
    QStringList types = manager->resourceTypeNames();
    QStringList descs = manager->resourceTypeDescriptions();

    QString desc =
      KInputDialog::getItem(
        "Select Resource",
        "Select the resource you wish to test. Test data will be used.",
        descs );

    kDebug() << "Selected Resource: " << desc << endl;
    if ( !desc.isNull() )
      m_resource_type = types[ descs.indexOf( desc ) ];
  }
  assert( !m_resource_type.isNull() );
  /* Either read one from the config file, or create a default one. */
  if ( m_config ) {
    kDebug() << "Reading config from file" << endl;
    KRES::Factory *factory = KRES::Factory::self( "calendar" );
    m_res = dynamic_cast<ResourceCalendar*>( factory->resource( m_resource_type, *m_config ) );
  } else {
    kDebug() << "Creating blank resource" << endl;
    m_res = manager->createResource( m_resource_type );
  }
  assert( m_res );
}

void TestResource::runAll()
{
  testOpenAndClose();
  /* now we can trust it to open correctly */
  m_res->open();
  testResourceAttributes();
  testResourceCalendarAttributes();
  testEventAddRemove();
  testTodoAddRemove();
  testJournalAddRemove();
  m_res->close();
}

bool TestResource::check(const QString& txt, const QString &a, const QString &b)
{
  QString ta = a; QString tb = b;
  if (ta.isEmpty())
    ta.clear();
  if (tb.isEmpty())
    tb.clear();
  if (ta == tb) {
    kDebug() << txt
             << " : checking '" << ta
             << "' against expected value '" << tb
             << "'... " << "ok" << endl;
  }
  else {
    kDebug() << txt
             << " : checking '" << ta
             << "' against expected value '" << tb
             << "'... " << "KO !" << endl;
    cleanup();
    exit(1);
  }
  return true;
}

void TestResource::testOpenAndClose()
{
  kDebug() << k_funcinfo << endl;
  assert( m_res->open() );
  assert( m_res->isOpen() );
  m_res->close();
  assert( !m_res->isOpen() );
}

void TestResource::testResourceAttributes()
{
  kDebug() << k_funcinfo << endl;

  check( "type", m_res->type(), m_resource_type );

  m_res->setReadOnly( true );
  assert( m_res->readOnly() );
  m_res->setReadOnly( false );
  assert( !m_res->readOnly() );

  m_res->setResourceName( "Margarete" );
  check( "name", m_res->resourceName(), "Margarete" );

  m_res->setActive( false );
  assert( !m_res->isActive() );
  m_res->setActive( true );
  assert( m_res->isActive() );
  m_res->dump();
}

void TestResource::testResourceCalendarAttributes()
{
  kDebug() << k_funcinfo << endl;
}


void TestResource::testEventAddRemove()
{
  ICalFormat f;
  kDebug() << k_funcinfo << endl;

  int oldcount = m_res->rawIncidences().count();
  Event *event = makeTestEvent();
  const QString origString = f.toString( event );
  m_res->addEvent( event );
  Event *fromRes = m_res->event( event->uid() );
  assert( fromRes == event );
  const QString fromResString = f.toString( fromRes );
  check( "add", origString, fromResString );
  m_res->deleteEvent( event );
  assert( !m_res->event( event->uid() ) );
  int newcount = m_res->rawIncidences().count();
  assert( oldcount == newcount );
  delete event;
}

void TestResource::testTodoAddRemove()
{
  ICalFormat f;
  kDebug() << k_funcinfo << endl;

  int oldcount = m_res->rawIncidences().count();
  Todo *todo = makeTestTodo();
  const QString origString = f.toString( todo );
  m_res->addTodo( todo );
  Todo *fromRes = m_res->todo( todo->uid() );
  assert( fromRes == todo );
  const QString fromResString = f.toString( fromRes );
  check( "add", origString, fromResString );
  m_res->deleteTodo( todo );
  assert( !m_res->todo( todo->uid() ) );
  int newcount = m_res->rawIncidences().count();
  assert( oldcount == newcount );
  delete todo;
}

void TestResource::testJournalAddRemove()
{
  ICalFormat f;
  kDebug() << k_funcinfo << endl;

  int oldcount = m_res->rawIncidences().count();
  Journal *journal = makeTestJournal();
  const QString origString = f.toString( journal );
  m_res->addJournal( journal );
  Journal *fromRes = m_res->journal( journal->uid() );
  assert( fromRes == journal );
  const QString fromResString = f.toString( fromRes );
  check( "add", origString, fromResString );
  m_res->deleteJournal( journal );
  assert( !m_res->journal( journal->uid() ) );
  int newcount = m_res->rawIncidences().count();
  assert( oldcount == newcount );
  delete journal;
}

void TestResource::cleanup()
{
  kDebug() << k_funcinfo << endl;
}

}

#include "testresource.moc"
