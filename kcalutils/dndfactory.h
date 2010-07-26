/*
  This file is part of the kcalutils library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#ifndef KCALUTILS_DNDFACTORY_H
#define KCALUTILS_DNDFACTORY_H

#include "kcalutils_export.h"

#include <kcalcore/event.h>
#include <kcalcore/journal.h>
#include <kcalcore/todo.h>
#include <kcalcore/memorycalendar.h>

#include <kdatetime.h>


class QDrag;
class QDropEvent;
class QMimeData;

namespace KCalUtils {

/**
  @brief
  vCalendar/iCalendar Drag-and-Drop object factory.

  This class implements functions to create Drag and Drop objects used for
  Drag-and-Drop and Copy-and-Paste.
*/
class KCALUTILS_EXPORT DndFactory
{
  public:
    explicit DndFactory( const KCalCore::MemoryCalendar::Ptr &cal );

    ~DndFactory();

    /**
      Create the calendar that is contained in the drop event's data.
     */
    KCalCore::MemoryCalendar::Ptr createDropCalendar( QDropEvent *de );

    /**
      Create the calendar that is contained in the mime data.
     */
    KCalCore::MemoryCalendar::Ptr createDropCalendar( const QMimeData *md );

     /**
      Create the calendar that is contained in the mime data.
     */
    static KCalCore::MemoryCalendar::Ptr createDropCalendar( const QMimeData *md,
                                                             const KDateTime::Spec &timeSpec );

    /**
      Create the mime data for the whole calendar.
    */
    QMimeData *createMimeData();

    /**
      Create a drag object for the whole calendar.
    */
    QDrag *createDrag( QWidget *owner );

    /**
      Create the mime data for a single incidence.
    */
    QMimeData *createMimeData( const KCalCore::Incidence::Ptr &incidence );

    /**
      Create a drag object for a single incidence.
    */
    QDrag *createDrag( const KCalCore::Incidence::Ptr &incidence, QWidget *owner );

    /**
      Create Todo object from mime data.
    */
    KCalCore::Todo::Ptr createDropTodo( const QMimeData *md );

    /**
      Create Todo object from drop event.
    */
    KCalCore::Todo::Ptr createDropTodo( QDropEvent *de );

    /**
      Create Event object from mime data.
    */
    KCalCore::Event::Ptr createDropEvent( const QMimeData *md );

    /**
      Create Event object from drop event.
    */
    KCalCore::Event::Ptr createDropEvent( QDropEvent *de );

    /**
      Cut the incidence to the clipboard.
    */
    void cutIncidence( const KCalCore::Incidence::Ptr & );

    /**
      Copy the incidence to clipboard/
    */
    bool copyIncidence( const KCalCore::Incidence::Ptr & );

    /**
      Cuts a list of @p incidences to the clipboard.
    */
    bool cutIncidences( const KCalCore::Incidence::List &incidences );

    /**
      Copies a list of @p incidences to the clipboard.
    */
    bool copyIncidences( const KCalCore::Incidence::List &incidences );

    /**
      Pastes and returns the incidences from the clipboard
      If no date and time are given, the incidences will be pasted at
      their original date/time

      @param newDate The new date where the incidences shall be pasted.
      @param newTime The new time where the incidences shall be pasted.
    */
    KCalCore::Incidence::List pasteIncidences( const QDate &newDate = QDate(),
                                               const QTime *newTime = 0 );

    /**
      Pastes the event or todo and return a pointer to the new incidence pasted.
    */
    KCalCore::Incidence::Ptr pasteIncidence( const QDate &, const QTime *newTime = 0 );

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( DndFactory )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
