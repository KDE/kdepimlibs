/*
  This file is part of the kcal library.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CalendarResources class.

   @brief
   This class provides a Calendar which is composed of other Calendars
   known as "Resources".

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "calendarresources.moc"
#include "incidence.h"
#include "journal.h"
#include "resourcecalendar.h"

#include "kresources/manager.h"
#include "kresources/selectdialog.h"
#include "kabc/lock.h"

#include <kdebug.h>
#include <kdatetime.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include <QtCore/QString>
#include <QtCore/QList>

#include <stdlib.h>

using namespace KCal;

/**
  Private classes that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::CalendarResources::Private
{
  public:
    Private( const QString &family )
      : mManager( new CalendarResourceManager( family ) ),
        mStandardPolicy( new StandardDestinationPolicy( mManager ) ),
        mDestinationPolicy( mStandardPolicy ),
        mAskPolicy( new AskDestinationPolicy( mManager ) ),
        mPendingDeleteFromResourceMap( false )
    {}
    ~Private()
    {
      delete mManager;
      delete mStandardPolicy;
      delete mAskPolicy;
    }
    bool mOpen;  //flag that indicates if the resources are "open"

    KRES::Manager<ResourceCalendar>* mManager;
    QMap <Incidence*, ResourceCalendar*> mResourceMap;

    StandardDestinationPolicy *mStandardPolicy;
    DestinationPolicy *mDestinationPolicy;
    AskDestinationPolicy *mAskPolicy;

    QMap<ResourceCalendar *, Ticket *> mTickets;
    QMap<ResourceCalendar *, int> mChangeCounts;

    bool mPendingDeleteFromResourceMap;

    template< class IncidenceList >
    void appendIncidences( IncidenceList &result, const IncidenceList &extra,
                           ResourceCalendar * );
};

class KCal::CalendarResources::DestinationPolicy::Private
{
  public:
    Private( CalendarResourceManager *manager, QWidget *parent )
      : mManager( manager ),
        mParent( parent )
    {}
    CalendarResourceManager *mManager;
    QWidget *mParent;
};

class KCal::CalendarResources::StandardDestinationPolicy::Private
{
  public:
    Private()
    {}
};

class KCal::CalendarResources::AskDestinationPolicy::Private
{
  public:
    Private()
    {}
};

class KCal::CalendarResources::Ticket::Private
{
  public:
    Private( ResourceCalendar *resource )
      : mResource( resource )
    {}
    ResourceCalendar *mResource;
};
//@endcond

CalendarResources::DestinationPolicy::DestinationPolicy(
  CalendarResourceManager *manager, QWidget *parent )
  : d( new KCal::CalendarResources::DestinationPolicy::Private( manager, parent ) )
{
}

CalendarResources::DestinationPolicy::~DestinationPolicy()
{
  delete d;
}

QWidget *CalendarResources::DestinationPolicy::parent()
{
  return d->mParent;
}

void CalendarResources::DestinationPolicy::setParent( QWidget *parent )
{
  d->mParent = parent;
}

CalendarResourceManager *CalendarResources::DestinationPolicy::resourceManager()
{
  return d->mManager;
}

CalendarResources::StandardDestinationPolicy::StandardDestinationPolicy(
  CalendarResourceManager *manager, QWidget *parent )
  : DestinationPolicy( manager, parent ),
    d( new KCal::CalendarResources::StandardDestinationPolicy::Private )
{
}

CalendarResources::StandardDestinationPolicy::~StandardDestinationPolicy()
{
  delete d;
}

ResourceCalendar *CalendarResources::StandardDestinationPolicy::destination( Incidence *incidence )
{
  Q_UNUSED( incidence );
  return resourceManager()->standardResource();
}

CalendarResources::AskDestinationPolicy::AskDestinationPolicy(
  CalendarResourceManager *manager, QWidget *parent )
  : DestinationPolicy( manager, parent ),
    d( new KCal::CalendarResources::AskDestinationPolicy::Private )
{
}

CalendarResources::AskDestinationPolicy::~AskDestinationPolicy()
{
  delete d;
}

ResourceCalendar *CalendarResources::AskDestinationPolicy::destination( Incidence *incidence )
{
  Q_UNUSED( incidence );
  QList<KRES::Resource*> list;

  CalendarResourceManager::ActiveIterator it;
  for ( it = resourceManager()->activeBegin();
        it != resourceManager()->activeEnd(); ++it ) {
    if ( !(*it)->readOnly() ) {
      //Insert the first the Standard resource to get be the default selected.
      if ( resourceManager()->standardResource() == *it ) {
        list.insert( 0, *it );
      } else {
        list.append( *it );
      }
    }
  }

  KRES::Resource *r;
  r = KRES::SelectDialog::getResource( list, parent() );
  return static_cast<ResourceCalendar *>( r );
}

CalendarResources::CalendarResources( const KDateTime::Spec &timeSpec,
                                      const QString &family )
  : Calendar( timeSpec ),
    d( new KCal::CalendarResources::Private( family ) )
{
  d->mManager->addObserver( this );
}

CalendarResources::CalendarResources( const QString &timeZoneId,
                                      const QString &family )
  : Calendar( timeZoneId ),
    d( new KCal::CalendarResources::Private( family ) )
{
  d->mManager->addObserver( this );
}

CalendarResources::~CalendarResources()
{
  close();
  delete d;
}

void CalendarResources::readConfig( KConfig *config )
{
  d->mManager->readConfig( config );

  CalendarResourceManager::Iterator it;
  for ( it = d->mManager->begin(); it != d->mManager->end(); ++it ) {
    connectResource( *it );
  }
}

void CalendarResources::load()
{
  if ( !d->mManager->standardResource() ) {
    kDebug() << "Warning! No standard resource yet.";
  }

  // set the timezone for all resources. Otherwise we'll have those terrible tz
  // troubles ;-((
  CalendarResourceManager::Iterator i1;
  for ( i1 = d->mManager->begin(); i1 != d->mManager->end(); ++i1 ) {
    (*i1)->setTimeSpec( timeSpec() );
  }

  QList<ResourceCalendar *> failed;

  // Open all active resources
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    if ( !(*it)->load() ) {
      failed.append( *it );
    }
    Incidence::List incidences = (*it)->rawIncidences();
    Incidence::List::Iterator incit;
    for ( incit = incidences.begin(); incit != incidences.end(); ++incit ) {
      (*incit)->registerObserver( this );
      notifyIncidenceAdded( *incit );
    }
  }

  QList<ResourceCalendar *>::ConstIterator it2;
  for ( it2 = failed.begin(); it2 != failed.end(); ++it2 ) {
    (*it2)->setActive( false );
    emit signalResourceModified( *it2 );
  }

  d->mOpen = true;
  emit calendarLoaded();
}

bool CalendarResources::reload()
{
  save();
  close();
  load();
  return true;
}

CalendarResourceManager *CalendarResources::resourceManager() const
{
  return d->mManager;
}

void CalendarResources::setStandardDestinationPolicy()
{
  d->mDestinationPolicy = d->mStandardPolicy;
}

void CalendarResources::setAskDestinationPolicy()
{
  d->mDestinationPolicy = d->mAskPolicy;
}

QWidget *CalendarResources::dialogParentWidget()
{
  return d->mDestinationPolicy->parent();
}

void CalendarResources::setDialogParentWidget( QWidget *parent )
{
  d->mDestinationPolicy->setParent( parent );
}

void CalendarResources::close()
{
  if ( d->mOpen ) {
    CalendarResourceManager::ActiveIterator it;
    for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
      (*it)->close();
    }

    setModified( false );
    d->mOpen = false;
  }
}

bool CalendarResources::save()
{
  bool status = true;
  if ( d->mOpen && isModified() ) {
    status = false;
    CalendarResourceManager::ActiveIterator it;
    for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
      status = (*it)->save() || status;
    }
    setModified( false );
  }

  return status;
}

bool CalendarResources::isSaving()
{
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    if ( (*it)->isSaving() ) {
      return true;
    }
  }
  return false;
}

bool CalendarResources::addIncidence( Incidence *incidence,
                                      ResourceCalendar *resource )
{
  // FIXME: Use proper locking via begin/endChange!
  bool validRes = false;
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    if ( (*it) == resource ) {
      validRes = true;
    }
  }

  ResourceCalendar *oldResource = 0;
  if ( d->mResourceMap.contains( incidence ) ) {
    oldResource = d->mResourceMap[incidence];
  }
  d->mResourceMap[incidence] = resource;
  if ( validRes && beginChange( incidence ) &&
       resource->addIncidence( incidence ) ) {
//    d->mResourceMap[incidence] = resource;
    incidence->registerObserver( this );
    notifyIncidenceAdded( incidence );
    setModified( true );
    endChange( incidence );
    return true;
  } else {
    if ( oldResource ) {
      d->mResourceMap[incidence] = oldResource;
    } else {
      d->mResourceMap.remove( incidence );
    }
  }

  return false;
}

bool CalendarResources::addIncidence( Incidence *incidence )
{
  ResourceCalendar *resource = d->mDestinationPolicy->destination( incidence );

  if ( resource ) {
    d->mResourceMap[ incidence ] = resource;

    if ( beginChange( incidence ) && resource->addIncidence( incidence ) ) {
      incidence->registerObserver( this );
      notifyIncidenceAdded( incidence );

      d->mResourceMap[ incidence ] = resource;
      setModified( true );
      endChange( incidence );
      return true;
    } else {
      d->mResourceMap.remove( incidence );
    }
  } else {
    kDebug() << "no resource";
  }

  return false;
}

bool CalendarResources::addEvent( Event *event )
{
  return addIncidence( event );
}

bool CalendarResources::addEvent( Event *Event, ResourceCalendar *resource )
{
  return addIncidence( Event, resource );
}

bool CalendarResources::deleteEvent( Event *event )
{
  bool status;
  if ( d->mResourceMap.find( event ) != d->mResourceMap.end() ) {
    status = d->mResourceMap[event]->deleteEvent( event );
    if ( status ) {
      d->mPendingDeleteFromResourceMap = true;
    }
  } else {
    status = false;
    CalendarResourceManager::ActiveIterator it;
    for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
      status = (*it)->deleteEvent( event ) || status;
    }
  }
  if ( status ) {
    notifyIncidenceDeleted( event );
  }

  setModified( status );
  return status;
}

void CalendarResources::deleteAllEvents()
{
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    (*it)->deleteAllEvents();
  }
}

Event *CalendarResources::event( const QString &uid )
{
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    Event *event = (*it)->event( uid );
    if ( event ) {
      d->mResourceMap[event] = *it;
      return event;
    }
  }

  // Not found
  return 0;
}

bool CalendarResources::addTodo( Todo *todo )
{
  return addIncidence( todo );
}

bool CalendarResources::addTodo( Todo *todo, ResourceCalendar *resource )
{
  return addIncidence( todo, resource );
}

bool CalendarResources::deleteTodo( Todo *todo )
{
  bool status;
  if ( d->mResourceMap.find( todo ) != d->mResourceMap.end() ) {
    status = d->mResourceMap[todo]->deleteTodo( todo );
    if ( status ) {
      d->mPendingDeleteFromResourceMap = true;
    }
  } else {
    CalendarResourceManager::ActiveIterator it;
    status = false;
    for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
      status = (*it)->deleteTodo( todo ) || status;
    }
  }

  setModified( status );
  return status;
}

void CalendarResources::deleteAllTodos()
{
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    (*it)->deleteAllTodos();
  }
}

Todo::List CalendarResources::rawTodos( TodoSortField sortField,
                                        SortDirection sortDirection )
{
  Todo::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    d->appendIncidences<Todo::List>( result,
                                     (*it)->rawTodos( TodoSortUnsorted ), *it );
  }
  return sortTodos( &result, sortField, sortDirection );
}

Todo *CalendarResources::todo( const QString &uid )
{
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    Todo *todo = (*it)->todo( uid );
    if ( todo ) {
      d->mResourceMap[todo] = *it;
      return todo;
    }
  }

  // Not found
  return 0;
}

Todo::List CalendarResources::rawTodosForDate( const QDate &date )
{
  Todo::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    d->appendIncidences<Todo::List>( result,
                                     (*it)->rawTodosForDate( date ), *it );
  }
  return result;
}

Alarm::List CalendarResources::alarmsTo( const KDateTime &to )
{
  Alarm::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    result += (*it)->alarmsTo( to );
  }
  return result;
}

Alarm::List CalendarResources::alarms( const KDateTime &from,
                                       const KDateTime &to )
{
  Alarm::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    result += (*it)->alarms( from, to );
  }
  return result;
}

/****************************** PROTECTED METHODS ****************************/

