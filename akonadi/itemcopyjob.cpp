/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>

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

#include "itemcopyjob.h"

#include "collection.h"
#include "imapset_p.h"
#include "job_p.h"

using namespace Akonadi;

class Akonadi::ItemCopyJobPrivate : public JobPrivate
{
  public:
    ItemCopyJobPrivate( ItemCopyJob *parent )
      : JobPrivate( parent )
    {
    }

    Item::List mItems;
    Collection mTarget;
};

ItemCopyJob::ItemCopyJob(const Item & item, const Collection & target, QObject * parent)
  : Job( new ItemCopyJobPrivate( this ), parent )
{
  Q_D( ItemCopyJob );

  d->mItems << item;
  d->mTarget = target;
}

ItemCopyJob::ItemCopyJob(const Item::List & items, const Collection & target, QObject * parent)
  : Job( new ItemCopyJobPrivate( this ), parent )
{
  Q_D( ItemCopyJob );

  d->mItems = items;
  d->mTarget = target;
}

ItemCopyJob::~ItemCopyJob()
{
}

void ItemCopyJob::doStart()
{
  Q_D( ItemCopyJob );

  QVector<Item::Id> ids;
  foreach ( const Item &item, d->mItems )
    ids << item.id();
  ImapSet set;
  set.add( ids );
  QByteArray cmd( d->newTag() );
  cmd += " COPY ";
  cmd += set.toImapSequenceSet();
  cmd += ' ';
  cmd += QByteArray::number( d->mTarget.id() );
  cmd += '\n';
  d->writeData( cmd );
}

