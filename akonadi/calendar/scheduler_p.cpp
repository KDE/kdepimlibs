/*
  Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2012-2013 Sérgio Martins <iamsergio@gmail.com>

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
#include "scheduler_p.h"
#include <kcalutils/stringify.h>

#include <kcalcore/icalformat.h>
#include <kcalcore/freebusycache.h>

#include <KDebug>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSystemTimeZones>

using namespace KCalCore;
using namespace Akonadi;

struct Akonadi::Scheduler::Private
{
  public:
    Private( Scheduler *qq ) : mFreeBusyCache( 0 )
                             , q( qq )
    {
    }

    FreeBusyCache *mFreeBusyCache;
    Scheduler *const q;
};

Scheduler::Scheduler( QObject *parent ) : QObject( parent )
                                        , d( new Akonadi::Scheduler::Private( this ) )
{
  mFormat = new ICalFormat();
  mFormat->setTimeSpec( KSystemTimeZones::local() );
}

Scheduler::~Scheduler()
{
  delete mFormat;
  delete d;
}

void Scheduler::setFreeBusyCache( FreeBusyCache *c )
{
  d->mFreeBusyCache = c;
}

FreeBusyCache *Scheduler::freeBusyCache() const
{
  return d->mFreeBusyCache;
}

void Scheduler::acceptTransaction( const IncidenceBase::Ptr &incidence,
                                   const Akonadi::CalendarBase::Ptr &calendar,
                                   iTIPMethod method,
                                   ScheduleMessage::Status status, const QString &email )
{
  Q_ASSERT( incidence );
  Q_ASSERT( calendar );
  kDebug() << "method=" << ScheduleMessage::methodName( method ); //krazy:exclude=kdebug
  connectCalendar( calendar );
  switch ( method ) {
  case iTIPPublish:
    acceptPublish( incidence, calendar, status, method );
    break;
  case iTIPRequest:
    acceptRequest( incidence, calendar, status, email );
    break;
  case iTIPAdd:
    acceptAdd( incidence, status );
    break;
  case iTIPCancel:
    acceptCancel( incidence, calendar, status, email );
    break;
  case iTIPDeclineCounter:
    acceptDeclineCounter( incidence, status );
    break;
  case iTIPReply:
    acceptReply( incidence,calendar, status, method );
    break;
  case iTIPRefresh:
    acceptRefresh( incidence, status );
    break;
  case iTIPCounter:
    acceptCounter( incidence, status );
    break;
  default:
    kWarning() << "Unhandled method: " << method;
  }
}

void Scheduler::acceptPublish( const IncidenceBase::Ptr &newIncBase,
                               const Akonadi::CalendarBase::Ptr &calendar,
                               ScheduleMessage::Status status,
                               iTIPMethod method )
{
  if ( newIncBase->type() == IncidenceBase::TypeFreeBusy ) {
    acceptFreeBusy( newIncBase, method );
    return;
  }

  QString errorString;
  Result result = ResultSuccess;

  kDebug() << "status="
           << KCalUtils::Stringify::scheduleMessageStatus( status ); //krazy:exclude=kdebug

  Incidence::Ptr newInc = newIncBase.staticCast<Incidence>() ;
  Incidence::Ptr calInc = calendar->incidence( newIncBase->uid() );
  switch ( status ) {
    case ScheduleMessage::Unknown:
    case ScheduleMessage::PublishNew:
    case ScheduleMessage::PublishUpdate:
      if ( calInc && newInc ) {
        if ( ( newInc->revision() > calInc->revision() ) ||
             ( newInc->revision() == calInc->revision() &&
               newInc->lastModified() > calInc->lastModified() ) ) {
          const QString oldUid = calInc->uid();

          if ( calInc->type() != newInc->type() ) {
            result = ResultAssigningDifferentTypes;
            errorString = i18n( "Error: Assigning different incidence types." );
            kError() << errorString;
          } else {
            newInc->setSchedulingID( newInc->uid(), oldUid );
            const bool success = calendar->modifyIncidence( newInc );

            if ( !success ) {
              emit transactionFinished( ResultModifyingError, QLatin1String( "Error modifying incidence" ) );
            } else {
              // signal will be emitted in the handleModifyFinished() slot
            }

            return;
          }
        }
      }
      break;
    case ScheduleMessage::Obsolete:
      break;
    default:
      break;
  }

  emit transactionFinished( result, errorString );
}

void Scheduler::acceptRequest( const IncidenceBase::Ptr &incidenceBase,
                               const Akonadi::CalendarBase::Ptr &calendar,
                               ScheduleMessage::Status status,
                               const QString &email )
{
  Incidence::Ptr incidence = incidenceBase.staticCast<Incidence>() ;

  if ( incidence->type() == IncidenceBase::TypeFreeBusy ) {
    // reply to this request is handled in korganizer's incomingdialog
    emit transactionFinished( ResultSuccess, QString() );
    return;
  }

  QString schedulingUid = incidence->uid();
  QString errorString;
  Result result = ResultSuccess;

  const Incidence::List existingIncidences = calendar->incidencesFromSchedulingID( schedulingUid );
  kDebug() << "status=" << KCalUtils::Stringify::scheduleMessageStatus( status ) //krazy:exclude=kdebug
           << ": found " << existingIncidences.count()
           << " incidences with schedulingID " << incidence->schedulingID()
           << "; uid was = " << schedulingUid;

  if ( existingIncidences.isEmpty() ) {
    // Perfectly normal if the incidence doesn't exist. This is probably
    // a new invitation.
    kDebug() << "incidence not found; calendar = " << calendar.data()
             << "; incidence count = " << calendar->incidences().count();
  }

  foreach ( const KCalCore::Incidence::Ptr &existingIncidence, existingIncidences ) {
    kDebug() << "Considering this found event ("
             << ( existingIncidence->isReadOnly() ? "readonly" : "readwrite" )
             << ") :" << mFormat->toString( existingIncidence );
    // If it's readonly, we can't possible update it.
    if ( existingIncidence->isReadOnly() ) {
      continue;
    }

    const QString existingUid = existingIncidence->uid();
    const int existingRevision = existingIncidence->revision();

    if ( existingRevision <= incidence->revision() ) {
      // The new incidence might be an update for the found one
      bool isUpdate = true;
      // Code for new invitations:
      // If you think we could check the value of "status" to be RequestNew:  we can't.
      // It comes from a similar check inside libical, where the event is compared to
      // other events in the calendar. But if we have another version of the event around
      // (e.g. shared folder for a group), the status could be RequestNew, Obsolete or Updated.
      kDebug() << "looking in " << existingUid << "'s attendees";
      // This is supposed to be a new request, not an update - however we want to update
      // the existing one to handle the "clicking more than once on the invitation" case.
      // So check the attendee status of the attendee.
      const Attendee::List attendees = existingIncidence->attendees();
      Attendee::List::ConstIterator ait;
      for ( ait = attendees.begin(); ait != attendees.end(); ++ait ) {
        if( (*ait)->email() == email && (*ait)->status() == Attendee::NeedsAction ) {
          // This incidence wasn't created by me - it's probably in a shared folder
          // and meant for someone else, ignore it.
          kDebug() << "ignoring " << existingUid << " since I'm still NeedsAction there";
          isUpdate = false;
          break;
        }
      }
      if ( isUpdate ) {
        if ( existingRevision == incidence->revision() &&
             existingIncidence->lastModified() > incidence->lastModified() ) {
          // This isn't an update - the found incidence was modified more recently
          errorString = i18n( "This isn't an update. "
                              "The found incidence was modified more recently." );

          kWarning() << errorString
                     << "; revision=" << existingIncidence->revision()
                     << "; existing->lastModified=" << existingIncidence->lastModified()
                     << "; update->lastModified=" << incidence->lastModified();

          emit transactionFinished( ResultOutatedUpdate, errorString );
          return;
        }
        kDebug() << "replacing existing incidence " << existingUid;
        if ( existingIncidence->type() != incidence->type() ) {
          kError() << "assigning different incidence types";
          result = ResultAssigningDifferentTypes;
          errorString = i18n( "Error: Assigning different incidence types." );
          emit transactionFinished( result, errorString );
        } else {
          incidence->setSchedulingID( schedulingUid, existingUid ) ;

          if ( incidence->hasRecurrenceId() ) {
            Incidence::Ptr existingInstance = calendar->incidence(incidence->instanceIdentifier());
            if ( !existingInstance ) {
              // The organizer created an exception, lets create it in our calendar, we don't have it yet
              const bool success = calendar->addIncidence(incidence);

              if ( !success ) {
                emit transactionFinished( ResultCreatingError, QLatin1String( "Error creating incidence" ) );
              } else {
                // Signal emitted in the result slot of addFinished()
              }

              return;
            }
          }


          const bool success = calendar->modifyIncidence( incidence );

          if ( !success ) {
            emit transactionFinished( ResultModifyingError, QLatin1String( "Error modifying incidence" ) );
          } else {
            //handleModifyFinished() will emit the final signal.
          }
        }
        return;
      }
    } else {
      errorString = i18n( "This isn't an update. "
                          "The found incidence was modified more recently." );
      kWarning() << errorString;
      // This isn't an update - the found incidence has a bigger revision number
      kDebug() << "This isn't an update - the found incidence has a bigger revision number";
      emit transactionFinished( ResultOutatedUpdate, errorString );
      return;
    }
  }

  // Move the uid to be the schedulingID and make a unique UID
  incidence->setSchedulingID( schedulingUid, CalFormat::createUniqueId() );
  // notify the user in case this is an update and we didn't find the to-be-updated incidence
  if ( existingIncidences.count() == 0 && incidence->revision() > 0 ) {
    KMessageBox::information(
      0,
      i18nc( "@info",
             "<para>You accepted an invitation update, but an earlier version of the "
             "item could not be found in your calendar.</para>"
             "<para>This may have occurred because:<list>"
             "<item>the organizer did not include you in the original invitation</item>"
             "<item>you did not accept the original invitation yet</item>"
             "<item>you deleted the original invitation from your calendar</item>"
             "<item>you no longer have access to the calendar containing the invitation</item>"
             "</list></para>"
             "<para>This is not a problem, but we thought you should know.</para>" ),
      i18nc( "@title", "Cannot find invitation to be updated" ), "AcceptCantFindIncidence" );
  }
  kDebug() << "Storing new incidence with scheduling uid=" << schedulingUid << " and uid=" << incidence->uid();

  const bool success = calendar->addIncidence( incidence );
  if ( !success ) {
    emit transactionFinished( ResultCreatingError, QLatin1String( "Error adding incidence" ) );
  } else {
    // The slot will emit the result
  }
}

void Scheduler::acceptAdd( const IncidenceBase::Ptr &, ScheduleMessage::Status )
{
  emit transactionFinished( ResultSuccess, QString() );
}

void Scheduler::acceptCancel( const IncidenceBase::Ptr &incidenceBase,
                              const Akonadi::CalendarBase::Ptr &calendar,
                              ScheduleMessage::Status status,
                              const QString &attendeeEmail )
{
  Incidence::Ptr incidence = incidenceBase.staticCast<Incidence>();

  if ( incidence->type() == IncidenceBase::TypeFreeBusy ) {
    // reply to this request is handled in korganizer's incomingdialog
    emit transactionFinished( ResultSuccess, QString() );
    return;
  }

  if ( incidence->type() == IncidenceBase::TypeJournal ) {
    emit transactionFinished( ResultUnsupported, QLatin1String( "Unsupported incidence type") );
    return;
  }

  const Incidence::List existingIncidences = calendar->incidencesFromSchedulingID( incidence->uid() );
  kDebug() << "Scheduler::acceptCancel="
           << KCalUtils::Stringify::scheduleMessageStatus( status ) //krazy2:exclude=kdebug
           << ": found " << existingIncidences.count()
           << " incidences with schedulingID " << incidence->schedulingID();

  Result result = ResultIncidenceToDeleteNotFound;
  QString errorString = i18n( "Could not find incidence to delete." );
  foreach ( const KCalCore::Incidence::Ptr &existingIncidence, existingIncidences ) {
    kDebug() << "Considering this found event ("
             << ( existingIncidence->isReadOnly() ? "readonly" : "readwrite" )
             << ") :" << mFormat->toString( existingIncidence );

    // If it's readonly, we can't possible remove it.
    if ( existingIncidence->isReadOnly() ) {
      continue;
    }

    const QString existingUid = existingIncidence->uid();

    // Code for new invitations:
    // We cannot check the value of "status" to be RequestNew because
    // "status" comes from a similar check inside libical, where the event
    // is compared to other events in the calendar. But if we have another
    // version of the event around (e.g. shared folder for a group), the
    // status could be RequestNew, Obsolete or Updated.
    kDebug() << "looking in " << existingUid << "'s attendees";

    // This is supposed to be a new request, not an update - however we want
    // to update the existing one to handle the "clicking more than once
    // on the invitation" case. So check the attendee status of the attendee.
    bool isMine = true;
    const Attendee::List attendees = existingIncidence->attendees();
    foreach ( const KCalCore::Attendee::Ptr &attendee, attendees ) {
      if ( attendee->email() == attendeeEmail &&
           attendee->status() == Attendee::NeedsAction ) {
        // This incidence wasn't created by me - it's probably in a shared
        // folder and meant for someone else, ignore it.
        kDebug() << "ignoring " << existingUid << " since I'm still NeedsAction there";
        isMine = false;
        break;
      }
    }

    if ( !isMine ) {
      continue;
    }

    kDebug() << "removing existing incidence " << existingUid;

    result = calendar->deleteIncidence( existingIncidence ) ? ResultSuccess : ResultErrorDelete;
    if ( result != ResultSuccess ) {
      emit transactionFinished( result, errorString );
    }

    // The success case will be handled in handleDeleteFinished()
    return;
  }

  // in case we didn't find the to-be-removed incidence
  if ( !existingIncidences.isEmpty() && incidence->revision() > 0 ) {
    KMessageBox::error(
      0,
      i18nc( "@info",
             "The event or task could not be removed from your calendar. "
             "Maybe it has already been deleted or is not owned by you. "
             "Or it might belong to a read-only or disabled calendar." ) );
  }
  emit transactionFinished( result, errorString );
}

void Scheduler::acceptDeclineCounter( const IncidenceBase::Ptr &, ScheduleMessage::Status)
{
  //Not sure why KCalUtils::Scheduler returned false here
  emit transactionFinished( ResultGenericError, i18n( "Generic Error" ) );
}

void Scheduler::acceptReply( const IncidenceBase::Ptr &incidenceBase,
                             const Akonadi::CalendarBase::Ptr &calendar,
                             ScheduleMessage::Status status,
                             iTIPMethod method )
{
  Q_UNUSED( status );
  if ( incidenceBase->type() == IncidenceBase::TypeFreeBusy ) {
    acceptFreeBusy( incidenceBase, method );
    return;
  }

  Result result = ResultGenericError;
  QString errorString = i18n( "Generic Error" );

  Incidence::Ptr incidence = calendar->incidence( incidenceBase->uid() );

  // try harder to find the correct incidence
  if ( !incidence ) {
    const Incidence::List list = calendar->incidences();
    for ( Incidence::List::ConstIterator it=list.constBegin(), end=list.constEnd();
          it != end; ++it ) {
      if ( (*it)->schedulingID() == incidenceBase->uid() ) {
        incidence = ( *it ).dynamicCast<Incidence>();
        break;
      }
    }
  }

  if ( incidence ) {
    //get matching attendee in calendar
    kDebug() << "match found!";
    Attendee::List attendeesIn = incidenceBase->attendees();
    Attendee::List attendeesEv;
    Attendee::List attendeesNew;

    attendeesEv = incidence->attendees();
    Attendee::List::ConstIterator inIt;
    Attendee::List::ConstIterator evIt;
    for ( inIt = attendeesIn.constBegin(); inIt != attendeesIn.constEnd(); ++inIt ) {
      Attendee::Ptr attIn = *inIt;
      bool found = false;
      for ( evIt = attendeesEv.constBegin(); evIt != attendeesEv.constEnd(); ++evIt ) {
        Attendee::Ptr attEv = *evIt;
        if ( attIn->email().toLower() == attEv->email().toLower() ) {
          //update attendee-info
          kDebug() << "update attendee";
          attEv->setStatus( attIn->status() );
          attEv->setDelegate( attIn->delegate() );
          attEv->setDelegator( attIn->delegator() );
          result = ResultSuccess;
          errorString.clear();
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
      Attendee::Ptr attNew = *it;
      QString msg =
        i18nc( "@info", "%1 wants to attend %2 but was not invited.",
               attNew->fullName(), incidence->summary() );
      if ( !attNew->delegator().isEmpty() ) {
        msg = i18nc( "@info", "%1 wants to attend %2 on behalf of %3.",
                     attNew->fullName(), incidence->summary() , attNew->delegator() );
      }
      if ( KMessageBox::questionYesNo(
             0, msg, i18nc( "@title", "Uninvited attendee" ),
             KGuiItem( i18nc( "@option", "Accept Attendance" ) ),
             KGuiItem( i18nc( "@option", "Reject Attendance" ) ) ) != KMessageBox::Yes ) {
        Incidence::Ptr cancel = incidence;
        cancel->addComment( i18nc( "@info",
                                   "The organizer rejected your attendance at this meeting." ) );
        performTransaction( incidenceBase, iTIPCancel, attNew->fullName() );
        continue;
      }

      Attendee::Ptr a( new Attendee( attNew->name(), attNew->email(), attNew->RSVP(),
                                     attNew->status(), attNew->role(), attNew->uid() ) );

      a->setDelegate( attNew->delegate() );
      a->setDelegator( attNew->delegator() );
      incidence->addAttendee( a );

      result = ResultSuccess;
      errorString.clear();
      attendeeAdded = true;
    }

    // send update about new participants
    if ( attendeeAdded ) {
      bool sendMail = false;
      if ( KMessageBox::questionYesNo(
              0,
              i18nc( "@info",
                    "An attendee was added to the incidence. "
                    "Do you want to email the attendees an update message?" ),
              i18nc( "@title", "Attendee Added" ),
              KGuiItem( i18nc( "@option", "Send Messages" ) ),
              KGuiItem( i18nc( "@option", "Do Not Send" ) ) ) == KMessageBox::Yes ) {
        sendMail = true;
      }

      incidence->setRevision( incidence->revision() + 1 );
      if ( sendMail ) {
        performTransaction( incidence, iTIPRequest );
      }
    }

    if ( incidence->type() == Incidence::TypeTodo ) {
      // for VTODO a REPLY can be used to update the completion status of
      // a to-do. see RFC2446 3.4.3
      Todo::Ptr update = incidenceBase.dynamicCast<Todo>();
      Todo::Ptr calendarTodo = incidence.staticCast<Todo>();
      Q_ASSERT( update );
      if ( update && ( calendarTodo->percentComplete() != update->percentComplete() ) ) {
        calendarTodo->setPercentComplete( update->percentComplete() );
        calendarTodo->updated();
        const bool success = calendar->modifyIncidence( calendarTodo );
        if ( !success ) {
          emit transactionFinished( ResultModifyingError, QLatin1String( "Error modifying incidence" ) );
        } else {
          // success will be emitted in the handleModifyFinished() slot
        }
        return;
      }
    }

    if ( result == ResultSuccess ) {
      // We set at least one of the attendees, so the incidence changed
      // Note: This should not result in a sequence number bump
      incidence->updated();
      const bool success = calendar->modifyIncidence( incidence );

      if ( !success ) {
        emit transactionFinished( ResultModifyingError, i18n( "Error modifying incidence" ) );
      } else {
        // success will be emitted in the handleModifyFinished() slot
      }

      return;
    }
  } else {
    result = ResultSuccess;
    errorString = i18n( "No incidence for scheduling." );
    kError() << errorString;
  }
  emit transactionFinished( result, errorString );
}

void Scheduler::acceptRefresh( const IncidenceBase::Ptr &, ScheduleMessage::Status )
{
  // handled in korganizer's IncomingDialog
  // Not sure why it returns false here
  emit transactionFinished( ResultGenericError, i18n( "Generic Error" ) );
}

void Scheduler::acceptCounter( const IncidenceBase::Ptr &, ScheduleMessage::Status )
{
  // Not sure why it returns false here
  emit transactionFinished( ResultGenericError, i18n( "Generic Error" ) );
}

void Scheduler::acceptFreeBusy( const IncidenceBase::Ptr &incidence, iTIPMethod method )
{
  if ( !d->mFreeBusyCache ) {
    kError() << "Scheduler: no FreeBusyCache.";
    emit transactionFinished( ResultNoFreeBusyCache, i18n( "No Free Busy Cache" ) );
    return;
  }

  FreeBusy::Ptr freebusy = incidence.staticCast<FreeBusy>();

  kDebug() << "freeBusyDirName:" << freeBusyDir();

  Person::Ptr from;
  if( method == iTIPPublish ) {
    from = freebusy->organizer();
  }
  if ( ( method == iTIPReply ) && ( freebusy->attendeeCount() == 1 ) ) {
    Attendee::Ptr attendee = freebusy->attendees().first();
    from->setName( attendee->name() );
    from->setEmail( attendee->email() );
  }

  if ( !d->mFreeBusyCache->saveFreeBusy( freebusy, from ) ) {
    emit transactionFinished( ResultErrorSavingFreeBusy, i18n( "Error saving freebusy object" ) );
  } else {
    emit transactionFinished( ResultNoFreeBusyCache, QString() );
  }
}

void Scheduler::handleCreateFinished( bool success, const QString &errorMessage )
{
  emit transactionFinished( success ? ResultSuccess : ResultCreatingError, errorMessage );
}

void Scheduler::handleModifyFinished( bool success, const QString &errorMessage )
{
  kDebug() << "Modification finished. Success=" << success << errorMessage;
  emit transactionFinished( success ? ResultSuccess : ResultModifyingError, errorMessage );
}

void Scheduler::handleDeleteFinished( bool success, const QString &errorMessage )
{
  emit transactionFinished( success ? ResultSuccess : ResultDeletingError, errorMessage );
}

void Scheduler::connectCalendar( const Akonadi::CalendarBase::Ptr &calendar )
{
  connect( calendar.data(), SIGNAL(createFinished(bool,QString)),
           SLOT(handleCreateFinished(bool,QString)), Qt::UniqueConnection );
  connect( calendar.data(), SIGNAL(modifyFinished(bool,QString)),
           SLOT(handleModifyFinished(bool,QString)), Qt::UniqueConnection );
  connect( calendar.data(), SIGNAL(deleteFinished(bool,QString)),
           SLOT(handleDeleteFinished(bool,QString)), Qt::UniqueConnection );
}
