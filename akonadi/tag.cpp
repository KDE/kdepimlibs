/*
    Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include "tag.h"
#include <akonadi/entitydisplayattribute.h>

using namespace Akonadi;

struct Akonadi::Tag::Private {
    Private()
        :id(-1)
    {}

    Id id;
    QByteArray gid;
    QByteArray remoteId;
    QScopedPointer<Tag> parent;
    QByteArray type;
};

Tag::Tag()
    :AttributeEntity(),
    d(new Private)
{

}

Tag::Tag(Tag::Id id)
    :AttributeEntity(),
    d(new Private)
{
    d->id = id;
}

Tag::Tag(const QByteArray &gid, const QByteArray &type, const QString &name, const Tag &parent)
    :AttributeEntity(),
    d(new Private)
{
    d->gid = gid;
    d->type = type;
    setName(name);
    setParent(parent);
}

Tag::Tag(const Tag &other)
    :d(new Private)
{
    operator=(other);
}

Tag& Tag::operator=(const Tag &other)
{
    d->id = other.d->id;
    d->gid = other.d->gid;
    d->remoteId = other.d->remoteId;
    d->type = other.d->type;
    if (other.d->parent) {
        d->parent.reset(new Tag(*other.d->parent));
    }
    return *this;
}

bool Tag::operator==(const Tag &other)
{
    if (isValid() && other.isValid()) {
        return d->id == other.d->id;
    }
    return d->gid == other.d->gid;
}

void Tag::setId(Tag::Id identifier)
{
    d->id = identifier;
}

Tag::Id Tag::id() const
{
    return d->id;
}

void Tag::setGid(const QByteArray &gid) const
{
    d->gid = gid;
}

QByteArray Tag::gid() const
{
    return d->gid;
}

void Tag::setRemoteId(const QByteArray &remoteId) const
{
    d->remoteId = remoteId;
}

QByteArray Tag::remoteId() const
{
    return d->remoteId;
}

void Tag::setName(const QString &name)
{
    if (!name.isEmpty()) {
        EntityDisplayAttribute* const attr = attribute<EntityDisplayAttribute>(Akonadi::AttributeEntity::AddIfMissing);
        attr->setDisplayName(name);
    }
}

QString Tag::name() const
{
    const EntityDisplayAttribute* const attr = attribute<EntityDisplayAttribute>();
    const QString displayName = attr ? attr->displayName() : QString();
    return !displayName.isEmpty() ? displayName : QString::fromLatin1(d->gid);
}

void Tag::setParent(const Tag &parent)
{
    if (parent.isValid()) {
        d->parent.reset(new Tag(parent));
    }
}

Tag Tag::parent() const
{
    if (!d->parent) {
        return Tag();
    }
    return *d->parent;
}

bool Tag::isValid() const
{
    return d->id >= 0;
}