/*
  This file is part of the kcal library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>
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
  defines the CalendarLocal class.

  @brief
  This class provides a calendar stored as a local file.

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
 */

#include "calendarlocal.h"

#include "incidence.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "filestorage.h"
#include <QtCore/QDate>
#include <QtCore/QHash>
#include <QtCore/QMultiHash>
#include <QtCore/QString>

#include <kdebug.h>
#include <kdatetime.h>
#include <klocale.h>
#include <kmessagebox.h>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::CalendarLocal::Private
{
  public:
    Private() : mFormat(0) {}
    QString mFileName;                     // filename where calendar is stored
    CalFormat *mFormat;                    // calendar format

    QHash<QString, Event *>mEvents;        // hash on uids of all Events
    QMultiHash<QString, Event *>mEventsForDate;// on start dates of non-recurring, single-day Events
    QHash<QString, Todo *>mTodos;          // hash on uids of all Todos
    QMultiHash<QString, Todo*>mTodosForDate;// on due dates for all Todos
    QHash<QString, Journal *>mJournals;    // hash on uids of all Journals
    QMultiHash<QString, Journal *>mJournalsForDate; // on dates of all Journals

    void insertEvent( Event *event );
    void insertTodo( Todo *todo );
    void insertJournal( Journal *journal );
};

// helper
namespace {
template <typename T>
void removeIncidenceFromMultiHashByUID( QMultiHash< QString, T >& container,
                                        const QString &key,
                                        const QString &uid )
{
  const QList<T> values = container.values( key );
  QListIterator<T> it(values);
  while ( it.hasNext() ) {
    T const inc = it.next();
    if ( inc->uid() == uid ) {
      container.remove( key, inc );
    }
  }
}
}
//@endcond

CalendarLocal::CalendarLocal( const KDateTime::Spec &timeSpec )
  : Calendar( timeSpec ),
    d( new KCal::CalendarLocal::Private )
{
}

CalendarLocal::CalendarLocal( const QString &timeZoneId )
  : Calendar( timeZoneId ),
    d( new KCal::CalendarLocal::Private )
{
}

CalendarLocal::~CalendarLocal()
{
  close();
  delete d;
}

bool CalendarLocal::load( const QString &fileName, CalFormat *format )
{
  d->mFileName = fileName;
  FileStorage storage( this, fileName, format );
  return storage.load();
}

bool CalendarLocal::reload()
{
  const QString filename = d->mFileName;
  save();
  close();
  d->mFileName = filename;
  FileStorage storage( this, d->mFileName );
  return storage.load();
}

bool CalendarLocal::save()
{
  if ( d->mFileName.isEmpty() ) {
    return false;
  }

  if ( isModified() ) {
    FileStorage storage( this, d->mFileName, d->mFormat );
    return storage.save();
  } else {
    return true;
  }
}

bool CalendarLocal::save( const QString &fileName, CalFormat *format )
{
  // Save only if the calendar is either modified, or saved to a
  // different file than it was loaded from
  if ( d->mFileName != fileName || isModified() ) {
    FileStorage storage( this, fileName, format );
    return storage.save();
  } else {
    return true;
  }
}

void CalendarLocal::close()
{
  setObserversEnabled( false );
  d->mFileName.clear();

  deleteAllEvents();
  deleteAllTodos();
  deleteAllJournals();

  setModified( false );

  setObserversEnabled( true );
}

bool CalendarLocal::addEvent( Event *event )
{
  d->insertEvent( event );

  event->registerObserver( this );

  setModified( true );

  notifyIncidenceAdded( event );

  return true;
}

bool CalendarLocal::deleteEvent( Event *event )
{
  const QString uid = event->uid();
  if ( d->mEvents.remove( uid ) ) {
    setModified( true );
    notifyIncidenceDeleted( event );
    if ( !event->recurs() ) {
      removeIncidenceFromMultiHashByUID<Event *>(
        d->mEventsForDate, event->dtStart().date().toString(), event->uid() );
    }
    return true;
  } else {
    kWarning() << "Event not found.";
    return false;
  }
}

