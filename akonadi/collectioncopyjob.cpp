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

#include "collectioncopyjob.h"

#include "collection.h"
#include "job_p.h"

using namespace Akonadi;

class Akonadi::CollectionCopyJobPrivate : public JobPrivate
{
  public:
    CollectionCopyJobPrivate( CollectionCopyJob *parent )
      : JobPrivate( parent )
    {
    }

    Collection mSource;
    Collection mTarget;
};

CollectionCopyJob::CollectionCopyJob(const Collection & source, const Collection & target, QObject * parent)
  : Job( new CollectionCopyJobPrivate( this ), parent )
{
  Q_D( CollectionCopyJob );

  d->mSource = source;
  d->mTarget = target;
}

CollectionCopyJob::~CollectionCopyJob()
{
}

void CollectionCopyJob::doStart()
{
  Q_D( CollectionCopyJob );

  QByteArray command = d->newTag();
  command += " COLCOPY ";
  command += QByteArray::number( d->mSource.id() );
  command += ' ';
  command += QByteArray::number( d->mTarget.id() );
  command += '\n';
  d->writeData( command );
}

