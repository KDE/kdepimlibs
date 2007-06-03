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

  @author Preston Brown
  @author Cornelius Schumacher
 */

#include "calendarlocal.h"

#include "incidence.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "filestorage.h"
#include <QtCore/QHash>
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
    Private()
    {
      mDeletedIncidences.setAutoDelete( true );
    }
    QString mFileName;                 // filename where the calendar is stored
    CalFormat *mFormat;                // calendar format

    QHash<QString, Event *> mEvents;    // hash on uids of all calendar events
    QHash<QString, Todo *> mTodos;      // hash on uids of all calendar to-dos
    QHash<QString, Journal *> mJournals;// hash on uids of all calendar journals
    Incidence::List mDeletedIncidences; // list of all deleted incidences

    void insertEvent( Event *event );
    void insertTodo( Todo *todo );
    void insertJournal( Journal *journal );
};
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

  d->mDeletedIncidences.clear();
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
  if ( d->mEvents.remove( event->uid() ) ) {
    setModified( true );
    notifyIncidenceDeleted( event );
    d->mDeletedIncidences.append( event );
    return true;
  } else {
    kWarning() << "CalendarLocal::deleteEvent(): Event not found." << endl;
    return false;
  }
}

void CalendarLocal::deleteAllEvents()
{
  foreach ( Event *e, d->mEvents ) {
    notifyIncidenceDeleted( e );
  }
  qDeleteAll( d->mEvents );
  d->mEvents.clear();
}

Event *CalendarLocal::event( const QString &uid )
{
  return d->mEvents.value( uid );
}

bool CalendarLocal::addTodo( Todo *todo )
{
  d->insertTodo( todo );

  todo->registerObserver( this );

  // Set up subtask relations
  setupRelations( todo );

  setModified( true );

  notifyIncidenceAdded( todo );

  return true;
}

//@cond PRIVATE
void CalendarLocal::Private::insertTodo( Todo *todo )
{
  QString uid = todo->uid();
  if ( mTodos.value( uid ) == 0 ) {
    mTodos.insert( uid, todo );
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
    d->mDeletedIncidences.append( todo );
    return true;
  } else {
    kWarning() << "CalendarLocal::deleteTodo(): Todo not found." << endl;
    return false;
  }
}

void CalendarLocal::deleteAllTodos()
{
  foreach ( Todo *t, d->mTodos ) {
    notifyIncidenceDeleted( t );
  }
  qDeleteAll( d->mTodos );
  d->mTodos.clear();
}

Todo *CalendarLocal::todo( const QString &uid )
{
  return d->mTodos.value( uid );
}

Todo::List CalendarLocal::rawTodos( TodoSortField sortField,
                                    SortDirection sortDirection )
{
  Todo::List todoList;
  foreach ( Todo *t, d->mTodos ) {
    todoList.append( t );
  }
  return sortTodos( &todoList, sortField, sortDirection );
}

Todo::List CalendarLocal::rawTodosForDate( const QDate &date )
{
  Todo::List todoList;
  foreach ( Todo *t, d->mTodos ) {
    if ( t->hasDueDate() && t->dtDue().date() == date ) {
      todoList.append( t );
    }
  }
  return todoList;
}

Alarm::List CalendarLocal::alarmsTo( const KDateTime &to )
{
  return alarms( KDateTime( QDate( 1900, 1, 1 ) ), to );
}

Alarm::List CalendarLocal::alarms( const KDateTime &from, const KDateTime &to )
{
  Alarm::List alarms;
  foreach ( Event *e, d->mEvents ) {
    if ( e->doesRecur() ) {
      appendRecurringAlarms( alarms, e, from, to );
    } else {
      appendAlarms( alarms, e, from, to );
    }
  }

  foreach ( Todo *t, d->mTodos ) {
    if (! t->isCompleted() ) {
      appendAlarms( alarms, t, from, to );
    }
  }

  return alarms;
}

//@cond PRIVATE
void CalendarLocal::Private::insertEvent( Event *event )
{
  QString uid = event->uid();
  if ( mEvents.value( uid ) == 0 ) {
    mEvents.insert( uid, event );
  } else {
#ifdef NDEBUG
    // if we already have an event with this UID, it must be the same event,
    // otherwise something's really broken
    Q_ASSERT( mEvents.value( uid ) == event );
#endif
  }
}
//@endcond