void CalendarLocal::deleteAllEvents()
{
  QHashIterator<QString, Event *>i( d->mEvents );
  while ( i.hasNext() ) {
    i.next();
    notifyIncidenceDeleted( i.value() );
    // suppress update notifications for the relation removal triggered
    // by the following deletions
    i.value()->startUpdates();
  }
  qDeleteAll( d->mEvents );
  d->mEvents.clear();
  d->mEventsForDate.clear();
}

Event *CalendarLocal::event( const QString &uid )
{
  return d->mEvents.value( uid );
}

bool CalendarLocal::addTodo( Todo *todo )
{
  d->insertTodo( todo );

  todo->registerObserver( this );

  // Set up sub-to-do relations
  setupRelations( todo );

  setModified( true );

  notifyIncidenceAdded( todo );

  return true;
}

//@cond PRIVATE
void CalendarLocal::Private::insertTodo( Todo *todo )
{
  QString uid = todo->uid();
  if ( !mTodos.contains( uid ) ) {
    mTodos.insert( uid, todo );
    if ( todo->hasDueDate() ) {
      mTodosForDate.insert( todo->dtDue().date().toString(), todo );
    }

  } else {
#ifndef NDEBUG
    // if we already have an to-do with this UID, it must be the same to-do,
    // otherwise something's really broken
    Q_ASSERT( mTodos.value( uid ) == todo );
#endif
  }
}
//@endcond

bool CalendarLocal::deleteTodo( Todo *todo )
{
  // Handle orphaned children
  removeRelations( todo );

  if ( d->mTodos.remove( todo->uid() ) ) {
    setModified( true );
    notifyIncidenceDeleted( todo );
    if ( todo->hasDueDate() ) {
      removeIncidenceFromMultiHashByUID<Todo *>(
        d->mTodosForDate, todo->dtDue().date().toString(), todo->uid() );
    }
    return true;
  } else {
    kWarning() << "Todo not found.";
    return false;
  }
}

void CalendarLocal::deleteAllTodos()
{
  QHashIterator<QString, Todo *>i( d->mTodos );
  while ( i.hasNext() ) {
    i.next();
    notifyIncidenceDeleted( i.value() );
    // suppress update notifications for the relation removal triggered
    // by the following deletions
    i.value()->startUpdates();
  }
  qDeleteAll( d->mTodos );
  d->mTodos.clear();
  d->mTodosForDate.clear();
}

Todo *CalendarLocal::todo( const QString &uid )
{
  return d->mTodos.value( uid );
}

Todo::List CalendarLocal::rawTodos( TodoSortField sortField,
                                    SortDirection sortDirection )
{
  Todo::List todoList;
  QHashIterator<QString, Todo *>i( d->mTodos );
  while ( i.hasNext() ) {
    i.next();
    todoList.append( i.value() );
  }
  return sortTodos( &todoList, sortField, sortDirection );
}

Todo::List CalendarLocal::rawTodosForDate( const QDate &date )
{
  Todo::List todoList;
  Todo *t;

  QString dateStr = date.toString();
  QMultiHash<QString, Todo *>::const_iterator it = d->mTodosForDate.constFind( dateStr );
  while ( it != d->mTodosForDate.constEnd() && it.key() == dateStr ) {
    t = it.value();
    todoList.append( t );
    ++it;
  }
  return todoList;
}

Alarm::List CalendarLocal::alarmsTo( const KDateTime &to )
{
  return alarms( KDateTime( QDate( 1900, 1, 1 ) ), to );
}

Alarm::List CalendarLocal::alarms( const KDateTime &from, const KDateTime &to )
{
  Alarm::List alarmList;
  QHashIterator<QString, Event *>ie( d->mEvents );
  Event *e;
  while ( ie.hasNext() ) {
    ie.next();
    e = ie.value();
    if ( e->recurs() ) {
      appendRecurringAlarms( alarmList, e, from, to );
    } else {
      appendAlarms( alarmList, e, from, to );
    }
  }

  QHashIterator<QString, Todo *>it( d->mTodos );
  Todo *t;
  while ( it.hasNext() ) {
    it.next();
    t = it.value();
    if ( !t->isCompleted() ) {
      if ( t->recurs() ) {
        appendRecurringAlarms( alarmList, t, from, to );
      } else {
        appendAlarms( alarmList, t, from, to );
      }
    }
  }

  return alarmList;
}

