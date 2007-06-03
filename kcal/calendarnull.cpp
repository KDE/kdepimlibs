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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CalendarNull class.

  @brief
  Represents a null calendar class; that is, a calendar which contains
  no information and provides no capabilities.

  @author Cornelius Schumacher
*/
#include "calendarnull.h"

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::CalendarNull::Private
{
};
static CalendarNull *mSelf = 0;
//@endcond

CalendarNull::CalendarNull( const KDateTime::Spec &timeSpec )
  : Calendar( timeSpec ),
    d( new KCal::CalendarNull::Private )
{}

CalendarNull::CalendarNull( const QString &timeZoneId )
  : Calendar( timeZoneId ),
    d( new KCal::CalendarNull::Private )
{}

CalendarNull::~CalendarNull()
{
  delete d;
}

CalendarNull *CalendarNull::self()
{
  if ( !mSelf ) {
    mSelf = new CalendarNull( KDateTime::UTC );
  }

  return mSelf;
}

void CalendarNull::close()
{
}

bool CalendarNull::save()
{
  return true;
}

bool CalendarNull::reload()
{
  return true;
}

bool CalendarNull::addEvent( Event *event )
{
  Q_UNUSED ( event );
  return false;
}

bool CalendarNull::deleteEvent( Event *event )
{
  Q_UNUSED( event );
  return false;
}

void CalendarNull::deleteAllEvents()
{
}

Event::List CalendarNull::rawEvents( EventSortField sortField,
                                     SortDirection sortDirection )
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Event::List();
}

Event::List CalendarNull::rawEvents( const QDate &start, const QDate &end,
                                     bool inclusive )
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_UNUSED( inclusive );
  return Event::List();
}

Event::List CalendarNull::rawEventsForDate( const QDate &date,
                                            EventSortField sortField,
                                            SortDirection sortDirection )
{
  Q_UNUSED( date );
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Event::List();
}

Event::List CalendarNull::rawEventsForDate( const KDateTime &dt )
{
  Q_UNUSED( dt );
  return Event::List();
}

Event *CalendarNull::event( const QString &uid )
{
  Q_UNUSED( uid );
  return 0;
}

bool CalendarNull::addTodo( Todo *todo )
{
  Q_UNUSED( todo );
  return false;
}

bool CalendarNull::deleteTodo( Todo *todo )
{
  Q_UNUSED( todo );
  return false;
}

void CalendarNull::deleteAllTodos()
{
}

Todo::List CalendarNull::rawTodos( TodoSortField sortField,
                                   SortDirection sortDirection )
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Todo::List();
}

Todo::List CalendarNull::rawTodosForDate( const QDate &date )
{
  Q_UNUSED( date );
  return Todo::List();
}

Todo *CalendarNull::todo( const QString &uid )
{
  Q_UNUSED( uid );
  return 0;
}

bool CalendarNull::addJournal( Journal *journal )
{
  Q_UNUSED( journal );
  return false;
}

bool CalendarNull::deleteJournal( Journal *journal )
{
  Q_UNUSED( journal );
  return false;
}

void CalendarNull::deleteAllJournals()
{
}

Journal::List CalendarNull::rawJournals( JournalSortField sortField,
                                         SortDirection sortDirection )
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Journal::List();
}

Journal::List CalendarNull::rawJournalsForDate( const QDate &date )
{
  Q_UNUSED( date );
  return Journal::List();
}

Journal *CalendarNull::journal( const QString &uid )
{
  Q_UNUSED( uid );
  return 0;
}

Alarm::List CalendarNull::alarms( const KDateTime &from, const KDateTime &to )
{
  Q_UNUSED( from );
  Q_UNUSED( to );
  return Alarm::List();
}

void CalendarNull::incidenceUpdated( IncidenceBase *incidenceBase )
{
  Q_UNUSED( incidenceBase );
}
