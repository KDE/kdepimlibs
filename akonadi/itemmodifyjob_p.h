/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_ITEMMODIFYJOB_P_H
#define AKONADI_ITEMMODIFYJOB_P_H

#include "job_p.h"

namespace Akonadi {

/**
 * @internal
 */
class ItemModifyJobPrivate : public JobPrivate
{
  public:
    enum Operation
    {
      RemoteId,
      RemoteRevision,
      Dirty
    };

    ItemModifyJobPrivate( ItemModifyJob *parent );

    void setClean();
    QByteArray nextPartHeader();

    void conflictResolved();
    void conflictResolveError( const QString& );

    void doUpdateItemRevision( Entity::Id, int oldRevision, int newRevision );

    Q_DECLARE_PUBLIC( ItemModifyJob )

    QSet<int> mOperations;
    QByteArray mTag;
    Item::List mItems;
    bool mRevCheck;
    QSet<QByteArray> mParts;
    QByteArray mPendingData;
    bool mIgnorePayload;
    bool mAutomaticConflictHandlingEnabled;
};

}

#endif