//@cond PRIVATE
void CalendarLocal::Private::insertEvent( Event *event )
{
  QString uid = event->uid();
  if ( !mEvents.contains( uid ) ) {
    mEvents.insert( uid, event );
    if ( !event->recurs() && !event->isMultiDay() ) {
      mEventsForDate.insert( event->dtStart().date().toString(), event );
    }
  } else {
#ifdef NDEBUG
    // if we already have an event with this UID, it must be the same event,
    // otherwise something's really broken
    Q_ASSERT( mEvents.value( uid ) == event );
#endif
  }
}
//@endcond

void CalendarLocal::incidenceUpdated( IncidenceBase *incidence )
{
  KDateTime nowUTC = KDateTime::currentUtcDateTime();
  incidence->setLastModified( nowUTC );
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

  if ( incidence->type() == "Event" ) {
    Event *event = static_cast<Event*>( incidence );
    removeIncidenceFromMultiHashByUID<Event *>(
      d->mEventsForDate, event->dtStart().date().toString(), event->uid() );
    if ( !event->recurs() && !event->isMultiDay() ) {
      d->mEventsForDate.insert( event->dtStart().date().toString(), event );
    }
  } else if ( incidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo*>( incidence );
    removeIncidenceFromMultiHashByUID<Todo *>(
      d->mTodosForDate, todo->dtDue().date().toString(), todo->uid() );
    if ( todo->hasDueDate() ) {
      d->mTodosForDate.insert( todo->dtDue().date().toString(), todo );
    }
  } else if ( incidence->type() == "Journal" ) {
    Journal *journal = static_cast<Journal*>( incidence );
    removeIncidenceFromMultiHashByUID<Journal *>(
      d->mJournalsForDate, journal->dtStart().date().toString(), journal->uid() );
    d->mJournalsForDate.insert( journal->dtStart().date().toString(), journal );
  } else {
    Q_ASSERT( false );
  }

  // The static_cast is ok as the CalendarLocal only observes Incidence objects
  notifyIncidenceChanged( static_cast<Incidence *>( incidence ) );

  setModified( true );
}

Event::List CalendarLocal::rawEventsForDate( const QDate &date,
                                             const KDateTime::Spec &timespec,
                                             EventSortField sortField,
                                             SortDirection sortDirection )
{
  Event::List eventList;
  Event *ev;

  // Find the hash for the specified date
  QString dateStr = date.toString();
  QMultiHash<QString, Event *>::const_iterator it = d->mEventsForDate.constFind( dateStr );
  // Iterate over all non-recurring, single-day events that start on this date
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime kdt( date, ts );
  while ( it != d->mEventsForDate.constEnd() && it.key() == dateStr ) {
    ev = it.value();
    KDateTime end( ev->dtEnd().toTimeSpec( ev->dtStart() ) );
    if ( ev->allDay() ) {
      end.setDateOnly( true );
    } else {
      end = end.addSecs( -1 );
    }
    if ( end >= kdt ) {
      eventList.append( ev );
    }
    ++it;
  }

  // Iterate over all events. Look for recurring events that occur on this date
  QHashIterator<QString, Event *>i( d->mEvents );
  while ( i.hasNext() ) {
    i.next();
    ev = i.value();
    if ( ev->recurs() ) {
      if ( ev->isMultiDay() ) {
        int extraDays = ev->dtStart().date().daysTo( ev->dtEnd().date() );
        for ( int i = 0; i <= extraDays; ++i ) {
          if ( ev->recursOn( date.addDays( -i ), ts ) ) {
            eventList.append( ev );
            break;
          }
        }
      } else {
        if ( ev->recursOn( date, ts ) ) {
          eventList.append( ev );
        }
      }
    } else {
      if ( ev->isMultiDay() ) {
        if ( ev->dtStart().date() <= date && ev->dtEnd().date() >= date ) {
          eventList.append( ev );
        }
      }
    }
  }

  return sortEventsForDate( &eventList, date, timespec, sortField, sortDirection );
}

