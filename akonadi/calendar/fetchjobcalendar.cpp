/*
   Copyright (C) 2011 Sérgio Martins <sergio.martins@kdab.com>
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

#include "fetchjobcalendar.h"
#include "fetchjobcalendar_p.h"
#include "incidencefetchjob_p.h"

#include <akonadi/item.h>
#include <akonadi/collection.h>

using namespace Akonadi;
using namespace KCalCore;

FetchJobCalendarPrivate::FetchJobCalendarPrivate( FetchJobCalendar *qq ) : CalendarBasePrivate( qq )
                                                                         , m_isLoaded( false )
                                                                         , q( qq )
{
  IncidenceFetchJob *job = new IncidenceFetchJob();
  connect( job, SIGNAL(result(KJob*)),
           SLOT(slotSearchJobFinished(KJob*)) );
}

FetchJobCalendarPrivate::~FetchJobCalendarPrivate()
{
}

void FetchJobCalendarPrivate::slotSearchJobFinished( KJob *job )
{
  IncidenceFetchJob *searchJob = static_cast<Akonadi::IncidenceFetchJob*>( job );
  bool success = true;
  QString errorMessage;
  if ( searchJob->error() ) {
    success = false;
    errorMessage = searchJob->errorText();
    kWarning() << "Unable to fetch incidences:" << searchJob->errorText();
  } else {
    foreach( const Akonadi::Item &item, searchJob->items() ) {
      internalInsert( item );
    }
  }
  m_isLoaded = true;
  // emit loadFinished() in a delayed manner, due to freezes because of execs.
  QMetaObject::invokeMethod( q, "loadFinished", Qt::QueuedConnection,
                             Q_ARG( bool, success ), Q_ARG( QString, errorMessage ) );
}

FetchJobCalendar::FetchJobCalendar() : CalendarBase( new FetchJobCalendarPrivate( this ) )
{
}

FetchJobCalendar::~FetchJobCalendar()
{
}

bool FetchJobCalendar::isLoaded() const
{
   FetchJobCalendarPrivate *d = static_cast<FetchJobCalendarPrivate*>( d_ptr.data() );
   return d->m_isLoaded;
}

#include "fetchjobcalendar.moc"
#include "fetchjobcalendar_p.moc"
