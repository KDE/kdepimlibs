/*
  This file is part of the kcalcore library.

  Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  Contact: Alvaro Manera <alvaro.manera@nokia.com>

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

  @brief
  An abstract base class that provides a common base for all calendar incidence
  classes.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "incidencebase.h"
#include "calformat.h"
#include "visitor.h"

#include <kurl.h>

#include <QtCore/QStringList>

using namespace KCalCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCalCore::IncidenceBase::Private
{
  public:
    Private()
      : mOrganizer( new Person() ),
        mUpdateGroupLevel( 0 ),
        mUpdatedPending( false ),
        mAllDay( true ),
        mHasDuration( false )
    {}

    Private( const Private &other )
      : mUpdateGroupLevel( 0 ),
        mUpdatedPending( false ),
        mAllDay( true ),
        mHasDuration( false )
    {
      init( other );
    }

    void init( const Private &other );

    KDateTime mLastModified;     // incidence last modified date
    KDateTime mDtStart;          // incidence start time
    Person::Ptr mOrganizer;           // incidence person (owner)
    QString mUid;                // incidence unique id
    Duration mDuration;          // incidence duration
    int mUpdateGroupLevel;       // if non-zero, suppresses update() calls
    bool mUpdatedPending;        // true if an update has occurred since startUpdates()
    bool mAllDay;                // true if the incidence is all-day
    bool mHasDuration;           // true if the incidence has a duration
    Attendee::List mAttendees;   // list of incidence attendees
    QStringList mComments;       // list of incidence comments
    QStringList mContacts;       // list of incidence contacts
    QList<IncidenceObserver*> mObservers; // list of incidence observers
};

void IncidenceBase::Private::init( const Private &other )
{
  mLastModified = other.mLastModified;
  mDtStart = other.mDtStart;
  mOrganizer = other.mOrganizer;
  mUid = other.mUid;
  mDuration = other.mDuration;
  mAllDay = other.mAllDay;
  mHasDuration = other.mHasDuration;

  mComments = other.mComments;
  mContacts = other.mContacts;

  mAttendees.clear();
  Attendee::List::ConstIterator it;
  for ( it = other.mAttendees.constBegin(); it != other.mAttendees.constEnd(); ++it ) {
    mAttendees.append( Attendee::Ptr( new Attendee( *(*it) ) ) );
  }
}
//@endcond

IncidenceBase::IncidenceBase()
 : d( new KCalCore::IncidenceBase::Private )
{
  mReadOnly = false;

  setUid( CalFormat::createUniqueId() );
}

IncidenceBase::IncidenceBase( const IncidenceBase &i )
 : CustomProperties( i ),
   d( new KCalCore::IncidenceBase::Private( *i.d ) )
{
  mReadOnly = i.mReadOnly;
}

IncidenceBase::~IncidenceBase()
{
  delete d;
}

IncidenceBase &IncidenceBase::operator=( const IncidenceBase &other )
{
  Q_ASSERT( type() == other.type() );

  startUpdates();

  // assign is virtual, will call the derived class's
  IncidenceBase &ret = assign( other );
  endUpdates();
  return ret;
}

IncidenceBase &IncidenceBase::assign( const IncidenceBase &other )
{
  CustomProperties::operator=( other );
  d->init( *other.d );
  mReadOnly = other.mReadOnly;
  return *this;
}

bool IncidenceBase::operator==( const IncidenceBase &i2 ) const
{
  if ( i2.type() != type() ) {
    return false;
  } else {
    // equals is virtual, so here we're calling the derived class method
    return equals( i2 );
  }
}

bool IncidenceBase::operator!=( const IncidenceBase &i2 ) const
{
  return !operator==( i2 );
}

bool IncidenceBase::equals( const IncidenceBase &i2 ) const
{
  if ( attendees().count() != i2.attendees().count() ) {
    return false;
  }

  Attendee::List al1 = attendees();
  Attendee::List al2 = i2.attendees();
  Attendee::List::ConstIterator a1 = al1.constBegin();
  Attendee::List::ConstIterator a2 = al2.constBegin();
  //TODO Does the order of attendees in the list really matter?
  //Please delete this comment if you know it's ok, kthx
  for ( ; a1 != al1.constEnd() && a2 != al2.constEnd(); ++a1, ++a2 ) {
    if ( !( **a1 == **a2 ) ) {
      return false;
    }
  }

  if ( !CustomProperties::operator==( i2 ) ) {
    return false;
  }

  return
    ( ( dtStart() == i2.dtStart() ) ||
      ( !dtStart().isValid() && !i2.dtStart().isValid() ) ) &&
    organizer() == i2.organizer() &&
    uid() == i2.uid() &&
    // Don't compare lastModified, otherwise the operator is not
    // of much use. We are not comparing for identity, after all.
    allDay() == i2.allDay() &&
    duration() == i2.duration() &&
    hasDuration() == i2.hasDuration();
    // no need to compare mObserver
}

bool IncidenceBase::accept( Visitor &v, IncidenceBase::Ptr incidence )
{
  Q_UNUSED( v );
  Q_UNUSED( incidence );
  return false;
}

void IncidenceBase::setUid( const QString &uid )
{
  update();
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

void IncidenceBase::setOrganizer( const Person::Ptr &o )
{
  update();
  // we don't check for readonly here, because it is
  // possible that by setting the organizer we are changing
  // the event's readonly status...
  d->mOrganizer = o;

  updated();
}

void IncidenceBase::setOrganizer( const QString &o )
{
  QString mail( o );
  if ( mail.startsWith( QLatin1String( "MAILTO:" ), Qt::CaseInsensitive ) ) {
    mail = mail.remove( 0, 7 );
  }

  // split the string into full name plus email.
  const Person::Ptr organizer = Person::fromFullName( mail );
  setOrganizer( organizer );
}

Person::Ptr IncidenceBase::organizer() const
{
  return d->mOrganizer;
}

void IncidenceBase::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
}

bool IncidenceBase::isReadOnly() const
{
  return mReadOnly;
}

void IncidenceBase::setDtStart( const KDateTime &dtStart )
{
//  if ( mReadOnly ) return;
  update();
  d->mDtStart = dtStart;
  d->mAllDay = dtStart.isDateOnly();
  updated();
}

KDateTime IncidenceBase::dtStart() const
{
  return d->mDtStart;
}

bool IncidenceBase::allDay() const
{
  return d->mAllDay;
}

void IncidenceBase::setAllDay( bool f )
{
  if ( mReadOnly || f == d->mAllDay ) {
    return;
  }
  update();
  d->mAllDay = f;
  updated();
}

void IncidenceBase::shiftTimes( const KDateTime::Spec &oldSpec,
                                const KDateTime::Spec &newSpec )
{
  update();
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

void IncidenceBase::addContact( const QString &contact )
{
  if ( !contact.isEmpty() ) {
    d->mContacts += contact;
  }
}

bool IncidenceBase::removeContact( const QString &contact )
{
  bool found = false;
  QStringList::Iterator i;

  for ( i = d->mContacts.begin(); !found && i != d->mContacts.end(); ++i ) {
    if ( (*i) == contact ) {
      found = true;
      d->mContacts.erase( i );
    }
  }
  return found;
}

void IncidenceBase::clearContacts()
{
  d->mContacts.clear();
}

QStringList IncidenceBase::contacts() const
{
  return d->mContacts;
}

void IncidenceBase::addAttendee( const Attendee::Ptr &a, bool doupdate )
{
  if ( !a || mReadOnly ) {
    return;
  }

  if ( doupdate ) {
    update();
  }
  if ( a->name().left(7).toUpper() == "MAILTO:" ) {
    a->setName( a->name().remove( 0, 7 ) );
  }

  /* If Uid is empty, just use the pointer to Attendee (encoded to
   * string) as Uid. Only thing that matters is that the Uid is unique
   * insofar IncidenceBase is concerned, and this does that (albeit
   * not very nicely). If these are ever saved to disk, should use
   * (considerably more expensive) CalFormat::createUniqueId(). As Uid
   * is not part of Attendee in iCal std, it's fairly safe bet that
   * these will never hit disc though so faster generation speed is
   * more important than actually being forever unique.*/
  if ( a->uid().isEmpty() ) {
    a->setUid( QString::number( (qlonglong)a.data() ) );
  }

  d->mAttendees.append( a );
  if ( doupdate ) {
    updated();
  }
}

