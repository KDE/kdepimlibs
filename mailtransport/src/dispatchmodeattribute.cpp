/*
    Copyright 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "dispatchmodeattribute.h"

#include <QDebug>

#include <attributefactory.h>

using namespace Akonadi;
using namespace MailTransport;

class DispatchModeAttribute::Private
{
public:
    DispatchMode mMode;
    QDateTime mDueDate;
};

DispatchModeAttribute::DispatchModeAttribute(DispatchMode mode)
    : d(new Private)
{
    d->mMode = mode;
}

DispatchModeAttribute::~DispatchModeAttribute()
{
    delete d;
}

DispatchModeAttribute *DispatchModeAttribute::clone() const
{
    DispatchModeAttribute *const cloned = new DispatchModeAttribute(d->mMode);
    cloned->setSendAfter(d->mDueDate);
    return cloned;
}

QByteArray DispatchModeAttribute::type() const
{
    static const QByteArray sType("DispatchModeAttribute");
    return sType;
}

QByteArray DispatchModeAttribute::serialized() const
{
    switch (d->mMode) {
    case Automatic: {
        if (!d->mDueDate.isValid()) {
            return "immediately";
        } else {
            return "after" + d->mDueDate.toString(Qt::ISODate).toLatin1();
        }
    }
    case Manual: return "never";
    }

    Q_ASSERT(false);
    return QByteArray(); // suppress control-reaches-end-of-non-void-function warning
}

void DispatchModeAttribute::deserialize(const QByteArray &data)
{
    d->mDueDate = QDateTime();
    if (data == "immediately") {
        d->mMode = Automatic;
    } else if (data == "never") {
        d->mMode = Manual;
    } else if (data.startsWith(QByteArray("after"))) {
        d->mMode = Automatic;
        d->mDueDate = QDateTime::fromString(QString::fromLatin1(data.mid(5)), Qt::ISODate);
        // NOTE: 5 is the strlen of "after".
    } else {
        qWarning() << "Failed to deserialize data [" << data << "]";
    }
}

DispatchModeAttribute::DispatchMode DispatchModeAttribute::dispatchMode() const
{
    return d->mMode;
}

void DispatchModeAttribute::setDispatchMode(DispatchMode mode)
{
    d->mMode = mode;
}

QDateTime DispatchModeAttribute::sendAfter() const
{
    return d->mDueDate;
}

void DispatchModeAttribute::setSendAfter(const QDateTime &date)
{
    d->mDueDate = date;
}

