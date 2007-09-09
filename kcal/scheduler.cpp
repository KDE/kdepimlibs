/*
  This file is part of the kcal library.

  Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>
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

#include "scheduler.h"

#include "event.h"
#include "todo.h"
#include "freebusy.h"
#include "icalformat.h"
#include "calendar.h"
#include "freebusycache.h"

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

using namespace KCal;

ScheduleMessage::ScheduleMessage( IncidenceBase *incidence, int method,
                                  ScheduleMessage::Status status )
  : d( 0 )
{
  mIncidence = incidence;
  mMethod = method;
  mStatus = status;
}

IncidenceBase *ScheduleMessage::event()
{
  return mIncidence;
}

int ScheduleMessage::method()
{
  return mMethod;
}

ScheduleMessage::Status ScheduleMessage::status()
{
  return mStatus;
}

QString ScheduleMessage::statusName( ScheduleMessage::Status status )
{
  switch( status ) {
  case PublishNew:
    return i18nc( "@item", "Publish" );
  case PublishUpdate:
    return i18nc( "@item", "Updated Publish" );
  case Obsolete:
    return i18nc( "@item", "Obsolete" );
  case RequestNew:
    return i18nc( "@item", "New Request" );
  case RequestUpdate:
    return i18nc( "@item", "Updated Request" );
  default:
    return i18nc( "@item", "Unknown Status: %1", status );
  }
}

QString ScheduleMessage::error()
{
  return mError;
}

//@cond PRIVATE
struct KCal::Scheduler::Private
{
  Private() : mFreeBusyCache( 0 ) {}

  FreeBusyCache *mFreeBusyCache;
};
//@endcond

Scheduler::Scheduler( Calendar *calendar ) : d( 0 )
{
  mCalendar = calendar;
  mFormat = new ICalFormat();
  mFormat->setTimeSpec( calendar->timeSpec() );
}

Scheduler::~Scheduler()
{
  delete mFormat;
}

void Scheduler::setFreeBusyCache( FreeBusyCache *c )
{
  d->mFreeBusyCache = c;
}

FreeBusyCache *Scheduler::freeBusyCache() const
{
  return d->mFreeBusyCache;
}

bool Scheduler::acceptTransaction( IncidenceBase *incidence, Method method,
                                   ScheduleMessage::Status status )
{
  kDebug(5800) << "Scheduler::acceptTransaction, method=" << methodName( method );

  switch ( method ) {
  case Publish:
    return acceptPublish( incidence, status, method );
  case Request:
    return acceptRequest( incidence, status );
  case Add:
    return acceptAdd( incidence, status );
  case Cancel:
    return acceptCancel( incidence, status );
  case Declinecounter:
    return acceptDeclineCounter( incidence, status );
  case Reply:
    return acceptReply( incidence, status, method );
  case Refresh:
    return acceptRefresh( incidence, status );
  case Counter:
    return acceptCounter( incidence, status );
  default:
    break;
  }
  deleteTransaction( incidence );
  return false;
}

QString Scheduler::methodName( Method method )
{
  switch ( method ) {
  case Publish:
    return QLatin1String( "Publish" );
  case Request:
    return QLatin1String( "Request" );
  case Refresh:
    return QLatin1String( "Refresh" );
  case Cancel:
    return QLatin1String( "Cancel" );
  case Add:
    return QLatin1String( "Add" );
  case Reply:
    return QLatin1String( "Reply" );
  case Counter:
    return QLatin1String( "Counter" );
  case Declinecounter:
    return QLatin1String( "Decline Counter" );
  default:
    return QLatin1String( "Unknown" );
  }
}

QString Scheduler::translatedMethodName( Method method )
{
  switch ( method ) {
  case Publish:
    return i18nc( "@item event, to-do, journal or freebusy posting", "Publish" );
  case Request:
    return i18nc( "@item event, to-do or freebusy scheduling requests", "Request" );
  case Reply:
    return i18nc( "@item event, to-do or freebusy reply to request", "Reply" );
  case Add:
    return i18nc(
      "@item event, to-do or journal additional properties request", "Add" );
  case Cancel:
    return i18nc( "@item event, to-do or journal cancellation notice", "Cancel" );
  case Refresh:
    return i18nc( "@item event or to-do description update request", "Refresh" );
  case Counter:
    return i18nc( "@item event or to-do description counter proposal submission", "Counter" );
  case Declinecounter:
    return i18nc( "@item event or to-do decline a counter proposal", "Decline Counter" );
  default:
    return i18nc( "@item no method", "Unknown" );
  }
}

bool Scheduler::deleteTransaction(IncidenceBase *)
{
  return true;
}

bool Scheduler::acceptPublish( IncidenceBase *newIncBase,
                               ScheduleMessage::Status status, Method method )
{
  if( newIncBase->type() == "FreeBusy" ) {
    return acceptFreeBusy( newIncBase, method );
  }

  bool res = false;
  kDebug(5800) << "Scheduler::acceptPublish, status="
            << ScheduleMessage::statusName( status );
  Incidence *newInc = static_cast<Incidence *>( newIncBase );
  Incidence *calInc = mCalendar->incidence( newIncBase->uid() );
  switch ( status ) {
    case ScheduleMessage::Unknown:
    case ScheduleMessage::PublishNew:
    case ScheduleMessage::PublishUpdate:
      res = true;
      if ( calInc ) {
        if ( ( newInc->revision() > calInc->revision() ) ||
             ( newInc->revision() == calInc->revision() &&
               newInc->lastModified() > calInc->lastModified() ) ) {
          mCalendar->deleteIncidence( calInc );
        } else {
          res = false;
        }
      }
      if ( res ) {
        mCalendar->addIncidence( newInc );
      }
      break;
    case ScheduleMessage::Obsolete:
      res = true;
      break;
    default:
      break;
  }
  deleteTransaction( newIncBase );
  return res;
}

bool Scheduler::acceptRequest( IncidenceBase *newIncBase, ScheduleMessage::Status /* status */)
{
  if ( newIncBase->type() == "FreeBusy" ) {
    // reply to this request is handled in korganizer's incomingdialog
    return true;
  }
  Incidence *newInc = dynamic_cast<Incidence *>( newIncBase );
  if ( newInc ) {
    bool res = true;
    Incidence *exInc = mCalendar->incidenceFromSchedulingID( newIncBase->uid() );
    if ( exInc ) {
      res = false;
      if ( ( newInc->revision() > exInc->revision() ) ||
           ( newInc->revision() == exInc->revision() &&
             newInc->lastModified()>exInc->lastModified() ) ) {
        mCalendar->deleteIncidence( exInc );
        res = true;
      }
    }
    if ( res ) {
      // Move the uid to be the schedulingID and make a unique UID
      newInc->setSchedulingID( newInc->uid() );
      newInc->setUid( CalFormat::createUniqueId() );

      mCalendar->addIncidence( newInc );
    }
    deleteTransaction( newIncBase );
    return res;
  }
  return false;
}