void IncidenceBase::deleteAttendee( const Attendee::Ptr &a, bool doupdate )
{
  if ( !a || mReadOnly ) {
    return;
  }

  if ( doupdate ) {
    update();
  }

  d->mAttendees.removeOne( a );

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
  if ( mReadOnly ) {
    return;
  }
  d->mAttendees.clear();
}

Attendee::Ptr IncidenceBase::attendeeByMail( const QString &email ) const
{
  Attendee::List::ConstIterator it;
  for ( it = d->mAttendees.constBegin(); it != d->mAttendees.constEnd(); ++it ) {
    if ( (*it)->email() == email ) {
      return *it;
    }
  }

  return Attendee::Ptr();
}

Attendee::Ptr IncidenceBase::attendeeByMails( const QStringList &emails,
                                          const QString &email ) const
{
  QStringList mails = emails;
  if ( !email.isEmpty() ) {
    mails.append( email );
  }

  Attendee::List::ConstIterator itA;
  for ( itA = d->mAttendees.constBegin(); itA != d->mAttendees.constEnd(); ++itA ) {
    for ( QStringList::const_iterator it = mails.constBegin(); it != mails.constEnd(); ++it ) {
      if ( (*itA)->email() == (*it) ) {
        return *itA;
      }
    }
  }

  return Attendee::Ptr();
}

