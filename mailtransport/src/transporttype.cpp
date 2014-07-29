/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "transporttype.h"
#include "transporttype_p.h"
#include "transport.h"

#include <agentmanager.h>

using namespace MailTransport;

TransportType::TransportType()
    : d(new Private)
{
}

TransportType::TransportType(const TransportType &other)
    : d(other.d)
{
}

TransportType::~TransportType()
{
}

TransportType &TransportType::operator=(const TransportType &other)
{
    if (this != &other) {
        d = other.d;
    }
    return *this;
}

bool TransportType::operator==(const TransportType &other) const
{
    if (d->mType == Transport::EnumType::Akonadi &&
            other.d->mType == Transport::EnumType::Akonadi) {
        return (d->mAgentType == other.d->mAgentType);
    }
    return (d->mType == other.d->mType);
}

bool TransportType::isValid() const
{
    using namespace Akonadi;

    if (d->mType == Transport::EnumType::Akonadi) {
        return d->mAgentType.isValid() &&
               AgentManager::self()->types().contains(d->mAgentType);
    } else {
        return d->mType >= 0;
    }
}

TransportBase::EnumType::type TransportType::type() const
{
    return static_cast<TransportBase::EnumType::type>(d->mType);
}

QString TransportType::name() const
{
    return d->mName;
}

QString TransportType::description() const
{
    return d->mDescription;
}

Akonadi::AgentType TransportType::agentType() const
{
    Q_ASSERT(d->mType == Transport::EnumType::Akonadi);
    return d->mAgentType;
}
