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

#include <kurl.h>
#include <kdirwatch.h>


#include "resourcecached.h"

#include "kcal.h"

class QString;
class KConfig;

namespace KCal {

class CalendarLocal;
class Incidence;

/**
  \internal

  This class provides a calendar stored as a file per incidence in a directory.
*/
class KCAL_EXPORT ResourceLocalDir : public ResourceCached
{
    Q_OBJECT

    friend class ResourceLocalDirConfig;

  public:
    ResourceLocalDir();
    explicit ResourceLocalDir( const KConfigGroup &group );
    explicit ResourceLocalDir( const QString& fileName );
    virtual ~ResourceLocalDir();

    void readConfig( const KConfigGroup &group );
    void writeConfig( KConfigGroup &group );

    KABC::Lock *lock();

    /** deletes an event from this calendar. */
    bool deleteEvent(Event *);

    /**
      Remove a todo from the todolist.
    */
    bool deleteTodo( Todo * );

    /**
      Remove a journal from the journallist.
    */
    bool deleteJournal( Journal * );

    void dump() const;

  protected Q_SLOTS:
    void reload( const QString & );

  protected:
    virtual bool doLoad( bool syncCache );
    virtual bool doSave( bool syncCache );
    bool doSave( bool syncCache, Incidence * );
    virtual bool doFileLoad( CalendarLocal &, const QString &fileName );

  private:
    void init();
    bool deleteIncidenceFile(Incidence *incidence);

    KUrl mURL;
//     ICalFormat mFormat;
    KDirWatch mDirWatch;
    KABC::Lock *mLock;

    class Private;
    Private *const d;
};

}

#endif
