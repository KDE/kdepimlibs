/*
  This file is part of the kcal library.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2009 Sergio Martins <iamsergio@gmail.com>

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

#include <kcal/assignmentvisitor.h>
#include <kcal/comparisonvisitor.h>
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>

#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <typeinfo>
#include <stdlib.h>

#include "resourcelocaldir.moc"
#include "resourcelocaldir_p.moc"

using namespace KCal;

ResourceLocalDir::ResourceLocalDir()
  : ResourceCached(), d( new KCal::ResourceLocalDir::Private( this ) )
{
  d->init();
}

ResourceLocalDir::ResourceLocalDir( const KConfigGroup &group )
  : ResourceCached( group ), d( new KCal::ResourceLocalDir::Private( this ) )
{
  readConfig( group );
  d->init();
}

ResourceLocalDir::ResourceLocalDir( const QString &dirName )
  : ResourceCached(), d( new KCal::ResourceLocalDir::Private( dirName, this ) )
{
  d->init();
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

void ResourceLocalDir::Private::init( )
{
  mResource->setType( "dir" );

  mResource->setSavePolicy( SaveDelayed );

  connect( &mDirWatch, SIGNAL( dirty( const QString & ) ),
           this, SLOT( updateIncidenceInCalendar( const QString & ) ) );
  connect( &mDirWatch, SIGNAL( created( const QString & ) ),
           this, SLOT( addIncidenceToCalendar( const QString & ) ) );
  connect( &mDirWatch, SIGNAL( deleted( const QString & ) ),
           this, SLOT( deleteIncidenceFromCalendar( const QString & ) ) );

  connect ( this, SIGNAL(resourceChanged( ResourceCalendar *)),
            mResource, SIGNAL(resourceChanged( ResourceCalendar *)) );

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

bool ResourceLocalDir::doOpen()
{
  QFileInfo dirInfo( d->mURL.path() );
  return dirInfo.isDir() && dirInfo.isReadable() &&
    ( dirInfo.isWritable() || readOnly() );
}

bool ResourceLocalDir::doLoad( bool )
{
  kDebug();

  calendar()->close();
  QString dirName = d->mURL.path();

  if ( !( KStandardDirs::exists( dirName ) || KStandardDirs::exists( dirName + '/' ) ) ) {
    kDebug() << "Directory '" << dirName << "' doesn't exist yet. Creating it.";

    // Create the directory. Use 0775 to allow group-writable if the umask
    // allows it (permissions will be 0775 & ~umask). This is desired e.g. for
    // group-shared directories!
    return KStandardDirs::makeDir( dirName, 0775 );
  }

  // The directory exists. Now try to open (the files in) it.
  kDebug() << dirName;
  QFileInfo dirInfo( dirName );
  if ( !( dirInfo.isDir() && dirInfo.isReadable() &&
          ( dirInfo.isWritable() || readOnly() ) ) ) {
    return false;
  }

  QDir dir( dirName );
  const QStringList entries = dir.entryList( QDir::Files | QDir::Readable );

  bool success = true;

  foreach ( const QString &entry, entries ) {
    if ( d->isTempFile( entry ) ) {
      continue;  // backup or temporary file, ignore it
    }

    const QString fileName = dirName + '/' + entry;
    kDebug() << " read '" << fileName << "'";
    CalendarLocal cal( calendar()->timeSpec() );
    if ( !doFileLoad( cal, fileName ) ) {
      success = false;
    }
  }

  return success;
}

bool ResourceLocalDir::doFileLoad( CalendarLocal &cal, const QString &fileName )
{
  return d->doFileLoad( cal, fileName, false );
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
  if ( d->mDeletedIncidences.contains( incidence ) ) {
    d->mDeletedIncidences.removeAll( incidence );
    return true;
  }

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
void ResourceLocalDir::reload( const QString &file ) {
  Q_UNUSED( file );
}

bool ResourceLocalDir::deleteEvent( Event *event )
{
  kDebug();
  if ( d->deleteIncidenceFile( event ) ) {
    if ( calendar()->deleteEvent( event ) ) {
      d->mDeletedIncidences.append( event );
      return true;
    } else {
      return false;
    }
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
    if ( calendar()->deleteTodo( todo ) ) {
      d->mDeletedIncidences.append( todo );
      return true;
    } else {
      return false;
    }
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
    if ( calendar()->deleteJournal( journal ) ) {
      d->mDeletedIncidences.append( journal );
      return true;
    } else {
      return false;
    }
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

bool ResourceLocalDir::Private::isTempFile( const QString &fileName ) const
{
  return fileName.contains( QRegExp( "(~|\\.new|\\.tmp)$" ) )       ||
         QFileInfo( fileName ).fileName().startsWith( "qt_temp." )  ||
         fileName == mURL.path();
}

void ResourceLocalDir::Private::addIncidenceToCalendar( const QString &file )
{

  if ( mResource->isOpen() &&
       !isTempFile( file ) &&
       !mResource->calendar()->incidence( getUidFromFileName( file ) ) ) {

    CalendarLocal cal( mResource->calendar()->timeSpec() );
    if ( doFileLoad( cal, file, true ) ) {
      emit resourceChanged( mResource );
    }
  }
}

void ResourceLocalDir::Private::updateIncidenceInCalendar( const QString &file )
{
  if ( mResource->isOpen() && !isTempFile( file ) ) {
    CalendarLocal cal( mResource->calendar()->timeSpec() );
    if ( doFileLoad( cal, file, true ) ) {
      emit resourceChanged( mResource );
    }
  }
}

QString ResourceLocalDir::Private::getUidFromFileName( const QString &fileName )
{
  return QFileInfo( fileName ).fileName();
}

void ResourceLocalDir::Private::deleteIncidenceFromCalendar( const QString &file )
{

  if ( mResource->isOpen() && !isTempFile( file ) ) {
    Incidence *inc = mResource->calendar()->incidence( getUidFromFileName( file ) );

    if ( inc ) {
      mResource->calendar()->deleteIncidence( inc );
      emit resourceChanged( mResource );
    }
  }
}

bool ResourceLocalDir::Private::doFileLoad( CalendarLocal &cal,
                                            const QString &fileName,
                                            const bool replace )
{
  if ( !cal.load( fileName ) ) {
    return false;
  }
  Incidence::List incidences = cal.rawIncidences();
  Incidence::List::ConstIterator it;
  Incidence *inc;
  ComparisonVisitor compVisitor;
  AssignmentVisitor assVisitor;
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    Incidence *i = *it;
    if ( i ) {
      // should we replace, and does the incidence exist in calendar?
      if ( replace && ( inc = mResource->calendar()->incidence( i->uid() ) ) ) {
        if ( compVisitor.compare( i, inc ) ) {
          // no need to do anything
          return false;
        } else {
          inc->startUpdates();

          bool assignResult = assVisitor.assign( inc, i );

          if ( assignResult ) {
            if ( !inc->relatedToUid().isEmpty() ) {
              QString uid = inc->relatedToUid();
              inc->setRelatedTo( mResource->calendar()->incidence( uid ) );
            }
            inc->updated();
            inc->endUpdates();
          } else {
            inc->endUpdates();
            kWarning( 5800 ) << "Incidence (uid=" << inc->uid()
                             << ", summary=" << inc->summary()
                             << ") changed type. Replacing it.";

            mResource->calendar()->deleteIncidence( inc );
            delete inc;
            mResource->calendar()->addIncidence( i->clone() );
          }
        }
      } else {
        mResource->calendar()->addIncidence( i->clone() );
      }
    }
  }
  return true;
}
