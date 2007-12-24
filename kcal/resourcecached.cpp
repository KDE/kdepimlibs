/*
  This file is part of the kcal library.

  Copyright © 2006 by David Jarvie <software@astrojar.org.uk>
  Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "resourcecached.h"
#include "calendarlocal.h"
#include "event.h"
#include "exceptions.h"
#include "incidence.h"
#include "journal.h"
#include "todo.h"

#include "kresources/idmapper.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>

#include <QtCore/QDateTime>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QTimer>

#include "resourcecached.moc"

using namespace KCal;

//@cond PRIVATE
class ResourceCached::Private
{
  public:
    Private()
      : mCalendar( QLatin1String( "UTC" ) ),
        mReloadPolicy( ResourceCached::ReloadNever ),
        mReloadInterval( 10 ),
        mInhibitReload( false ),
        mReloaded( false ),
        mSavePending( false ),
        mSavePolicy( ResourceCached::SaveNever ),
        mSaveInterval( 10 ),
        mIdMapper( "kcal/uidmaps/" )
    {}

    CalendarLocal mCalendar;

    int mReloadPolicy;
    int mReloadInterval;
    QTimer mReloadTimer;
    bool mInhibitReload;   // true to prevent downloads by load(DefaultCache)
    bool mReloaded;        // true once it has been downloaded
    bool mSavePending;     // true if a save of changes has been scheduled on the timer

    int mSavePolicy;
    int mSaveInterval;
    QTimer mSaveTimer;

    KDateTime mLastLoad;
    KDateTime mLastSave;

    QMap<KCal::Incidence *,bool> mAddedIncidences;
    QMap<KCal::Incidence *,bool> mChangedIncidences;
    QMap<KCal::Incidence *,bool> mDeletedIncidences;

    KRES::IdMapper mIdMapper;
};
//@endcond

ResourceCached::ResourceCached()
  : ResourceCalendar(),
    d( new Private )
{
  connect( &d->mReloadTimer, SIGNAL( timeout() ), SLOT( slotReload() ) );
  connect( &d->mSaveTimer, SIGNAL( timeout() ), SLOT( slotSave() ) );
}

ResourceCached::ResourceCached( const KConfigGroup &group )
  : ResourceCalendar( group ),
    d( new Private )
{
  connect( &d->mReloadTimer, SIGNAL( timeout() ), SLOT( slotReload() ) );
  connect( &d->mSaveTimer, SIGNAL( timeout() ), SLOT( slotSave() ) );
}

ResourceCached::~ResourceCached()
{
  delete d;
}

CalendarLocal *ResourceCached::calendar() const
{
  return &d->mCalendar;
}

bool ResourceCached::defaultReloadInhibited() const
{
  return d->mInhibitReload;
}

bool ResourceCached::reloaded() const
{
  return d->mReloaded;
}

void ResourceCached::setReloaded( bool done )
{
  d->mReloaded = done;
}

void ResourceCached::setReloadPolicy( int i )
{
  d->mReloadPolicy = i;

  setupReloadTimer();
}

int ResourceCached::reloadPolicy() const
{
  return d->mReloadPolicy;
}

void ResourceCached::setReloadInterval( int minutes )
{
  d->mReloadInterval = minutes;
}

int ResourceCached::reloadInterval() const
{
  return d->mReloadInterval;
}

bool ResourceCached::inhibitDefaultReload( bool inhibit )
{
  if ( inhibit == d->mInhibitReload ) {
    return false;
  }
  d->mInhibitReload = inhibit;
  return true;
}

void ResourceCached::setSavePolicy( int i )
{
  d->mSavePolicy = i;

  setupSaveTimer();
}

int ResourceCached::savePolicy() const
{
  return d->mSavePolicy;
}

void ResourceCached::setSaveInterval( int minutes )
{
  d->mSaveInterval = minutes;
}

int ResourceCached::saveInterval() const
{
  return d->mSaveInterval;
}

void ResourceCached::readConfig( const KConfigGroup &group )
{
  d->mReloadPolicy = group.readEntry( "ReloadPolicy", int(ReloadNever) );
  d->mReloadInterval = group.readEntry( "ReloadInterval", 10 );

  d->mSaveInterval = group.readEntry( "SaveInterval", 10 );
  d->mSavePolicy = group.readEntry( "SavePolicy", int(SaveNever) );

  QDateTime curDt = QDateTime::currentDateTime();
  QDateTime dt = group.readEntry( "LastLoad", curDt );
  d->mLastLoad = KDateTime( dt, KDateTime::UTC );
  dt = group.readEntry( "LastSave", curDt );
  d->mLastSave = KDateTime( dt, KDateTime::UTC );

  setupSaveTimer();
  setupReloadTimer();
}

void ResourceCached::setupSaveTimer()
{
  if ( d->mSavePolicy == SaveInterval ) {
    kDebug(5800) << "ResourceCached::setSavePolicy(): start save timer (interval"
              << d->mSaveInterval << "minutes).";
    d->mSaveTimer.start( d->mSaveInterval * 60 * 1000 ); // n minutes
  } else {
    d->mSaveTimer.stop();
  }
}

void ResourceCached::setupReloadTimer()
{
  if ( d->mReloadPolicy == ReloadInterval ) {
    kDebug(5800) << "ResourceCached::setSavePolicy(): start reload timer (interval"
                 << d->mReloadInterval << "minutes)";
    d->mReloadTimer.start( d->mReloadInterval * 60 * 1000 ); // n minutes
  } else {
    d->mReloadTimer.stop();
  }
}

void ResourceCached::writeConfig( KConfigGroup &group )
{
  group.writeEntry( "ReloadPolicy", d->mReloadPolicy );
  group.writeEntry( "ReloadInterval", d->mReloadInterval );

  group.writeEntry( "SavePolicy", d->mSavePolicy );
  group.writeEntry( "SaveInterval", d->mSaveInterval );

  group.writeEntry( "LastLoad", d->mLastLoad.toUtc().dateTime() );
  group.writeEntry( "LastSave", d->mLastSave.toUtc().dateTime() );
}

bool ResourceCached::addEvent( Event *event )
{
  return d->mCalendar.addEvent( event );
}

// probably not really efficient, but...it works for now.
bool ResourceCached::deleteEvent( Event *event )
{
  kDebug(5800) << "ResourceCached::deleteEvent";

  return d->mCalendar.deleteEvent( event );
}

void ResourceCached::deleteAllEvents()
{
  d->mCalendar.deleteAllEvents();
}

Event *ResourceCached::event( const QString &uid )
{
  return d->mCalendar.event( uid );
}

Event::List ResourceCached::rawEventsForDate( const QDate &qd, const KDateTime::Spec &timespec,
                                              EventSortField sortField,
                                              SortDirection sortDirection )
{
  Event::List list = d->mCalendar.rawEventsForDate( qd, timespec, sortField, sortDirection );

  return list;
}

Event::List ResourceCached::rawEvents( const QDate &start, const QDate &end,
                                       const KDateTime::Spec &timespec, bool inclusive )
{
  return d->mCalendar.rawEvents( start, end, timespec, inclusive );
}

Event::List ResourceCached::rawEventsForDate( const KDateTime &kdt )
{
  return d->mCalendar.rawEventsForDate( kdt );
}

Event::List ResourceCached::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  return d->mCalendar.rawEvents( sortField, sortDirection );
}

bool ResourceCached::addTodo( Todo *todo )
{
  return d->mCalendar.addTodo( todo );
}

bool ResourceCached::deleteTodo( Todo *todo )
{
  return d->mCalendar.deleteTodo( todo );
}

void ResourceCached::deleteAllTodos()
{
  d->mCalendar.deleteAllTodos();
}

bool ResourceCached::deleteJournal( Journal *journal )
{
  return d->mCalendar.deleteJournal( journal );
}

void ResourceCached::deleteAllJournals()
{
  d->mCalendar.deleteAllJournals();
}

Todo::List ResourceCached::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
  return d->mCalendar.rawTodos( sortField, sortDirection );
}

Todo *ResourceCached::todo( const QString &uid )
{
  return d->mCalendar.todo( uid );
}

Todo::List ResourceCached::rawTodosForDate( const QDate &date )
{
  return d->mCalendar.rawTodosForDate( date );
}

bool ResourceCached::addJournal( Journal *journal )
{
  kDebug(5800) << "Adding Journal on" << journal->dtStart().toString();

  return d->mCalendar.addJournal( journal );
}

Journal *ResourceCached::journal( const QString &uid )
{
  return d->mCalendar.journal( uid );
}

Journal::List ResourceCached::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  return d->mCalendar.rawJournals( sortField, sortDirection );
}

Journal::List ResourceCached::rawJournalsForDate( const QDate &date )
{
  return d->mCalendar.rawJournalsForDate( date );
}

Alarm::List ResourceCached::alarmsTo( const KDateTime &to )
{
  return d->mCalendar.alarmsTo( to );
}

Alarm::List ResourceCached::alarms( const KDateTime &from, const KDateTime &to )
{
//  kDebug(5800) << "ResourceCached::alarms(" << from.toString() << "-" << to.toString() << ")";

  return d->mCalendar.alarms( from, to );
}

void ResourceCached::setTimeSpec( const KDateTime::Spec &timeSpec )
{
  d->mCalendar.setTimeSpec( timeSpec );
}

KDateTime::Spec ResourceCached::timeSpec() const
{
  return d->mCalendar.timeSpec();
}

void ResourceCached::setTimeZoneId( const QString &tzid )
{
  d->mCalendar.setTimeZoneId( tzid );
}

QString ResourceCached::timeZoneId() const
{
  return d->mCalendar.timeZoneId();
}

void ResourceCached::shiftTimes( const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec )
{
  d->mCalendar.shiftTimes( oldSpec, newSpec );
}

void ResourceCached::clearChanges()
{
  d->mAddedIncidences.clear();
  d->mChangedIncidences.clear();
  d->mDeletedIncidences.clear();
}

bool ResourceCached::load( CacheAction action )
{
  kDebug(5800) << "Loading resource" << resourceName();

  setReceivedLoadError( false );

  bool success = true;
  if ( !isOpen() ) {
    success = open();
  }
  if ( success ) {
    bool update = false;
    switch ( action ) {
    case DefaultCache:
      if ( !d->mReloaded && !d->mInhibitReload ) {
        update = checkForReload();
      }
      break;
    case NoSyncCache:
      break;
    case SyncCache:
      update = true;
      break;
    }
    success = doLoad( update );
  }
  if ( !success && !receivedLoadError() ) {
    loadError();
  }

  // If the resource is read-only, we need to set its incidences to read-only,
  // too. This can't be done at a lower-level, since the read-only setting
  // happens at this level
  if ( !noReadOnlyOnLoad() && readOnly() ) {
    Incidence::List incidences( rawIncidences() );
    Incidence::List::Iterator it;
    for ( it = incidences.begin(); it != incidences.end(); ++it ) {
      (*it)->setReadOnly( true );
    }
  }

  kDebug(5800) << "Done loading resource" << resourceName();

  return success;
}

bool ResourceCached::load()
{
  return load( SyncCache );
}

bool ResourceCached::loadFromCache()
{
  setIdMapperIdentifier();
  d->mIdMapper.load();

  if ( !KStandardDirs::exists( cacheFile() ) ) {
    return false;
  }
  d->mCalendar.load( cacheFile() );
  if ( !noReadOnlyOnLoad() && readOnly() ) {
    Incidence::List incidences( rawIncidences() );
    Incidence::List::Iterator it;
    for ( it = incidences.begin(); it != incidences.end(); ++it ) {
      (*it)->setReadOnly( true );
    }
  }
  return true;
}

bool ResourceCached::save( CacheAction action, Incidence *incidence )
{
  d->mSavePending = false;
  if ( saveInhibited() ) {
    return true;
  }
  if ( !readOnly() ) {
    kDebug(5800) << "Save resource" << resourceName();

    setReceivedSaveError( false );

    if ( !isOpen() ) {
      return true;
    }
    bool upload = false;
    switch ( action ) {
    case DefaultCache:
      upload = checkForSave();
      break;
    case NoSyncCache:
      break;
    case SyncCache:
      upload = true;
      break;
    }
    bool success = incidence ? doSave( upload, incidence ) : doSave( upload );
    if ( !success && !receivedSaveError() ) {
      saveError();
    }
    return success;
  } else {
    // Read-only, just don't save...
    kDebug(5800) << "Don't save read-only resource" << resourceName();
    return true;
  }
}

bool ResourceCached::save( Incidence *incidence )
{
  return save( SyncCache, incidence );
}

bool ResourceCached::doSave( bool syncCache, Incidence *incidence )
{
  Q_UNUSED( incidence );
  return doSave( syncCache );
}

void ResourceCached::saveToCache()
{
  kDebug(5800) << "ResourceCached::saveToCache():" << cacheFile();

  setIdMapperIdentifier();
  d->mIdMapper.save();

  d->mCalendar.save( cacheFile() );
}

void ResourceCached::setIdMapperIdentifier()
{
  d->mIdMapper.setIdentifier( type() + '_' + identifier() );
}

void ResourceCached::clearCache()
{
  d->mCalendar.close();
}

void ResourceCached::cleanUpEventCache( const Event::List &eventList )
{
  CalendarLocal calendar ( QLatin1String( "UTC" ) );

  if ( KStandardDirs::exists( cacheFile() ) ) {
    calendar.load( cacheFile() );
  } else {
    return;
  }

  Event::List list = calendar.events();
  Event::List::ConstIterator cacheIt, it;
  for ( cacheIt = list.begin(); cacheIt != list.end(); ++cacheIt ) {
    bool found = false;
    for ( it = eventList.begin(); it != eventList.end(); ++it ) {
      if ( (*it)->uid() == (*cacheIt)->uid() ) {
        found = true;
      }
    }

    if ( !found ) {
      d->mIdMapper.removeRemoteId( d->mIdMapper.remoteId( (*cacheIt)->uid() ) );
      Event *event = d->mCalendar.event( (*cacheIt)->uid() );
      if ( event ) {
        d->mCalendar.deleteEvent( event );
      }
    }
  }

  calendar.close();
}

void ResourceCached::cleanUpTodoCache( const Todo::List &todoList )
{
  CalendarLocal calendar ( QLatin1String( "UTC" ) );

  if ( KStandardDirs::exists( cacheFile() ) ) {
    calendar.load( cacheFile() );
  } else {
    return;
  }

  Todo::List list = calendar.todos();
  Todo::List::ConstIterator cacheIt, it;
  for ( cacheIt = list.begin(); cacheIt != list.end(); ++cacheIt ) {

    bool found = false;
    for ( it = todoList.begin(); it != todoList.end(); ++it ) {
      if ( (*it)->uid() == (*cacheIt)->uid() ) {
        found = true;
      }
    }

    if ( !found ) {
      d->mIdMapper.removeRemoteId( d->mIdMapper.remoteId( (*cacheIt)->uid() ) );
      Todo *todo = d->mCalendar.todo( (*cacheIt)->uid() );
      if ( todo ) {
        d->mCalendar.deleteTodo( todo );
      }
    }
  }

  calendar.close();
}

KRES::IdMapper &ResourceCached::idMapper()
{
  return d->mIdMapper;
}

QString ResourceCached::cacheFile() const
{
  return KStandardDirs::locateLocal( "cache", "kcal/kresources/" + identifier() );
}

QString ResourceCached::changesCacheFile( const QString &type ) const
{
  return KStandardDirs::locateLocal( "cache", "kcal/changescache/" + identifier() + '_' + type );
}

void ResourceCached::saveChangesCache( const QMap<Incidence *, bool> &map, const QString &type )
{
  CalendarLocal calendar ( QLatin1String( "UTC" ) );

  bool isEmpty = true;
  QMap<Incidence *,bool>::ConstIterator it;
  for ( it = map.begin(); it != map.end(); ++it ) {
    isEmpty = false;
    calendar.addIncidence( it.key()->clone() );
  }

  if ( !isEmpty ) {
    calendar.save( changesCacheFile( type ) );
  } else {
    QFile file( changesCacheFile( type ) );
    file.remove();
  }

  calendar.close();
}

void ResourceCached::saveChangesCache()
{
  saveChangesCache( d->mAddedIncidences, "added" );
  saveChangesCache( d->mDeletedIncidences, "deleted" );
  saveChangesCache( d->mChangedIncidences, "changed" );
}

void ResourceCached::loadChangesCache( QMap<Incidence *, bool> &map, const QString &type )
{
  CalendarLocal calendar ( QLatin1String( "UTC" ) );

  if ( KStandardDirs::exists( changesCacheFile( type ) ) ) {
    calendar.load( changesCacheFile( type ) );
  } else {
    return;
  }

  const Incidence::List list = calendar.incidences();
  Incidence::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    map.insert( (*it)->clone(), true );
  }

  calendar.close();
}

void ResourceCached::loadChangesCache()
{
  loadChangesCache( d->mAddedIncidences, "added" );
  loadChangesCache( d->mDeletedIncidences, "deleted" );
  loadChangesCache( d->mChangedIncidences, "changed" );
}

void ResourceCached::calendarIncidenceAdded( Incidence *i )
{
#if 1
  kDebug(5800) << "ResourceCached::calendarIncidenceAdded():"
            << i->uid();
#endif

  QMap<Incidence *,bool>::ConstIterator it;
  it = d->mAddedIncidences.find( i );
  if ( it == d->mAddedIncidences.end() ) {
    d->mAddedIncidences.insert( i, true );
  }

  checkForAutomaticSave();
}

void ResourceCached::calendarIncidenceChanged( Incidence *i )
{
#if 1
  kDebug(5800) << "ResourceCached::calendarIncidenceChanged():"
            << i->uid();
#endif

  QMap<Incidence *,bool>::ConstIterator it;
  it = d->mChangedIncidences.find( i );
  // FIXME: If you modify an added incidence, there's no need to add it to d->mChangedIncidences!
  if ( it == d->mChangedIncidences.end() ) {
    d->mChangedIncidences.insert( i, true );
  }

  checkForAutomaticSave();
}

void ResourceCached::calendarIncidenceDeleted( Incidence *i )
{
#if 1
  kDebug(5800) << "ResourceCached::calendarIncidenceDeleted():"
            << i->uid();
#endif

  QMap<Incidence *,bool>::ConstIterator it;
  it = d->mDeletedIncidences.find( i );
  if ( it == d->mDeletedIncidences.end() ) {
    d->mDeletedIncidences.insert( i, true );
  }

  checkForAutomaticSave();
}

Incidence::List ResourceCached::addedIncidences() const
{
  Incidence::List added;
  QMap<Incidence *,bool>::ConstIterator it;
  for ( it = d->mAddedIncidences.begin(); it != d->mAddedIncidences.end(); ++it ) {
    added.append( it.key() );
  }
  return added;
}

Incidence::List ResourceCached::changedIncidences() const
{
  Incidence::List changed;
  QMap<Incidence *,bool>::ConstIterator it;
  for ( it = d->mChangedIncidences.begin(); it != d->mChangedIncidences.end(); ++it ) {
    changed.append( it.key() );
  }
  return changed;
}

Incidence::List ResourceCached::deletedIncidences() const
{
  Incidence::List deleted;
  QMap<Incidence *,bool>::ConstIterator it;
  for ( it = d->mDeletedIncidences.begin(); it != d->mDeletedIncidences.end(); ++it ) {
    deleted.append( it.key() );
  }
  return deleted;
}

Incidence::List ResourceCached::allChanges() const
{
  Incidence::List changes;
  QMap<Incidence *,bool>::ConstIterator it;
  for ( it = d->mAddedIncidences.begin(); it != d->mAddedIncidences.end(); ++it ) {
    changes.append( it.key() );
  }
  for ( it = d->mChangedIncidences.begin(); it != d->mChangedIncidences.end(); ++it ) {
    changes.append( it.key() );
  }
  for ( it = d->mDeletedIncidences.begin(); it != d->mDeletedIncidences.end(); ++it ) {
    changes.append( it.key() );
  }
  return changes;
}

bool ResourceCached::hasChanges() const
{
  return !( d->mAddedIncidences.isEmpty() && d->mChangedIncidences.isEmpty() &&
            d->mDeletedIncidences.isEmpty() );
}

void ResourceCached::clearChange( Incidence *incidence )
{
  clearChange( incidence->uid() );
}

void ResourceCached::clearChange( const QString &uid )
{
  QMap<Incidence *, bool>::Iterator it;

  for ( it = d->mAddedIncidences.begin(); it != d->mAddedIncidences.end(); ++it ) {
    if ( it.key()->uid() == uid ) {
      d->mAddedIncidences.erase( it );
      break;
    }
  }

  for ( it = d->mChangedIncidences.begin(); it != d->mChangedIncidences.end(); ++it ) {
    if ( it.key()->uid() == uid ) {
      d->mChangedIncidences.erase( it );
      break;
    }
  }

  for ( it = d->mDeletedIncidences.begin(); it != d->mDeletedIncidences.end(); ++it ) {
    if ( it.key()->uid() == uid ) {
      d->mDeletedIncidences.erase( it );
      break;
    }
  }
}

void ResourceCached::enableChangeNotification()
{
  d->mCalendar.registerObserver( this );
}

void ResourceCached::disableChangeNotification()
{
  d->mCalendar.unregisterObserver( this );
}

void ResourceCached::slotReload()
{
  if ( !isActive() ) {
    return;
  }

  kDebug(5800) << "ResourceCached::slotReload()";

  load( SyncCache );
}

void ResourceCached::slotSave()
{
  if ( !isActive() ) {
    return;
  }

  kDebug(5800) << "ResourceCached::slotSave()";

  save( SyncCache );
}

void ResourceCached::checkForAutomaticSave()
{
  if ( d->mSavePolicy == SaveAlways )  {
    kDebug(5800) << "ResourceCached::checkForAutomaticSave(): save now";
    d->mSavePending = true;
    d->mSaveTimer.setSingleShot( true );
    d->mSaveTimer.start( 1 * 1000 ); // 1 second
  } else if ( d->mSavePolicy == SaveDelayed ) {
    kDebug(5800) << "ResourceCached::checkForAutomaticSave(): save delayed";
    d->mSavePending = true;
    d->mSaveTimer.setSingleShot( true );
    d->mSaveTimer.start( 15 * 1000 ); // 15 seconds
  }
}

bool ResourceCached::checkForReload()
{
  if ( d->mReloadPolicy == ReloadNever ) {
    return false;
  }
  if ( d->mReloadPolicy == ReloadOnStartup ) {
    return !d->mReloaded;
  }
  return true;
}

bool ResourceCached::checkForSave()
{
  if ( d->mSavePolicy == SaveNever ) {
    return false;
  }
  return true;
}

void ResourceCached::addInfoText( QString &txt ) const
{
  if ( d->mLastLoad.isValid() ) {
    txt += "<br>";
    txt += i18n( "Last loaded: %1",
                 KGlobal::locale()->formatDateTime( d->mLastLoad.toUtc().dateTime() ) );
  }
  if ( d->mLastSave.isValid() ) {
    txt += "<br>";
    txt += i18n( "Last saved: %1",
                 KGlobal::locale()->formatDateTime( d->mLastSave.toUtc().dateTime() ) );
  }
}

void ResourceCached::doClose()
{
  if ( d->mSavePending ) {
    d->mSaveTimer.stop();
  }
  if ( d->mSavePending  ||  d->mSavePolicy == SaveOnExit  ||  d->mSavePolicy == SaveInterval ) {
    save( SyncCache );
  }
  d->mCalendar.close();
}

bool ResourceCached::doOpen()
{
  kDebug(5800) << "Opening resource" << resourceName();
  return true;
}

void KCal::ResourceCached::setOwner( const Person &owner )
{
  d->mCalendar.setOwner( owner );
}

Person KCal::ResourceCached::owner() const
{
  return d->mCalendar.owner();
}
