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



using namespace Akonadi;
using namespace MailTransport;

class SentBehaviourAttribute::Private
{
  public:
    SentBehaviour mBehaviour;
    Akonadi::Collection mMoveToCollection;
};

SentBehaviourAttribute::SentBehaviourAttribute( SentBehaviour beh, Collection moveToCollection )
  : d( new Private )
{
  d->mBehaviour = beh;
  d->mMoveToCollection = moveToCollection;
}

SentBehaviourAttribute::~SentBehaviourAttribute()
{
  delete d;
}

SentBehaviourAttribute *SentBehaviourAttribute::clone() const
{
  return new SentBehaviourAttribute( d->mBehaviour, d->mMoveToCollection );
}

QByteArray SentBehaviourAttribute::type() const
{
  static const QByteArray sType( "SentBehaviourAttribute" );
  return sType;
}

QByteArray SentBehaviourAttribute::serialized() const
{
  switch ( d->mBehaviour ) {
    case Delete: return "delete";
    case MoveToCollection: return "moveTo" + QByteArray::number( d->mMoveToCollection.id() );
    case MoveToDefaultSentCollection: return "moveToDefault";
  }

  Q_ASSERT( false );
  return QByteArray();
}

void SentBehaviourAttribute::deserialize( const QByteArray &data )
{
  d->mMoveToCollection = Akonadi::Collection( -1 );
  if ( data == "delete" ) {
    d->mBehaviour = Delete;
  } else if ( data == "moveToDefault" ) {
    d->mBehaviour = MoveToDefaultSentCollection;
  } else if ( data.startsWith( QByteArray( "moveTo" ) ) ) {
    d->mBehaviour = MoveToCollection;
    d->mMoveToCollection = Akonadi::Collection( data.mid( 6 ).toLongLong() );
    // NOTE: 6 is the strlen of "moveTo".
  } else {
    Q_ASSERT( false );
  }
}

SentBehaviourAttribute::SentBehaviour SentBehaviourAttribute::sentBehaviour() const
{
  return d->mBehaviour;
}

void SentBehaviourAttribute::setSentBehaviour( SentBehaviour beh )
{
  d->mBehaviour = beh;
}

Collection SentBehaviourAttribute::moveToCollection() const
{
  return d->mMoveToCollection;
}

void SentBehaviourAttribute::setMoveToCollection( Collection moveToCollection )
{
  d->mMoveToCollection = moveToCollection;
}

