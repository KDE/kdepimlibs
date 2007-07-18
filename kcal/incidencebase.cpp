/*
    This file is part of the kcal library.

    Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
  defines the IncidenceBase class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "incidencebase.h"

#include "calformat.h"

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::IncidenceBase::Private
{
  public:
    Private() : mFloats( true ), mHasDuration( false )
      { mAttendees.setAutoDelete( true ); }

    Private( const Private &other )
      : mLastModified( other.mLastModified ),
        mDtStart( other.mDtStart ),
        mOrganizer( other.mOrganizer ),
        mUid( other.mUid ),
        mDuration( other.mDuration ),
        mFloats( other.mFloats ),
        mHasDuration( other.mHasDuration )
        //????? mComments
        // mObservers: the copied object is a new one, so it isn't observed
        // by the observer of the original object.
      { mAttendees.setAutoDelete( true ); }

    KDateTime mLastModified;     // incidence last modified date
    KDateTime mDtStart;          // incidence start time
    Person mOrganizer;           // incidence person (owner)
    QString mUid;                // incidence unique id
    Duration mDuration;          // incidence duration
    bool mFloats;                // true if the incidence floats
    bool mHasDuration;           // true if the incidence has a duration

    Attendee::List mAttendees;   // list of incidence attendees
    QStringList mComments;       // list of incidence comments
    QList<IncidenceObserver*> mObservers; // list of incidence observers
};
//@endcond

IncidenceBase::IncidenceBase() : d( new KCal::IncidenceBase::Private )
{
  mReadOnly = false;

  setUid( CalFormat::createUniqueId() );
}

IncidenceBase::IncidenceBase( const IncidenceBase &i ) :
  CustomProperties( i ), d( new KCal::IncidenceBase::Private( *i.d ) )
{
  mReadOnly = i.mReadOnly;

  Attendee::List attendees = i.attendees();
  Attendee::List::ConstIterator it;
  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    d->mAttendees.append( new Attendee( *(*it) ) );
  }
}

IncidenceBase::~IncidenceBase()
{
  delete d;
}

bool IncidenceBase::operator==( const IncidenceBase &i2 ) const
{
  if ( attendees().count() != i2.attendees().count() ) {
    return false; // no need to check further
  }

  Attendee::List al1 = attendees();
  Attendee::List al2 = i2.attendees();
  Attendee::List::ConstIterator a1 = al1.begin();
  Attendee::List::ConstIterator a2 = al2.begin();
  for ( ; a1 != al1.end() && a2 != al2.end(); ++a1, ++a2 ) {
    if ( **a1 == **a2 ) {
      continue;
    } else {
      return false;
    }
  }

  if ( !CustomProperties::operator == (i2) ) {
    return false;
  }

  return ( dtStart() == i2.dtStart() &&
           organizer() == i2.organizer() &&
           uid() == i2.uid() &&
           // Don't compare lastModified, otherwise the operator is not
           // of much use. We are not comparing for identity, after all.
           floats() == i2.floats() &&
           duration() == i2.duration() &&
           hasDuration() == i2.hasDuration() );
  // no need to compare mObserver
}

void IncidenceBase::setUid( const QString &uid )
{
  d->mUid = uid;
  updated();
}

QString IncidenceBase::uid() const
{
  return d->mUid;
}

void IncidenceBase::setLastModified( const KDateTime &lm )
{
  // DON'T! updated() because we call this from
  // Calendar::updateEvent().

  // Convert to UTC and remove milliseconds part.
  KDateTime current = lm.toUtc();
  QTime t = current.time();
  t.setHMS( t.hour(), t.minute(), t.second(), 0 );
  current.setTime( t );

  d->mLastModified = current;
}

KDateTime IncidenceBase::lastModified() const
{
  return d->mLastModified;
}

void IncidenceBase::setOrganizer( const Person &o )
{
  // we don't check for readonly here, because it is
  // possible that by setting the organizer we are changing
  // the event's readonly status...
  d->mOrganizer = o;

  updated();
}

void IncidenceBase::setOrganizer( const QString &o )
{
  QString mail( o );
  if ( mail.startsWith( "MAILTO:", Qt::CaseInsensitive ) ) {
    mail = mail.remove( 0, 7 );
  }

  // split the string into full name plus email.
  Person organizer( mail );
  setOrganizer( organizer );
}

Person IncidenceBase::organizer() const
{
  return d->mOrganizer;
}

void IncidenceBase::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
}

void IncidenceBase::setDtStart( const KDateTime &dtStart )
{
//  if ( mReadOnly ) return;
  d->mDtStart = dtStart;
  d->mFloats = dtStart.isDateOnly();
  updated();
}

KDateTime IncidenceBase::dtStart() const
{
  return d->mDtStart;
}

QString IncidenceBase::dtStartTimeStr( bool shortfmt ) const
{
  return KGlobal::locale()->formatTime( dtStart().time(), shortfmt );
}

QString IncidenceBase::dtStartDateStr( bool shortfmt ) const
{
  return KGlobal::locale()->formatDate( dtStart().date(), (shortfmt ? KLocale::ShortDate : KLocale::LongDate) );
}

QString IncidenceBase::dtStartStr( bool shortfmt ) const
{
  return KGlobal::locale()->formatDateTime( dtStart().dateTime(), (shortfmt ? KLocale::ShortDate : KLocale::LongDate) );
}

bool IncidenceBase::floats() const
{
  return d->mFloats;
}

void IncidenceBase::setFloats( bool f )
{
  if ( mReadOnly || f == d->mFloats ) return;
  d->mFloats = f;
  updated();
}

void IncidenceBase::shiftTimes( const KDateTime::Spec &oldSpec,
                                const KDateTime::Spec &newSpec )
{
  d->mDtStart = d->mDtStart.toTimeSpec( oldSpec );
  d->mDtStart.setTimeSpec( newSpec );
  updated();
}

void IncidenceBase::addComment( const QString &comment )
{
  d->mComments += comment;
}

bool IncidenceBase::removeComment( const QString &comment )
{
  bool found = false;
  QStringList::Iterator i;

  for ( i = d->mComments.begin(); !found && i != d->mComments.end(); ++i ) {
    if ( (*i) == comment ) {
      found = true;
      d->mComments.erase( i );
    }
  }

  return found;
}

void IncidenceBase::clearComments()
{
  d->mComments.clear();
}

QStringList IncidenceBase::comments() const
{
  return d->mComments;
}

void IncidenceBase::addAttendee( Attendee *a, bool doupdate )
{
  if ( mReadOnly ) return;
  if ( a->name().left(7).toUpper() == "MAILTO:" ) {
    a->setName( a->name().remove( 0, 7 ) );
  }

  d->mAttendees.append( a );
  if ( doupdate ) {
    updated();
  }
}

const Attendee::List &IncidenceBase::attendees() const
{
  return d->mAttendees;
}

int IncidenceBase::attendeeCount() const
{
  return d->mAttendees.count();
}

void IncidenceBase::clearAttendees()
{
  if ( mReadOnly ) return;
  d->mAttendees.clear();
}

Attendee *IncidenceBase::attendeeByMail( const QString &email ) const
{
  Attendee::List::ConstIterator it;
  for ( it = d->mAttendees.begin(); it != d->mAttendees.end(); ++it ) {
    if ( (*it)->email() == email ) {
      return *it;
    }
  }

  return 0;
}

Attendee *IncidenceBase::attendeeByMails( const QStringList &emails,
                                          const QString &email ) const
{
  QStringList mails = emails;
  if ( !email.isEmpty() ) {
    mails.append( email );
  }

  Attendee::List::ConstIterator itA;
  for ( itA = d->mAttendees.begin(); itA != d->mAttendees.end(); ++itA ) {
    for ( QStringList::Iterator it = mails.begin(); it != mails.end(); ++it ) {
      if ( (*itA)->email() == (*it) ) {
        return *itA;
      }
    }
  }

  return 0;
}

Attendee *IncidenceBase::attendeeByUid( const QString &uid ) const
{
  Attendee::List::ConstIterator it;
  for ( it = d->mAttendees.begin(); it != d->mAttendees.end(); ++it ) {
    if ( (*it)->uid() == uid ) {
      return *it;
    }
  }

  return 0;
}

void IncidenceBase::setDuration( const Duration &duration )
{
  d->mDuration = duration;
  setHasDuration( true );
  updated();
}

Duration IncidenceBase::duration() const
{
  return d->mDuration;
}

void IncidenceBase::setHasDuration( bool hasDuration )
{
  d->mHasDuration = hasDuration;
}

bool IncidenceBase::hasDuration() const
{
  return d->mHasDuration;
}

void IncidenceBase::registerObserver( IncidenceBase::IncidenceObserver *observer )
{
  if ( !d->mObservers.contains( observer ) ) {
    d->mObservers.append( observer );
  }
  //updatedSilent(); FIXME:CRASH
}

void IncidenceBase::unRegisterObserver( IncidenceBase::IncidenceObserver *observer )
{
  d->mObservers.removeAll( observer );
}

void IncidenceBase::updated()
{
  foreach ( IncidenceObserver *o, d->mObservers ) {
    o->incidenceUpdated( this );
  }
}

void IncidenceBase::customPropertyUpdated()
{
  updated();
}

KUrl IncidenceBase::uri() const
{
  return KUrl( QString( "urn:x-ical:" ) + uid() );
}

void IncidenceBase::updatedSilent()
{
  foreach ( IncidenceObserver *o, d->mObservers ) {
    o->incidenceUpdatedSilent( this );
  }
}

