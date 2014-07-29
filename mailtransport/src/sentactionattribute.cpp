/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB,
      a KDAB Group company, info@kdab.net,
      author Tobias Koenig <tokoe@kdab.com>

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

#include "sentactionattribute.h"

#include <QtCore/QDataStream>
#include <QtCore/QSharedData>

using namespace Akonadi;
using namespace MailTransport;

class SentActionAttribute::Action::Private : public QSharedData
{
public:
    Private()
        : mType(Invalid)
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mType = other.mType;
        mValue = other.mValue;
    }

    Type mType;
    QVariant mValue;
};

SentActionAttribute::Action::Action()
    : d(new Private)
{
}

SentActionAttribute::Action::Action(Type type, const QVariant &value)
    : d(new Private)
{
    d->mType = type;
    d->mValue = value;
}

SentActionAttribute::Action::Action(const Action &other)
    : d(other.d)
{
}

SentActionAttribute::Action::~Action()
{
}

SentActionAttribute::Action::Type SentActionAttribute::Action::type() const
{
    return d->mType;
}

QVariant SentActionAttribute::Action::value() const
{
    return d->mValue;
}

SentActionAttribute::Action &SentActionAttribute::Action::operator=(const Action &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

bool SentActionAttribute::Action::operator==(const Action &other) const
{
    return ((d->mType == other.d->mType) && (d->mValue == other.d->mValue));
}

class SentActionAttribute::Private
{
public:
    Action::List mActions;
};

SentActionAttribute::SentActionAttribute()
    : d(new Private)
{
}

SentActionAttribute::~SentActionAttribute()
{
    delete d;
}

void SentActionAttribute::addAction(Action::Type type, const QVariant &value)
{
    d->mActions.append(Action(type, value));
}

SentActionAttribute::Action::List SentActionAttribute::actions() const
{
    return d->mActions;
}

SentActionAttribute *SentActionAttribute::clone() const
{
    SentActionAttribute *attribute = new SentActionAttribute;
    attribute->d->mActions = d->mActions;

    return attribute;
}

QByteArray SentActionAttribute::type() const
{
    static const QByteArray sType("SentActionAttribute");
    return sType;
}

QByteArray SentActionAttribute::serialized() const
{
    QVariantList list;
    foreach (const Action &action, d->mActions) {
        QVariantMap map;
        map.insert(QString::number(action.type()), action.value());

        list << QVariant(map);
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_6);
    stream << list;

    return data;
}

void SentActionAttribute::deserialize(const QByteArray &data)
{
    d->mActions.clear();

    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_4_6);

    QVariantList list;
    stream >> list;

    foreach (const QVariant &variant, list) {
        const QVariantMap map = variant.toMap();
        QMapIterator<QString, QVariant> it(map);
        while (it.hasNext()) {
            it.next();
            d->mActions << Action(static_cast<Action::Type>(it.key().toInt()), it.value());
        }
    }
}