bool Scheduler::acceptAdd( IncidenceBase *incidence, ScheduleMessage::Status /* status */)
{
  deleteTransaction(incidence);
  return false;
}

bool Scheduler::acceptCancel( IncidenceBase *incidence, ScheduleMessage::Status /* status */)
{
  bool ret = false;
  const IncidenceBase *toDelete = mCalendar->incidenceFromSchedulingID( incidence->uid() );
  if ( toDelete ) {
    Event *event = mCalendar->event( toDelete->uid() );
    if ( event ) {
      mCalendar->deleteEvent( event );
      ret = true;
    } else {
      Todo *todo = mCalendar->todo( toDelete->uid() );
      if ( todo ) {
        mCalendar->deleteTodo( todo );
        ret = true;
      }
    }
  }
  deleteTransaction( incidence );
  return ret;
}

bool Scheduler::acceptDeclineCounter( IncidenceBase *incidence, ScheduleMessage::Status status )
{
  Q_UNUSED( status );
  deleteTransaction( incidence );
  return false;
}

bool Scheduler::acceptReply( IncidenceBase *incidence, ScheduleMessage::Status status,
                             Method method )
{
  Q_UNUSED( status );
  if ( incidence->type() == "FreeBusy" ) {
    return acceptFreeBusy( incidence, method );
  }
  bool ret = false;
  Event *ev = mCalendar->event( incidence->uid() );
  Todo *to = mCalendar->todo( incidence->uid() );

  // try harder to find the correct incidence
  if ( !ev && !to ) {
    Incidence::List list = mCalendar->incidences();
    for ( Incidence::List::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it ) {
      if ( (*it)->schedulingID() == incidence->uid() ) {
        ev = dynamic_cast<Event*>( *it );
        to = dynamic_cast<Todo*>( *it );
        break;
      }
    }
  }

  if ( ev || to ) {
    //get matching attendee in calendar
    kDebug(5800) << "Scheduler::acceptTransaction match found!";
    Attendee::List attendeesIn = incidence->attendees();
    Attendee::List attendeesEv;
    Attendee::List attendeesNew;
    if ( ev ) {
      attendeesEv = ev->attendees();
    }
    if ( to ) {
      attendeesEv = to->attendees();
    }
    Attendee::List::ConstIterator inIt;
    Attendee::List::ConstIterator evIt;
    for ( inIt = attendeesIn.begin(); inIt != attendeesIn.end(); ++inIt ) {
      Attendee *attIn = *inIt;
      bool found = false;
      for ( evIt = attendeesEv.begin(); evIt != attendeesEv.end(); ++evIt ) {
        Attendee *attEv = *evIt;
        if ( attIn->email().toLower() == attEv->email().toLower() ) {
          //update attendee-info
          kDebug(5800) << "Scheduler::acceptTransaction update attendee";
          attEv->setStatus( attIn->status() );
          attEv->setDelegate( attIn->delegate() );
          attEv->setDelegator( attIn->delegator() );
          ret = true;
          found = true;
        }
      }
      if ( !found && attIn->status() != Attendee::Declined ) {
        attendeesNew.append( attIn );
      }
    }

    bool attendeeAdded = false;
    for ( Attendee::List::ConstIterator it = attendeesNew.constBegin();
          it != attendeesNew.constEnd(); ++it ) {
      Attendee *attNew = *it;
      QString msg =
        i18n( "%1 wants to attend %2 but was not invited.", attNew->fullName(),
              ( ev ? ev->summary() : to->summary() ) );
      if ( !attNew->delegator().isEmpty() ) {
        msg =
          i18n( "%1 wants to attend %2 on behalf of %3.", attNew->fullName(),
                ( ev ? ev->summary() : to->summary() ), attNew->delegator() );
      }
      if ( KMessageBox::questionYesNo(
             0, msg, i18n( "Uninvited attendee" ),
             KGuiItem( i18n( "Accept Attendance" ) ),
             KGuiItem( i18n( "Reject Attendance" ) ) ) != KMessageBox::Yes ) {
        KCal::Incidence *cancel = dynamic_cast<Incidence*>( incidence );
        if ( cancel ) {
          cancel->addComment( i18n( "The organizer rejected your attendance at this meeting." ) );
        }
        performTransaction( cancel ? cancel : incidence, Scheduler::Cancel, attNew->fullName() );
        delete cancel;
        continue;
      }

      Attendee *a = new Attendee( attNew->name(), attNew->email(), attNew->RSVP(),
                                  attNew->status(), attNew->role(), attNew->uid() );
      a->setDelegate( attNew->delegate() );
      a->setDelegator( attNew->delegator() );
      if ( ev ) {
        ev->addAttendee( a );
      } else if ( to ) {
        to->addAttendee( a );
      }
      ret = true;
      attendeeAdded = true;
    }

    // send update about new participants
    if ( attendeeAdded ) {
      if ( ev ) {
        ev->setRevision( ev->revision() + 1 );
        performTransaction( ev, Scheduler::Request );
      }
      if ( to ) {
        to->setRevision( ev->revision() + 1 );
        performTransaction( to, Scheduler::Request );
      }
    }

    if ( ret ) {
      // We set at least one of the attendees, so the incidence changed
      // Note: This should not result in a sequence number bump
      if ( ev ) {
        ev->updated();
      } else if ( to ) {
        to->updated();
      }
    }
    if ( to ) {
      // for VTODO a REPLY can be used to update the completion status of
      // a task. see RFC2446 3.4.3
      Todo *update = dynamic_cast<Todo*> ( incidence );
      Q_ASSERT( update );
      if ( update && ( to->percentComplete() != update->percentComplete() ) ) {
        to->setPercentComplete( update->percentComplete() );
        to->updated();
      }
    }
  } else {
    kError(5800) << "No incidence for scheduling\n";
  }

  if ( ret ) {
    deleteTransaction( incidence );
  }
  return ret;
}

