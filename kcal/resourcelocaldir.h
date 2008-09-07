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
#ifndef KCAL_RESOURCELOCALDIRDIR_H
#define KCAL_RESOURCELOCALDIRDIR_H

#include "kcal_export.h"
#include "resourcecached.h"

namespace KCal {

/**
  @brief
  This class provides a calendar stored as a file per incidence in a directory.
*/
class KCAL_EXPORT ResourceLocalDir : public ResourceCached
{
  Q_OBJECT
  friend class ResourceLocalDirConfig;

  public:
    ResourceLocalDir();
    explicit ResourceLocalDir( const KConfigGroup &group );
    explicit ResourceLocalDir( const QString &fileName );
    virtual ~ResourceLocalDir();

    void readConfig( const KConfigGroup &group );
    void writeConfig( KConfigGroup &group );

    KABC::Lock *lock();

    /** deletes an event from this calendar. */
    bool deleteEvent( Event *event );

    /** Removes all Events from this calendar. */
    void deleteAllEvents();

    /**
      Remove a todo from the todolist.
    */
    bool deleteTodo( Todo *todo );

    /**
      Removes all todos from this calendar.
    */
    void deleteAllTodos();

    /**
      Remove a journal from the journallist.
    */
    bool deleteJournal( Journal *journal );

    /**
      Removes all journals from this calendar.
    */
    void deleteAllJournals();

    void dump() const;

  protected Q_SLOTS:
    void reload( const QString &file );

  protected:
    bool doOpen();
    virtual bool doLoad( bool syncCache );
    virtual bool doSave( bool syncCache );
    bool doSave( bool syncCache, Incidence *incidence );
    virtual bool doFileLoad( CalendarLocal &cal, const QString &fileName );

  private:
    // Inherited virtual methods which should not be used by derived classes.
    using ResourceCalendar::doLoad;
    using ResourceCalendar::doSave;

    //@cond PRIVATE
    Q_DISABLE_COPY( ResourceLocalDir )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
