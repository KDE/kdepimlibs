/*
    This file is part of the kcal library.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KCAL_FREEBUSY_H
#define KCAL_FREEBUSY_H

#include <QtCore/QByteArray>
#include <QtCore/QList>

#include "period.h"
#include "incidencebase.h"

namespace KCal {

  class Calendar;
  typedef QList<Period> PeriodList;
/**
  This class provides information about free/busy time of a calendar user.
*/
class KCAL_EXPORT FreeBusy : public IncidenceBase
{
  public:
    FreeBusy();
    FreeBusy( const KDateTime &start, const KDateTime &end );
    FreeBusy( Calendar *calendar, const KDateTime &start,
              const KDateTime &end );
    explicit FreeBusy( const PeriodList & busyPeriods );

    ~FreeBusy();

    QByteArray type() const { return "FreeBusy"; }

    virtual void setDtStart( const KDateTime &dtStart );
    virtual KDateTime dtEnd() const;
    bool setDtEnd( const KDateTime &end );

    /**
      @copydoc
      IncidenceBase::shiftTimes()
    */
    virtual void shiftTimes(const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec);

    PeriodList busyPeriods() const;

    /** Adds a period to the freebusy list and sorts the list.  */
    void addPeriod( const KDateTime &start, const KDateTime &end );
    void addPeriod( const KDateTime &start, const Duration &dur );
    /** Adds a list of periods to the freebusy object and then sorts
     * that list. Use this if you are adding many items, instead of the
     * addPeriod method, to avoid sorting repeatedly.  */
    void addPeriods( const PeriodList & );
    void sortList();

    void merge( FreeBusy *freebusy );

  private:
    // Override virtual inherited method.
    bool accept( Visitor &v ) { return v.visit( this ); }

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