Event::List CalendarResources::rawEventsForDate( const QDate &date,
                                                 const KDateTime::Spec &timespec,
                                                 EventSortField sortField,
                                                 SortDirection sortDirection )
{
  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    d->appendIncidences<Event::List>( result,
                                      (*it)->rawEventsForDate( date, timespec ), *it );
  }
  return sortEvents( &result, sortField, sortDirection );
}

Event::List CalendarResources::rawEvents( const QDate &start, const QDate &end,
                                          const KDateTime::Spec &timespec, bool inclusive )
{
  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    d->appendIncidences<Event::List>( result,
                                      (*it)->rawEvents( start, end, timespec, inclusive ), *it );
  }
  return result;
}

Event::List CalendarResources::rawEventsForDate( const KDateTime &kdt )
{
  // @TODO: Remove the code duplication by the resourcemap iteration block.
  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    d->appendIncidences<Event::List>( result,
                                      (*it)->rawEventsForDate( kdt ), *it );
  }
  return result;
}

Event::List CalendarResources::rawEvents( EventSortField sortField,
                                          SortDirection sortDirection )
{
  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    d->appendIncidences<Event::List>( result,
                                      (*it)->rawEvents( EventSortUnsorted ), *it );
  }
  return sortEvents( &result, sortField, sortDirection );
}

