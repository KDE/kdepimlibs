/*
  This file is part of the kcal library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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

#include <QApplication>
#include <QClipboard>
#include <QDropEvent>
#include <QPixmap>

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

    Incidence * pasteIncidence( Incidence *inc,
                                const QDate &newDate,
                                const QTime *newTime = 0 )
    {
      if ( inc ) {
        inc = inc->clone();
        inc->recreate();
      }

      if ( inc && newDate.isValid() ) {
        if ( inc->type() == "Event" ) {

          Event *anEvent = static_cast<Event*>( inc );
          // Calculate length of event
          int daysOffset = anEvent->dtStart().date().daysTo(
            anEvent->dtEnd().date() );
          // new end date if event starts at the same time on the new day
          KDateTime endDate( anEvent->dtEnd() );
          endDate.setDate( newDate.addDays( daysOffset ) );

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
      }

      return inc;
    }

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

QMimeData *DndFactory::createMimeData()
{
  QMimeData *mimeData = new QMimeData;

  ICalDrag::populateMimeData( mimeData, d->mCalendar );
  VCalDrag::populateMimeData( mimeData, d->mCalendar );

  return mimeData;
}

QDrag *DndFactory::createDrag( QWidget *owner )
{
  QDrag *drag = new QDrag( owner );
  drag->setMimeData( createMimeData() );

  return drag;
}

QMimeData *DndFactory::createMimeData( Incidence *incidence )
{
  CalendarLocal cal( d->mCalendar->timeSpec() );
  Incidence *i = incidence->clone();
  cal.addIncidence( i );

  QMimeData *mimeData = new QMimeData;

  ICalDrag::populateMimeData( mimeData, &cal );
  VCalDrag::populateMimeData( mimeData, &cal );

  KUrl uri = i->uri();
  if ( uri.isValid() ) {
    QMap<QString, QString> metadata;
    metadata["labels"] = KUrl::toPercentEncoding( i->summary() );
    uri.populateMimeData( mimeData, metadata );
  }

  return mimeData;
}

QDrag *DndFactory::createDrag( Incidence *incidence, QWidget *owner )
{
  QDrag *drag = new QDrag( owner );
  drag->setMimeData( createMimeData( incidence ) );

  if ( incidence->type() == "Event" ) {
    drag->setPixmap( BarIcon( "view-calendar-day" ) );
  } else if ( incidence->type() == "Todo" ) {
    drag->setPixmap( BarIcon( "view-calendar-tasks" ) );
  }

  return drag;
}

Calendar *DndFactory::createDropCalendar( const QMimeData *md )
{
  return createDropCalendar( md, d->mCalendar->timeSpec() );
}

Calendar *DndFactory::createDropCalendar( const QMimeData *md, const KDateTime::Spec &timeSpec )
{
  Calendar *cal = new CalendarLocal( timeSpec );

  if ( ICalDrag::fromMimeData( md, cal ) ||
       VCalDrag::fromMimeData( md, cal ) ){
    return cal;
  }
  delete cal;
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

Event *DndFactory::createDropEvent( const QMimeData *md )
{
  kDebug();
  Event *ev = 0;
  Calendar *cal = createDropCalendar( md );

  if ( cal ) {
    Event::List events = cal->events();
    if ( !events.isEmpty() ) {
      ev = new Event( *events.first() );
    }
    delete cal;
  }
  return ev;
}

Event *DndFactory::createDropEvent( QDropEvent *de )
{
  Event *ev = createDropEvent( de->mimeData() );

  if ( ev ) {
    de->accept();
  }

  return ev;
}

Todo *DndFactory::createDropTodo( const QMimeData *md )
{
  kDebug();
  Todo *todo = 0;
  Calendar *cal = createDropCalendar( md );

  if ( cal ) {
    Todo::List todos = cal->todos();
    if ( !todos.isEmpty() ) {
      todo = new Todo( *todos.first() );
    }
    delete cal;
  }

  return todo;
}

Todo *DndFactory::createDropTodo( QDropEvent *de )
{
  Todo *todo = createDropTodo( de->mimeData() );

  if ( todo ) {
    de->accept();
  }

  return todo;
}

void DndFactory::cutIncidence( Incidence *selectedInc )
{
  Incidence::List list;
  list.append( selectedInc );
  cutIncidences( list );
}

bool DndFactory::cutIncidences( const Incidence::List &incidences )
{
  if ( copyIncidences( incidences ) ) {
    Incidence::List::ConstIterator it;
    for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
      d->mCalendar->deleteIncidence( *it );
    }
    return true;
  } else {
    return false;
  }
}

bool DndFactory::copyIncidences( const Incidence::List &incidences )
{
  QClipboard *cb = QApplication::clipboard();
  CalendarLocal cal( d->mCalendar->timeSpec() );

  Incidence::List::ConstIterator it;
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    if ( *it ) {
      cal.addIncidence( ( *it )->clone() );
    }
  }

  QMimeData *mimeData = new QMimeData;

  ICalDrag::populateMimeData( mimeData, &cal );
  VCalDrag::populateMimeData( mimeData, &cal );

  if ( cal.incidences().isEmpty() ) {
    return false;
  } else {
    cb->setMimeData( mimeData );
    return true;
  }
}

bool DndFactory::copyIncidence( Incidence *selectedInc )
{
  Incidence::List list;
  list.append( selectedInc );
  return copyIncidences( list );
}

Incidence::List DndFactory::pasteIncidences( const QDate &newDate,
                                             const QTime *newTime )
{
  QClipboard *cb = QApplication::clipboard();
  Calendar *cal = createDropCalendar( cb->mimeData() );
  Incidence::List list;

  if ( !cal ) {
    kDebug() << "Can't parse clipboard";
    return list;
  }

  // All pasted incidences get new uids, must keep track of old uids,
  // so we can update child's parents
  QHash<QString,Incidence*> oldUidToNewInc;

  Incidence::List::ConstIterator it;
  const Incidence::List incs = cal->incidences();
  for ( it = incs.constBegin();
        it != incs.constEnd(); ++it ) {
    Incidence *inc = d->pasteIncidence( *it, newDate, newTime );
    if ( inc ) {
      list.append( inc );
      oldUidToNewInc[( *it )->uid()] = inc;
    }
  }

  // update relations
  for ( it = list.constBegin(); it != list.constEnd(); ++it ) {
    Incidence *inc = *it;
    if ( oldUidToNewInc.contains( inc->relatedToUid() ) ) {
      Incidence *parentInc = oldUidToNewInc[inc->relatedToUid()];
      inc->setRelatedToUid( parentInc->uid() );
      inc->setRelatedTo( parentInc );
    } else {
      // not related to anything in the clipboard
      inc->setRelatedToUid( QString() );
      inc->setRelatedTo( 0 );
    }
  }

  return list;
}

Incidence *DndFactory::pasteIncidence( const QDate &newDate, const QTime *newTime )
{
  QClipboard *cb = QApplication::clipboard();
  Calendar *cal = createDropCalendar( cb->mimeData() );

  if ( !cal ) {
    kDebug() << "Can't parse clipboard";
    return 0;
  }

  Incidence::List incList = cal->incidences();
  Incidence *inc = incList.isEmpty() ? 0 : incList.first();

  Incidence *newInc = d->pasteIncidence( inc, newDate, newTime );
  newInc->setRelatedTo( 0 );
  return newInc;
}
