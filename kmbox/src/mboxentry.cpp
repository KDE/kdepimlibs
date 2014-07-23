/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#include "mboxentry.h"

#include "mboxentry_p.h"

using namespace KMBox;

MBoxEntry::MBoxEntry()
    : d(new Private)
{
}

MBoxEntry::MBoxEntry(quint64 offset)
    : d(new Private)
{
    d->mOffset = offset;
}

MBoxEntry::MBoxEntry(const MBoxEntry &other)
    : d(other.d)
{
}

MBoxEntry::~MBoxEntry()
{
}

MBoxEntry &MBoxEntry::operator=(const MBoxEntry &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

bool MBoxEntry::operator==(const MBoxEntry &other) const
{
    return (d->mOffset == other.d->mOffset);
}

bool MBoxEntry::operator!=(const MBoxEntry &other) const
{
    return !(other == *this);
}

bool MBoxEntry::isValid() const
{
    return ((d->mOffset != 0) && (d->mMessageSize != 0));
}

quint64 MBoxEntry::messageOffset() const
{
    return d->mOffset;
}

quint64 MBoxEntry::messageSize() const
{
    return d->mMessageSize;
}

quint64 MBoxEntry::separatorSize() const
{
    return d->mSeparatorSize;
}