Event::List CalendarLocal::rawEventsForDate( const QDate &qd,
                                             EventSortField sortField,
                                             SortDirection sortDirection )
{
//TODO: add KDateTime::Spec parameter?
  Event::List eventList;

  foreach ( Event *event, d->mEvents ) {

    if ( event->doesRecur() ) {
      if ( event->isMultiDay() ) {
        int extraDays = event->dtStart().date().daysTo( event->dtEnd().date() );
        int i;
        for ( i = 0; i <= extraDays; i++ ) {
          if ( event->recursOn( qd.addDays( -i ), timeSpec() ) ) {
            eventList.append( event );
            break;
          }
        }
      } else {
        if ( event->recursOn( qd, timeSpec() ) ) {
          eventList.append( event );
        }
      }
    } else {
      if ( event->dtStart().date() <= qd && event->dateEnd() >= qd ) {
        eventList.append( event );
      }
    }
  }

  return sortEvents( &eventList, sortField, sortDirection );
}

Event::List CalendarLocal::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
//TODO: add KDateTime::Spec parameter?
  Event::List eventList;

  // Get non-recurring events
  foreach ( Event *event, d->mEvents ) {
    if ( event->doesRecur() ) {
      QDate rStart = event->dtStart().date();
      bool found = false;
      if ( inclusive ) {
        if ( rStart >= start && rStart <= end ) {
          // Start date of event is in range. Now check for end date.
          // if duration is negative, event recurs forever, so do not include it.
          if ( event->recurrence()->duration() == 0 ) {  // End date set
            QDate rEnd = event->recurrence()->endDate();
            if ( rEnd >= start && rEnd <= end ) {  // End date within range
              found = true;
            }
          } else if ( event->recurrence()->duration() > 0 ) {  // Duration set
            // TODO: Calculate end date from duration. Should be done in Event
            // For now exclude all events with a duration.
          }
        }
      } else {
        if ( rStart <= end ) {  // Start date not after range
          if ( rStart >= start ) {  // Start date within range
            found = true;
          } else if ( event->recurrence()->duration() == -1 ) {// Recurs forever
            found = true;
          } else if ( event->recurrence()->duration() == 0 ) { // End date set
            QDate rEnd = event->recurrence()->endDate();
            if ( rEnd >= start && rEnd <= end ) {  // End date within range
              found = true;
            }
          } else {  // Duration set
            // TODO: Calculate end date from duration. Should be done in Event
            // For now include all events with a duration.
            found = true;
          }
        }
      }

      if ( found ) {
        eventList.append( event );
      }

    } else {
      QDate s = event->dtStart().date();
      QDate e = event->dtEnd().date();

      if ( inclusive ) {
        if ( s >= start && e <= end ) {
          eventList.append( event );
        }
      } else {
        if ( s <= end && e >= start ) {
          eventList.append( event );
        }
      }
    }
  }

  return eventList;
}

Event::List CalendarLocal::rawEventsForDate( const KDateTime &qdt )
{
  return rawEventsForDate( qdt.date() );
}

Event::List CalendarLocal::rawEvents( EventSortField sortField,
                                      SortDirection sortDirection )
{
  Event::List eventList;
  foreach ( Event *e, d->mEvents ) {
    eventList.append( e );
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
  if ( mJournals.value( uid ) == 0 ) {
    mJournals.insert( uid, journal );
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
    d->mDeletedIncidences.append( journal );
    return true;
  } else {
    kWarning() << "CalendarLocal::deleteJournal(): Journal not found." << endl;
    return false;
  }
}

void CalendarLocal::deleteAllJournals()
{
  foreach ( Journal *j, d->mJournals ) {
    notifyIncidenceDeleted( j );
  }
  qDeleteAll( d->mJournals );
  d->mJournals.clear();
}

Journal *CalendarLocal::journal( const QString &uid )
{
  return d->mJournals.value( uid );
}

Journal::List CalendarLocal::rawJournals( JournalSortField sortField,
                                          SortDirection sortDirection )
{
  Journal::List journalList;
  foreach ( Journal *j, d->mJournals ) {
    journalList.append( j );
  }
  return sortJournals( &journalList, sortField, sortDirection );
}

Journal::List CalendarLocal::rawJournalsForDate( const QDate &date )
{
  Journal::List journalList;
  foreach ( Journal *j, d->mJournals ) {
    if ( j->dtStart().date() == date ) {
      journalList.append( j );
    }
  }
  return journalList;
}