Event::List CalendarLocal::rawEvents( const QDate &start, const QDate &end,
                                      const KDateTime::Spec &timespec, bool inclusive )
{
  Event::List eventList;
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime st( start, ts );
  KDateTime nd( end, ts );
  KDateTime yesterStart = st.addDays( -1 );

  // Get non-recurring events
  QHashIterator<QString, Event *>i( d->mEvents );
  Event *event;
  while ( i.hasNext() ) {
    i.next();
    event = i.value();
    KDateTime rStart = event->dtStart();
    if ( nd < rStart ) {
      continue;
    }
    if ( inclusive && rStart < st ) {
      continue;
    }

    if ( !event->recurs() ) { // non-recurring events
      KDateTime rEnd = event->dtEnd();
      if ( rEnd < st ) {
        continue;
      }
      if ( inclusive && nd < rEnd ) {
        continue;
      }
    } else { // recurring events
      switch( event->recurrence()->duration() ) {
      case -1: // infinite
        if ( inclusive ) {
          continue;
        }
        break;
      case 0: // end date given
      default: // count given
        KDateTime rEnd( event->recurrence()->endDate(), ts );
        if ( !rEnd.isValid() ) {
          continue;
        }
        if ( rEnd < st ) {
          continue;
        }
        if ( inclusive && nd < rEnd ) {
          continue;
        }
        break;
      } // switch(duration)
    } //if(recurs)

    eventList.append( event );
  }

  return eventList;
}

Event::List CalendarLocal::rawEventsForDate( const KDateTime &kdt )
{
  return rawEventsForDate( kdt.date(), kdt.timeSpec() );
}

Event::List CalendarLocal::rawEvents( EventSortField sortField,
                                      SortDirection sortDirection )
{
  Event::List eventList;
  QHashIterator<QString, Event *>i( d->mEvents );
  while ( i.hasNext() ) {
    i.next();
    eventList.append( i.value() );
  }
  return sortEvents( &eventList, sortField, sortDirection );
}

bool CalendarLocal::addJournal( Journal *journal )
{
  d->insertJournal( journal );

  journal->registerObserver( this );

  setModified( true );

  notifyIncidenceAdded( journal );

  return true;
}

//@cond PRIVATE
void CalendarLocal::Private::insertJournal( Journal *journal )
{
  QString uid = journal->uid();
  if ( !mJournals.contains( uid ) ) {
    mJournals.insert( uid, journal );
    mJournalsForDate.insert( journal->dtStart().date().toString(), journal );
  } else {
#ifndef NDEBUG
    // if we already have an journal with this UID, it must be the same journal,
    // otherwise something's really broken
    Q_ASSERT( mJournals.value( uid ) == journal );
#endif
  }
}
//@endcond

bool CalendarLocal::deleteJournal( Journal *journal )
{
  if ( d->mJournals.remove( journal->uid() ) ) {
    setModified( true );
    notifyIncidenceDeleted( journal );
    removeIncidenceFromMultiHashByUID<Journal *>(
      d->mJournalsForDate, journal->dtStart().date().toString(), journal->uid() );
    return true;
  } else {
    kWarning() << "Journal not found.";
    return false;
  }
}

void CalendarLocal::deleteAllJournals()
{
  QHashIterator<QString, Journal *>i( d->mJournals );
  while ( i.hasNext() ) {
    i.next();
    notifyIncidenceDeleted( i.value() );
    // suppress update notifications for the relation removal triggered
    // by the following deletions
    i.value()->startUpdates();
  }
  qDeleteAll( d->mJournals );
  d->mJournals.clear();
  d->mJournalsForDate.clear();
}

Journal *CalendarLocal::journal( const QString &uid )
{
  return d->mJournals.value( uid );
}

Journal::List CalendarLocal::rawJournals( JournalSortField sortField,
                                          SortDirection sortDirection )
{
  Journal::List journalList;
  QHashIterator<QString, Journal *>i( d->mJournals );
  while ( i.hasNext() ) {
    i.next();
    journalList.append( i.value() );
  }
  return sortJournals( &journalList, sortField, sortDirection );
}

Journal::List CalendarLocal::rawJournalsForDate( const QDate &date )
{
  Journal::List journalList;
  Journal *j;

  QString dateStr = date.toString();
  QMultiHash<QString, Journal *>::const_iterator it = d->mJournalsForDate.constFind( dateStr );

  while ( it != d->mJournalsForDate.constEnd() && it.key() == dateStr ) {
    j = it.value();
    journalList.append( j );
    ++it;
  }
  return journalList;
}