bool CalendarResources::addJournal( Journal *journal )
{
  return addIncidence( journal );
}

bool CalendarResources::deleteJournal( Journal *journal )
{
  bool status;
  if ( d->mResourceMap.find( journal ) != d->mResourceMap.end() ) {
    status = d->mResourceMap[journal]->deleteJournal( journal );
    if ( status ) {
      d->mPendingDeleteFromResourceMap = true;
    }
  } else {
    CalendarResourceManager::ActiveIterator it;
    status = false;
    for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
      status = (*it)->deleteJournal( journal ) || status;
    }
  }

  setModified( status );
  return status;
}

void CalendarResources::deleteAllJournals()
{
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    (*it)->deleteAllJournals();
  }
}

bool CalendarResources::addJournal( Journal *journal,
                                    ResourceCalendar *resource )
{
  return addIncidence( journal, resource );
}

Journal *CalendarResources::journal( const QString &uid )
{
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    Journal *journal = (*it)->journal( uid );
    if ( journal ) {
      d->mResourceMap[journal] = *it;
      return journal;
    }
  }

  // Not found
  return 0;
}

Journal::List CalendarResources::rawJournals( JournalSortField sortField,
                                              SortDirection sortDirection )
{
  Journal::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    d->appendIncidences<Journal::List>( result,
                                        (*it)->rawJournals( JournalSortUnsorted ), *it );
  }
  return sortJournals( &result, sortField, sortDirection );
}

