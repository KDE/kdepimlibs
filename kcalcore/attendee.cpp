/*
  This file is part of the kcalcore library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include <QDataStream>

using namespace KCalCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCalCore::Attendee::Private
{
public:
    bool mRSVP;
    Role mRole;
    PartStat mStatus;
    QString mUid;
    QString mDelegate;
    QString mDelegator;
    CustomProperties mCustomProperties;
};
//@endcond

Attendee::Attendee(const QString &name, const QString &email, bool rsvp,
                   Attendee::PartStat status, Attendee::Role role, const QString &uid)
    : d(new Attendee::Private)
{
    setName(name);
    setEmail(email);
    d->mRSVP = rsvp;
    d->mStatus = status;
    d->mRole = role;
    d->mUid = uid;
}

Attendee::Attendee(const Attendee &attendee)
    : Person(attendee),
      d(new Attendee::Private(*attendee.d))
{
}

Attendee::~Attendee()
{
    delete d;
}

bool KCalCore::Attendee::operator==(const Attendee &attendee) const
{
    return
        d->mUid == attendee.d->mUid &&
        d->mRSVP == attendee.d->mRSVP &&
        d->mRole == attendee.d->mRole &&
        d->mStatus == attendee.d->mStatus &&
        d->mDelegate == attendee.d->mDelegate &&
        d->mDelegator == attendee.d->mDelegator &&
        (const Person &)*this == (const Person &)attendee;
}

bool KCalCore::Attendee::operator!=(const Attendee &attendee) const
{
    return !operator==(attendee);
}

Attendee &KCalCore::Attendee::operator=(const Attendee &attendee)
{
    // check for self assignment
    if (&attendee == this) {
        return *this;
    }

    *d = *attendee.d;
    setName(attendee.name());
    setEmail(attendee.email());
    return *this;
}

void Attendee::setRSVP(bool r)
{
    d->mRSVP = r;
}

bool Attendee::RSVP() const
{
    return d->mRSVP;
}

void Attendee::setStatus(Attendee::PartStat status)
{
    d->mStatus = status;
}

Attendee::PartStat Attendee::status() const
{
    return d->mStatus;
}

void Attendee::setRole(Attendee::Role role)
{
    d->mRole = role;
}

Attendee::Role Attendee::role() const
{
    return d->mRole;
}

void Attendee::setUid(const QString &uid)
{
    d->mUid = uid;
}

QString Attendee::uid() const
{
    return d->mUid;
}

void Attendee::setDelegate(const QString &delegate)
{
    d->mDelegate = delegate;
}

QString Attendee::delegate() const
{
    return d->mDelegate;
}

void Attendee::setDelegator(const QString &delegator)
{
    d->mDelegator = delegator;
}

QString Attendee::delegator() const
{
    return d->mDelegator;
}

void Attendee::setCustomProperty(const QByteArray &xname, const QString &xvalue)
{
    d->mCustomProperties.setNonKDECustomProperty(xname, xvalue);
}

CustomProperties &Attendee::customProperties()
{
    return d->mCustomProperties;
}

const CustomProperties &Attendee::customProperties() const
{
    return d->mCustomProperties;
}

QDataStream &KCalCore::operator<<(QDataStream &stream, const KCalCore::Attendee::Ptr &attendee)
{
    KCalCore::Person::Ptr p(new KCalCore::Person(*((Person *)attendee.data())));
    stream << p;
    return stream << attendee->d->mRSVP
           << int(attendee->d->mRole)
           << int(attendee->d->mStatus)
           << attendee->d->mUid
           << attendee->d->mDelegate
           << attendee->d->mDelegator
           << attendee->d->mCustomProperties;
}

QDataStream &KCalCore::operator>>(QDataStream &stream, KCalCore::Attendee::Ptr &attendee)
{
    bool RSVP;
    Attendee::Role role;
    Attendee::PartStat status;
    QString uid;
    QString delegate;
    QString delegator;
    CustomProperties customProperties;
    uint role_int;
    uint status_int;

    KCalCore::Person::Ptr person(new Person());
    stream >> person;
    stream >> RSVP
           >> role_int
           >> status_int
           >> uid
           >> delegate
           >> delegator
           >> customProperties;

    role = Attendee::Role(role_int);
    status = Attendee::PartStat(status_int);

    Attendee::Ptr att_temp(new KCalCore::Attendee(person->name(), person->email(),
                           RSVP, status, role, uid));
    att_temp->setDelegate(delegate);
    att_temp->setDelegator(delegator);
    att_temp->d->mCustomProperties = customProperties;
    attendee.swap(att_temp);
    return stream;
}
