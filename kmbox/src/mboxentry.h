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

#ifndef KMBOX_MBOXENTRY_H
#define KMBOX_MBOXENTRY_H

#include "kmbox_export.h"

#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QPair>
#include <QtCore/QSharedDataPointer>

namespace KMBox
{

/**
 * @short A class that encapsulates an entry of a MBox.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 * @since 4.6
 */
class KMBOX_EXPORT MBoxEntry
{
public:
    /**
     * Describes a list of mbox entry objects.
     */
    typedef QList<MBoxEntry> List;

    /**
     * Describes a pair of mbox entry objects.
     */
    typedef QPair<MBoxEntry, MBoxEntry> Pair;

    /**
     * Creates an invalid mbox entry object.
     */
    MBoxEntry();

    /**
     * Creates an mbox entry object.
     *
     * @param offset The offset of the message the object references.
     */
    explicit MBoxEntry(quint64 offset);

    /**
     * Creates an mbox entry object from an @p other object.
     */
    MBoxEntry(const MBoxEntry &other);

    /**
     * Destroys the mbox entry object.
     */
    ~MBoxEntry();

    /**
     * Replaces this mbox entry object with an @p other object.
     */
    MBoxEntry &operator=(const MBoxEntry &other);

    /**
     * Returns whether this mbox entry object is equal to an @p other.
     */
    bool operator==(const MBoxEntry &other) const;

    /**
     * Returns whether this mbox entry object is not equal to an @p other.
     */
    bool operator!=(const MBoxEntry &other) const;

    /**
     * Returns whether this is a valid mbox entry object.
     */
    bool isValid() const;

    /**
     * Returns the offset of the message that is referenced by this
     * mbox entry object.
     */
    quint64 messageOffset() const;

    /**
     * Returns the size of the message that is referenced by this
     * mbox entry object.
     */
    quint64 messageSize() const;

    /**
     * Returns the separator size of the message that is referenced by this
     * mbox entry object.
     */
    quint64 separatorSize() const;

private:
    //@cond PRIVATE
    friend class MBox;

    class Private;
    QSharedDataPointer<Private> d;
    //@endcond
};

}

Q_DECLARE_TYPEINFO(KMBox::MBoxEntry, Q_MOVABLE_TYPE);

#endif // KMBOX_MBOXENTRY_H
