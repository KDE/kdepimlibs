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

#include "sentbehaviourattribute.h"

#include <QDataStream>

#include <KDebug>

using namespace Akonadi;
using namespace OutboxInterface;

SentBehaviourAttribute::SentBehaviourAttribute( SentBehaviour beh, Collection::Id moveToCollection )
  : mBehaviour( beh )
  , mMoveToCollection( moveToCollection )
{
}

SentBehaviourAttribute::~SentBehaviourAttribute()
{
}

SentBehaviourAttribute* SentBehaviourAttribute::clone() const
{
  return new SentBehaviourAttribute( mBehaviour, mMoveToCollection );
}

QByteArray SentBehaviourAttribute::type() const
{
  static const QByteArray sType( "SentBehaviourAttribute" );
  return sType;
}

QByteArray SentBehaviourAttribute::serialized() const
{
  switch( mBehaviour ) {
    case Delete: return "delete";
    case MoveToCollection: return "moveTo" + QByteArray::number( mMoveToCollection );
    case MoveToDefaultSentCollection: return "moveToDefault";
  }

  Q_ASSERT( false );
  return QByteArray();
}

void SentBehaviourAttribute::deserialize( const QByteArray &data )
{
  mMoveToCollection = -1;
  if ( data == "delete" ) {
    mBehaviour = Delete;
  } else if ( data == "moveToDefault" ) {
    mBehaviour = MoveToDefaultSentCollection;
  } else if ( data.startsWith( QByteArray( "moveTo" ) ) ) {
    mBehaviour = MoveToCollection;
    mMoveToCollection = data.mid(6).toLongLong();
    // NOTE: 6 is the strlen of "moveTo".
  } else {
    Q_ASSERT( false );
  }
}

SentBehaviourAttribute::SentBehaviour SentBehaviourAttribute::sentBehaviour() const
{
  return mBehaviour;
}

void SentBehaviourAttribute::setSentBehaviour( SentBehaviour beh )
{
  mBehaviour = beh;
}

Collection::Id SentBehaviourAttribute::moveToCollection() const
{
  return mMoveToCollection;
}

void SentBehaviourAttribute::setMoveToCollection( Collection::Id moveToCollection )
{
  mMoveToCollection = moveToCollection;
}

