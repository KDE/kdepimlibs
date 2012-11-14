/*
   Copyright (C) 2012 Sérgio Martins <iamsergio@gmail.com>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef _AKONADI_CALENDAR_CLIPBOARD_H
#define _AKONADI_CALENDAR_CLIPBOARD_H

#include "etmcalendar.h"
#include "akonadi-calendar_export.h"

#include <kcalcore/incidence.h>
#include <QObject>

namespace Akonadi {
//TODO_SERGIO: Documentation and unit-tests.
class IncidenceChanger;

class AKONADI_CALENDAR_EXPORT CalendarClipboard : public QObject {
  Q_OBJECT
public:

  enum Mode {
    SingleMode = 0, ///< Only the specified incidence is cut/copied.
    RecursiveMode,  ///< The specified incidence's children are also cut/copied
    AskMode         ///< The user is asked if he wants children to be cut/copied too
  };

  explicit CalendarClipboard( const Akonadi::CalendarBase::Ptr &calendar,
                              Akonadi::IncidenceChanger *changer = 0,
                              QObject *parent = 0 );
  ~CalendarClipboard();

  /**
   * Copies the specified incidence into the clipboard and then deletes it from akonadi.
   * The incidence must be present in the calendar.
   *
   * After it's deletion from akonadi, signal cutFinished() is emitted.
   * 
   * @see cutFinished().
   */
  void cutIncidence( const KCalCore::Incidence::Ptr &incidence,
                     CalendarClipboard::Mode mode = RecursiveMode );


  /**
   * Copies the specified incidence into the clipboard.
   *
   * @return true on success
   */
  bool copyIncidence( const KCalCore::Incidence::Ptr &incidence,
                      CalendarClipboard::Mode mode = RecursiveMode );

  /**
   * Returns if there's any ical mime data available for pasting
   */
  bool pasteAvailable() const;

Q_SIGNALS:
  /**
   * Emitted after cutIncidences() finishes.
   * @param success true if the cut was successful
   * @param errorMessage if @p success if false, contains the error message, empty otherwise.
   */
  void cutFinished( bool success, const QString &errorMessage );

private:
  class Private;
  Private *const d;
};

}

#endif