Journal::List CalendarResources::rawJournalsForDate( const QDate &date )
{

  Journal::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = d->mManager->activeBegin(); it != d->mManager->activeEnd(); ++it ) {
    d->appendIncidences<Journal::List>( result,
                                        (*it)->rawJournalsForDate( date ), *it );
  }
  return result;
}

//@cond PRIVATE
template< class IncidenceList >
void CalendarResources::Private::appendIncidences( IncidenceList &result,
                                                   const IncidenceList &extra,
                                                   ResourceCalendar *resource )
{
  result += extra;
  for ( typename IncidenceList::ConstIterator it = extra.begin();
        it != extra.end();
        ++it ) {
    mResourceMap[ *it ] = resource;
  }
}
//@endcond

void CalendarResources::connectResource( ResourceCalendar *resource )
{
  connect( resource, SIGNAL( resourceChanged( ResourceCalendar * ) ),
           SIGNAL( calendarChanged() ) );
  connect( resource, SIGNAL( resourceSaved( ResourceCalendar * ) ),
           SIGNAL( calendarSaved() ) );

  connect( resource, SIGNAL( resourceLoadError( ResourceCalendar *,
                                                const QString & ) ),
           SLOT( slotLoadError( ResourceCalendar *, const QString & ) ) );
  connect( resource, SIGNAL( resourceSaveError( ResourceCalendar *,
                                                const QString & ) ),
           SLOT( slotSaveError( ResourceCalendar *, const QString & ) ) );
}

ResourceCalendar *CalendarResources::resource( Incidence *incidence )
{
  if ( d->mResourceMap.find( incidence ) != d->mResourceMap.end() ) {
    return d->mResourceMap[ incidence ];
  }
  return 0;
}

