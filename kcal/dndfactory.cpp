/*
  This file is part of the kcal library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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
  defines the DndFactory class.

  @brief
  vCalendar/iCalendar Drag-and-Drop object factory.

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "dndfactory.h"
#include "vcaldrag.h"
#include "icaldrag.h"
#include "calendar.h"
#include "calendarlocal.h"

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QDropEvent>
#include <QtGui/QPixmap>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::DndFactory::Private
{
  public:
    Private( Calendar *cal )
      : mCalendar ( cal )
    {}
    Calendar *mCalendar;
};
//@endcond

DndFactory::DndFactory( Calendar *cal )
  : d( new KCal::DndFactory::Private ( cal ) )
{
}

DndFactory::~DndFactory()
{
  delete d;
}

QDrag *DndFactory::createDrag( QWidget *owner )
{
  QDrag *drag = new QDrag( owner );
  QMimeData *mimeData = new QMimeData;
  drag->setMimeData( mimeData );

  ICalDrag::populateMimeData( mimeData, d->mCalendar );
  VCalDrag::populateMimeData( mimeData, d->mCalendar );

  return drag;
}

QDrag *DndFactory::createDrag( Incidence *incidence, QWidget *owner )
{
  CalendarLocal cal( d->mCalendar->timeSpec() );
  Incidence *i = incidence->clone();
  cal.addIncidence( i );

  QDrag *drag = new QDrag( owner );
  QMimeData *mimeData = new QMimeData;
  drag->setMimeData( mimeData );

  ICalDrag::populateMimeData( mimeData, &cal );
  VCalDrag::populateMimeData( mimeData, &cal );

  KUrl uri = i->uri();
  if ( uri.isValid() ) {
    QMap<QString, QString> metadata;
    metadata["labels"] = KUrl::toPercentEncoding( i->summary() );
    uri.populateMimeData( mimeData, metadata );
  }

  if ( i->type() == "Event" ) {
    drag->setPixmap( BarIcon( "appointment" ) );
  } else if ( i->type() == "Todo" ) {
    drag->setPixmap( BarIcon( "todo" ) );
  }

  return drag;
}

Calendar *DndFactory::createDropCalendar( const QMimeData *md )
{
  Calendar *cal = new CalendarLocal( d->mCalendar->timeSpec() );

  if ( ICalDrag::fromMimeData( md, cal ) ||
       VCalDrag::fromMimeData( md, cal ) ){
    return cal;
  }
  return 0;
}

Calendar *DndFactory::createDropCalendar( QDropEvent *de )
{
  Calendar *cal = createDropCalendar( de->mimeData() );
  if ( cal ) {
    de->accept();
    return cal;
  }
  return 0;
}

Event *DndFactory::createDropEvent( QDropEvent *de )
{
  kDebug();
  Event *ev = 0;
  Calendar *cal = createDropCalendar( de );

  if ( cal ) {
    Event::List events = cal->events();
    if ( !events.isEmpty() ) {
      ev = new Event( *events.first() );
    }
    delete cal;
  }
  return ev;
}

Todo *DndFactory::createDropTodo( QDropEvent *de )
{
  kDebug();
  Todo *todo = 0;
  Calendar *cal = createDropCalendar( de );

  if ( cal ) {
    Todo::List todos = cal->todos();
    if ( !todos.isEmpty() ) {
      todo = new Todo( *todos.first() );
    }
    delete cal;
  }

  return todo;
}

void DndFactory::cutIncidence( Incidence *selectedInc )
{
  if ( copyIncidence( selectedInc ) ) {
    d->mCalendar->deleteIncidence( selectedInc );
  }
}

bool DndFactory::copyIncidence( Incidence *selectedInc )
{
  if ( !selectedInc ) {
    return false;
  }

  QClipboard *cb = QApplication::clipboard();

  CalendarLocal cal( d->mCalendar->timeSpec() );
  Incidence *inc = selectedInc->clone();
  cal.addIncidence( inc );

  QMimeData *mimeData = new QMimeData;
  cb->setMimeData( mimeData );

  ICalDrag::populateMimeData( mimeData, &cal );
  VCalDrag::populateMimeData( mimeData, &cal );

  return true;
}

Incidence *DndFactory::pasteIncidence( const QDate &newDate, const QTime *newTime )
{
  QClipboard *cb = QApplication::clipboard();
  Calendar *cal = createDropCalendar( cb->mimeData() );

  if ( !cal ) {
    kDebug() << "Can't parse clipboard";
    return 0;
  }
  Incidence *ret = 0;

  Incidence::List incList = cal->incidences();
  Incidence *inc = incList.first();

  if ( !incList.isEmpty() && inc ) {
    inc = inc->clone();

    inc->recreate();

    if ( inc->type() == "Event" ) {

      Event *anEvent = static_cast<Event*>( inc );
      // Calculate length of event
      int daysOffset = anEvent->dtStart().date().daysTo(
        anEvent->dtEnd().date() );
      // new end date if event starts at the same time on the new day
      KDateTime endDate( anEvent->dtEnd() );
      endDate.setDate( newDate.addDays(daysOffset) );

      KDateTime startDate( anEvent->dtStart() );
      startDate.setDate( newDate );
      if ( newTime ) {
        // additional offset for new time of day
        int addSecsOffset( anEvent->dtStart().time().secsTo( *newTime ) );
        endDate=endDate.addSecs( addSecsOffset );
        startDate.setTime( *newTime );
      }
      anEvent->setDtStart( startDate );
      anEvent->setDtEnd( endDate );

    } else if ( inc->type() == "Todo" ) {
      Todo *anTodo = static_cast<Todo*>( inc );
      KDateTime dueDate( anTodo->dtDue() );
      dueDate.setDate( newDate );
      if ( newTime ) {
        dueDate.setTime( *newTime );
      }
      anTodo->setDtDue( dueDate );
    } else if ( inc->type() == "Journal" ) {
      Journal *anJournal = static_cast<Journal*>( inc );
      KDateTime startDate( anJournal->dtStart() );
      startDate.setDate( newDate );
      if ( newTime ) {
        startDate.setTime( *newTime );
      } else {
        startDate.setTime( QTime( 0, 0, 0 ) );
      }
      anJournal->setDtStart( startDate );
    } else {
      kDebug() << "Trying to paste unknown incidence of type" << inc->type();
    }

    ret = inc;
  }
  delete cal;
  return ret;
}
