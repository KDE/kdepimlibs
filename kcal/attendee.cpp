/*
  This file is part of the kcal library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
  defines the Attendee class.

  @brief
  Represents information related to an attendee of an Calendar Incidence.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "attendee.h"

#include <kdebug.h>
#include <klocale.h>

#include <QtCore/QStringList>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::Attendee::Private
{
  public:
    bool mRSVP;
    Role mRole;
    PartStat mStatus;
    QString mUid;
    QString mDelegate;
    QString mDelegator;
};
//@endcond

Attendee::Attendee( const QString &name, const QString &email, bool rsvp,
                    Attendee::PartStat status, Attendee::Role role, const QString &uid )
  : d( new Attendee::Private )
{
  setName( name );
  setEmail( email );
  d->mRSVP = rsvp;
  d->mStatus = status;
  d->mRole = role;
  d->mUid = uid;
}

Attendee::Attendee( const Attendee &attendee )
  : Person( attendee ),
    d( new Attendee::Private( *attendee.d ) )
{
}

Attendee::~Attendee()
{
  delete d;
}

bool KCal::Attendee::operator==( const Attendee &attendee )
{
  return
    ( Person & )*this == ( const Person & )attendee &&
    d->mRSVP == attendee.d->mRSVP &&
    d->mRole == attendee.d->mRole &&
    d->mStatus == attendee.d->mStatus &&
    d->mUid == attendee.d->mUid &&
    d->mDelegate == attendee.d->mDelegate &&
    d->mDelegator == attendee.d->mDelegator;
}

Attendee &KCal::Attendee::operator=( const Attendee &attendee )
{
  // check for self assignment
  if ( &attendee == this ) {
    return *this;
  }

  *d = *attendee.d;
  setName( attendee.name() );
  setEmail( attendee.email() );
  return *this;
}

void Attendee::setRSVP( bool r )
{
  d->mRSVP = r;
}

bool Attendee::RSVP() const
{
  return d->mRSVP;
}

void Attendee::setStatus( Attendee::PartStat status )
{
  d->mStatus = status;
}

Attendee::PartStat Attendee::status() const
{
  return d->mStatus;
}

QString Attendee::statusStr() const
{
  return statusName( d->mStatus );
}

QString Attendee::statusName( Attendee::PartStat status )
{
  switch ( status ) {
  default:
  case NeedsAction:
    return i18nc( "@item event, to-do or journal needs action", "Needs Action" );
    break;
  case Accepted:
    return i18nc( "@item event, to-do or journal accepted", "Accepted" );
    break;
  case Declined:
    return i18nc( "@item event, to-do or journal declined", "Declined" );
    break;
  case Tentative:
    return i18nc( "@item event or to-do tentatively accepted", "Tentative" );
    break;
  case Delegated:
    return i18nc( "@item event or to-do delegated", "Delegated" );
    break;
  case Completed:
    return i18nc( "@item to-do completed", "Completed" );
    break;
  case InProcess:
    return i18nc( "@item to-do in process of being completed", "In Process" );
    break;
  }
}

QStringList Attendee::statusList()
{
  QStringList list;
  list << statusName( NeedsAction );
  list << statusName( Accepted );
  list << statusName( Declined );
  list << statusName( Tentative );
  list << statusName( Delegated );
  list << statusName( Completed );
  list << statusName( InProcess );

  return list;
}

void Attendee::setRole( Attendee::Role role )
{
  d->mRole = role;
}

Attendee::Role Attendee::role() const
{
  return d->mRole;
}

QString Attendee::roleStr() const
{
  return roleName( d->mRole );
}

void Attendee::setUid( const QString &uid )
{
  d->mUid = uid;
}

QString Attendee::uid() const
{
  return d->mUid;
}

QString Attendee::roleName( Attendee::Role role )
{
  switch ( role ) {
  case Chair:
    return i18nc( "@item chairperson", "Chair" );
    break;
  default:
  case ReqParticipant:
    return i18nc( "@item participation is required", "Participant" );
    break;
  case OptParticipant:
    return i18nc( "@item participation is optional", "Optional Participant" );
    break;
  case NonParticipant:
    return i18nc( "@item non-participant copied for information", "Observer" );
    break;
  }
}

QStringList Attendee::roleList()
{
  QStringList list;
  list << roleName( ReqParticipant );
  list << roleName( OptParticipant );
  list << roleName( NonParticipant );
  list << roleName( Chair );

  return list;
}

void Attendee::setDelegate( const QString &delegate )
{
  d->mDelegate = delegate;
}

QString Attendee::delegate() const
{
  return d->mDelegate;
}

void Attendee::setDelegator( const QString &delegator )
{
  d->mDelegator = delegator;
}

QString Attendee::delegator() const
{
  return d->mDelegator;
}