void CalendarResources::resourceAdded( ResourceCalendar *resource )
{
  if ( !resource->isActive() ) {
    return;
  }

  if ( resource->open() ) {
    resource->load();
  }

  connectResource( resource );

  emit signalResourceAdded( resource );
}

void CalendarResources::resourceModified( ResourceCalendar *resource )
{
  emit signalResourceModified( resource );
}

void CalendarResources::resourceDeleted( ResourceCalendar *resource )
{
  emit signalResourceDeleted( resource );
}

void CalendarResources::doSetTimeSpec( const KDateTime::Spec &timeSpec )
{
  // set the timezone for all resources. Otherwise we'll have those terrible
  // tz troubles ;-((
  CalendarResourceManager::Iterator i1;
  for ( i1 = d->mManager->begin(); i1 != d->mManager->end(); ++i1 ) {
    (*i1)->setTimeSpec( timeSpec );
  }
}

CalendarResources::Ticket::Ticket( ResourceCalendar *resource )
  : d( new KCal::CalendarResources::Ticket::Private( resource ) )
{
}

CalendarResources::Ticket::~Ticket()
{
  delete d;
}

CalendarResources::Ticket *CalendarResources::requestSaveTicket( ResourceCalendar *resource )
{
  KABC::Lock *lock = resource->lock();
  if ( !lock ) {
    return 0;
  }
  if ( lock->lock() ) {
    return new Ticket( resource );
  } else {
    return 0;
  }
}

ResourceCalendar *CalendarResources::Ticket::resource() const
{
  return d->mResource;
}

bool CalendarResources::save( Ticket *ticket, Incidence *incidence )
{
  if ( !ticket || !ticket->resource() ) {
    return false;
  }

  // @TODO: Check if the resource was changed at all. If not, don't save.
  if ( ticket->resource()->save( incidence ) ) {
    releaseSaveTicket( ticket );
    return true;
  }

  return false;
}

void CalendarResources::releaseSaveTicket( Ticket *ticket )
{
  ticket->resource()->lock()->unlock();
  delete ticket;
}

bool CalendarResources::beginChange( Incidence *incidence )
{
  ResourceCalendar *r = resource( incidence );
  if ( !r ) {
    r = d->mDestinationPolicy->destination( incidence );
    if ( !r ) {
      kError() << "Unable to get destination resource.";
      return false;
    }
    d->mResourceMap[ incidence ] = r;
  }
  d->mPendingDeleteFromResourceMap = false;

  int count = incrementChangeCount( r );
  if ( count == 1 ) {
    Ticket *ticket = requestSaveTicket( r );
    if ( !ticket ) {
      kDebug() << "unable to get ticket.";
      decrementChangeCount( r );
      return false;
    } else {
      d->mTickets[ r ] = ticket;
    }
  }

  return true;
}

bool CalendarResources::endChange( Incidence *incidence )
{
  ResourceCalendar *r = resource( incidence );
  if ( !r ) {
    return false;
  }

  int count = decrementChangeCount( r );

  if ( d->mPendingDeleteFromResourceMap ) {
    d->mResourceMap.remove( incidence );
    d->mPendingDeleteFromResourceMap = false;
  }

  if ( count == 0 ) {
    bool ok = save( d->mTickets[ r ], incidence );
    if ( ok ) {
      d->mTickets.remove( r );
    } else {
      return false;
    }
  }

  return true;
}

int CalendarResources::incrementChangeCount( ResourceCalendar *r )
{
  if ( !d->mChangeCounts.contains( r ) ) {
    d->mChangeCounts.insert( r, 0 );
  }

  int count = d->mChangeCounts[ r ];
  ++count;
  d->mChangeCounts[ r ] = count;

  return count;
}

int CalendarResources::decrementChangeCount( ResourceCalendar *r )
{
  if ( !d->mChangeCounts.contains( r ) ) {
    kError() << "No change count for resource.";
    return 0;
  }

  int count = d->mChangeCounts[ r ];
  --count;
  if ( count < 0 ) {
    kError() << "Can't decrement change count. It already is 0.";
    count = 0;
  }
  d->mChangeCounts[ r ] = count;

  return count;
}

void CalendarResources::slotLoadError( ResourceCalendar *r, const QString &err )
{
  Q_UNUSED( r );
  emit signalErrorMessage( err );
}

void CalendarResources::slotSaveError( ResourceCalendar *r, const QString &err )
{
  Q_UNUSED( r );
  emit signalErrorMessage( err );
}
