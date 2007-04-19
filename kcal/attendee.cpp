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

  @author Cornelius Schumacher
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
};
//@endcond

Attendee::Attendee( const QString &name, const QString &email, bool rsvp,
                    Attendee::PartStat s, Attendee::Role r, const QString &u )
  : d( new KCal::Attendee::Private )
{
  setName( name );
  setEmail( email );
  d->mRSVP = rsvp;
  d->mStatus = s;
  d->mRole = r;
  d->mUid = u;
}

Attendee::~Attendee()
{
  delete d;
}

bool KCal::Attendee::operator==( const Attendee &attendee )
{
  return (
    (Person &)*this == (const Person &)attendee &&
    d->mRSVP == attendee.d->mRSVP &&
    d->mRole == attendee.d->mRole &&
    d->mStatus == attendee.d->mStatus &&
    d->mUid == attendee.d->mUid
    );
}

void Attendee::setRSVP( bool r )
{
  d->mRSVP = r;
}

bool Attendee::RSVP() const
{
  return d->mRSVP;
}

void Attendee::setStatus( Attendee::PartStat s )
{
  d->mStatus = s;
}

Attendee::PartStat Attendee::status() const
{
  return d->mStatus;
}

QString Attendee::statusStr() const
{
  return statusName( d->mStatus );
}

QString Attendee::statusName( Attendee::PartStat s )
{
  switch ( s ) {
  default:
  case NeedsAction:
    return i18n( "Needs Action" );
    break;
  case Accepted:
    return i18n( "Accepted" );
    break;
  case Declined:
    return i18n( "Declined" );
    break;
  case Tentative:
    return i18nc( "attendee status", "Tentative" );
    break;
  case Delegated:
    return i18n( "Delegated" );
    break;
  case Completed:
    return i18n( "Completed" );
    break;
  case InProcess:
    return i18n( "In Process" );
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

void Attendee::setRole( Attendee::Role r )
{
  d->mRole = r;
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

QString Attendee::roleName( Attendee::Role r )
{
  switch ( r ) {
  case Chair:
    return i18n( "Chair" );
    break;
  default:
  case ReqParticipant:
    return i18n( "Participant" );
    break;
  case OptParticipant:
    return i18n( "Optional Participant" );
    break;
  case NonParticipant:
    return i18n( "Observer" );
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
