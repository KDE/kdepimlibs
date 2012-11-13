/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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

#include "collectionmovejob.h"
#include "collection.h"
#include "job_p.h"
#include "movejobimpl_p.h"

using namespace Akonadi;

class Akonadi::CollectionMoveJobPrivate : public MoveJobImpl<Collection, CollectionMoveJob>
{
  public:
    CollectionMoveJobPrivate( CollectionMoveJob *parent ) : MoveJobImpl<Collection, CollectionMoveJob>( parent ) {}
    Q_DECLARE_PUBLIC( CollectionMoveJob )
};

CollectionMoveJob::CollectionMoveJob( const Collection &collection, const Collection &destination, QObject * parent )
  : Job( new CollectionMoveJobPrivate( this ), parent )
{
  Q_D( CollectionMoveJob );
  d->destination = destination;
  d->objectsToMove.append( collection );
}

void CollectionMoveJob::doStart()
{
  Q_D( CollectionMoveJob );
  d->sendCommand( "COLMOVE" );
}

