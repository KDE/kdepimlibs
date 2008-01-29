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

#include "resourcelocaldir.h"
#include "resourcelocaldir_p.h"
#include "calendarlocal.h"
#include "incidence.h"
#include "event.h"
#include "todo.h"
#include "journal.h"

#include "kresources/configwidget.h"

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>

#include <QtCore/QString>
#include <QtCore/QDir>

#include <typeinfo>
#include <stdlib.h>

#include "resourcelocaldir.moc"

using namespace KCal;

ResourceLocalDir::ResourceLocalDir()
  : ResourceCached(), d( new KCal::ResourceLocalDir::Private )
{
  d->init( this );
}

ResourceLocalDir::ResourceLocalDir( const KConfigGroup &group )
  : ResourceCached( group ), d( new KCal::ResourceLocalDir::Private )
{
  readConfig( group );
  d->init( this );
}

ResourceLocalDir::ResourceLocalDir( const QString &dirName )
  : ResourceCached(), d( new KCal::ResourceLocalDir::Private( dirName ) )
{
  d->init( this );
}

void ResourceLocalDir::readConfig( const KConfigGroup &group )
{
  QString url = group.readPathEntry( "CalendarURL", QString() );
  d->mURL = KUrl( url );
}

void ResourceLocalDir::writeConfig( KConfigGroup &group )
{
  kDebug();

  ResourceCalendar::writeConfig( group );

  group.writePathEntry( "CalendarURL", d->mURL.prettyUrl() );
}

void ResourceLocalDir::Private::init( ResourceLocalDir *rdir )
{
  rdir->setType( "dir" );

  rdir->setSavePolicy( SaveDelayed );

  rdir->connect( &mDirWatch, SIGNAL( dirty( const QString & ) ),
                 SLOT( reload( const QString & ) ) );
  rdir->connect( &mDirWatch, SIGNAL( created( const QString & ) ),
                 SLOT( reload( const QString & ) ) );
  rdir->connect( &mDirWatch, SIGNAL( deleted( const QString & ) ),
                 SLOT( reload( const QString & ) ) );

  mLock = new KABC::Lock( mURL.path() );

  mDirWatch.addDir( mURL.path(), KDirWatch::WatchFiles );
  mDirWatch.startScan();
}

ResourceLocalDir::~ResourceLocalDir()
{
  close();

  delete d->mLock;
  delete d;
}

bool ResourceLocalDir::doLoad( bool )
{
  kDebug();

  calendar()->close();
  QString dirName = d->mURL.path();
  bool success = true;

  if ( !( KStandardDirs::exists( dirName ) || KStandardDirs::exists( dirName + '/' ) ) ) {
    kDebug() << "Directory '" << dirName << "' doesn't exist yet. Creating it.";

    // Create the directory. Use 0775 to allow group-writable if the umask
    // allows it (permissions will be 0775 & ~umask). This is desired e.g. for
    // group-shared directories!
    success = KStandardDirs::makeDir( dirName, 0775 );
  } else {

    kDebug() << dirName;
    QDir dir( dirName );

    QStringList entries = dir.entryList( QDir::Files | QDir::Readable );

    QStringList::ConstIterator it;
    for ( it = entries.begin(); it != entries.end(); ++it ) {
      if ( (*it).endsWith( '~' ) ) { // is backup file, ignore it
        continue;
      }

      QString fileName = dirName + '/' + *it;
      kDebug() << " read '" << fileName << "'";
      CalendarLocal cal( calendar()->timeSpec() );
      if ( !doFileLoad( cal, fileName ) ) {
        success = false;
      }
    }
  }

  return success;
}

bool ResourceLocalDir::doFileLoad( CalendarLocal &cal, const QString &fileName )
{
  if ( !cal.load( fileName ) ) {
    return false;
  }
  Incidence::List incidences = cal.rawIncidences();
  Incidence::List::ConstIterator it;
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    Incidence *i = *it;
    if ( i ) {
      calendar()->addIncidence( i->clone() );
    }
  }
  return true;
}

bool ResourceLocalDir::doSave( bool syncCache )
{
  Q_UNUSED( syncCache );
  Incidence::List list;
  bool success = true;

  list = addedIncidences();
  list += changedIncidences();

  for ( Incidence::List::iterator it = list.begin(); it != list.end(); ++it ) {
    if ( !doSave( *it ) ) {
      success = false;
    }
  }

  return success;
}

bool ResourceLocalDir::doSave( bool, Incidence *incidence )
{
  d->mDirWatch.stopScan();  // do prohibit the dirty() signal and a following reload()

  QString fileName = d->mURL.path() + '/' + incidence->uid();
  kDebug() << "writing '" << fileName << "'";

  CalendarLocal cal( calendar()->timeSpec() );
  cal.addIncidence( incidence->clone() );
  const bool ret = cal.save( fileName );

  d->mDirWatch.startScan();

  return ret;
}

KABC::Lock *ResourceLocalDir::lock()
{
  return d->mLock;
}

void ResourceLocalDir::reload( const QString &file )
{
  kDebug();

  if ( !isOpen() ) {
    return;
  }

  kDebug() << "  File: '" << file << "'";

  calendar()->close();
  load();

  emit resourceChanged( this );
}

bool ResourceLocalDir::deleteEvent( Event *event )
{
  kDebug();
  if ( d->deleteIncidenceFile( event ) ) {
    return calendar()->deleteEvent( event );
  } else {
    return false;
  }
}

void ResourceLocalDir::deleteAllEvents()
{
  calendar()->deleteAllEvents();
}

bool ResourceLocalDir::deleteTodo( Todo *todo )
{
  if ( d->deleteIncidenceFile( todo ) ) {
    return calendar()->deleteTodo( todo );
  } else {
    return false;
  }
}

void ResourceLocalDir::deleteAllTodos()
{
  calendar()->deleteAllTodos();
}

bool ResourceLocalDir::deleteJournal( Journal *journal )
{
  if ( d->deleteIncidenceFile( journal ) ) {
    return calendar()->deleteJournal( journal );
  } else {
    return false;
  }
}

void ResourceLocalDir::deleteAllJournals()
{
  calendar()->deleteAllJournals();
}

void ResourceLocalDir::dump() const
{
  ResourceCalendar::dump();
  kDebug() << "  Url:" << d->mURL.url();
}

bool ResourceLocalDir::Private::deleteIncidenceFile( Incidence *incidence )
{
  QFile file( mURL.path() + '/' + incidence->uid() );
  if ( !file.exists() ) {
    return true;
  }

  mDirWatch.stopScan();
  bool removed = file.remove();
  mDirWatch.startScan();
  return removed;
}