Attendee::Ptr IncidenceBase::attendeeByUid( const QString &uid ) const
{
  Attendee::List::ConstIterator it;
  for ( it = d->mAttendees.constBegin(); it != d->mAttendees.constEnd(); ++it ) {
    if ( (*it)->uid() == uid ) {
      return *it;
    }
  }

  return Attendee::Ptr();
}

void IncidenceBase::setDuration( const Duration &duration )
{
  update();
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
}

void IncidenceBase::unRegisterObserver( IncidenceBase::IncidenceObserver *observer )
{
  d->mObservers.removeAll( observer );
}

void IncidenceBase::update()
{
  if ( !d->mUpdateGroupLevel ) {
    d->mUpdatedPending = true;
    foreach ( IncidenceObserver *o, d->mObservers ) {
      o->incidenceUpdate( uid() );
    }
  }
}

void IncidenceBase::updated()
{
  if ( d->mUpdateGroupLevel ) {
    d->mUpdatedPending = true;
  } else {
    foreach ( IncidenceObserver *o, d->mObservers ) {
      o->incidenceUpdated( uid() );
    }
  }
}

void IncidenceBase::startUpdates()
{
  update();
  ++d->mUpdateGroupLevel;
}

void IncidenceBase::endUpdates()
{
  if ( d->mUpdateGroupLevel > 0 ) {
    if ( --d->mUpdateGroupLevel == 0 && d->mUpdatedPending ) {
      d->mUpdatedPending = false;
      updated();
    }
  }
}

void IncidenceBase::customPropertyUpdate()
{
  update();
}

void IncidenceBase::customPropertyUpdated()
{
  updated();
}

KUrl IncidenceBase::uri() const
{
  return KUrl( QString( "urn:x-ical:" ) + uid() );
}

IncidenceBase::IncidenceObserver::~IncidenceObserver()
{
}