bool Scheduler::acceptRefresh( IncidenceBase *incidence, ScheduleMessage::Status status )
{
  Q_UNUSED( status );
  // handled in korganizer's IncomingDialog
  deleteTransaction( incidence );
  return false;
}

bool Scheduler::acceptCounter( IncidenceBase *incidence, ScheduleMessage::Status status )
{
  Q_UNUSED( status );
  deleteTransaction( incidence );
  return false;
}

bool Scheduler::acceptFreeBusy( IncidenceBase *incidence, Method method )
{
  if ( !d->mFreeBusyCache ) {
    kError() << "KCal::Scheduler: no FreeBusyCache.";
    return false;
  }

  FreeBusy *freebusy = static_cast<FreeBusy *>(incidence);

  kDebug(5800) << "acceptFreeBusy:: freeBusyDirName:" << freeBusyDir();

  Person from;
  if( method == Scheduler::Publish ) {
    from = freebusy->organizer();
  }
  if ( ( method == Scheduler::Reply ) && ( freebusy->attendeeCount() == 1 ) ) {
    Attendee *attendee = freebusy->attendees().first();
    from.setName( attendee->name() );
    from.setEmail( attendee->email() );
  }

  if ( !d->mFreeBusyCache->saveFreeBusy( freebusy, from ) ) {
    return false;
  }

  deleteTransaction( incidence );
  return true;
}
