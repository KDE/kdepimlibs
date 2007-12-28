/*
  This file is part of the kcal library.

  Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_IMIPSCHEDULER_H
#define KCAL_IMIPSCHEDULER_H

#include <QtCore/QList>
#include "kcal_export.h"

#include "scheduler.h"

namespace KCal {

/*
  @brief
  This class implements the iTIP interface using the email interface specified
  as iMIP.
*/
class KCAL_EXPORT IMIPScheduler : public Scheduler
{
  public:
    explicit IMIPScheduler( Calendar * );
    virtual ~IMIPScheduler();

    bool publish ( IncidenceBase *incidence, const QString &recipients );
    bool performTransaction( IncidenceBase *incidence, iTIPMethod method );
    QList<ScheduleMessage*> retrieveTransactions();

  private:
    using Scheduler::performTransaction;

    //@cond PRIVATE
    Q_DISABLE_COPY( IMIPScheduler )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
