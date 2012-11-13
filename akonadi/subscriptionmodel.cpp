/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "subscriptionmodel_p.h"
#include "collectionfetchjob.h"
#include "collectionutils_p.h"
#include "specialcollectionattribute_p.h"

#include "entityhiddenattribute.h"

#include <kdebug.h>

#include <QtCore/QStringList>
#include <QFont>

using namespace Akonadi;

/**
 * @internal
 */
class SubscriptionModel::Private
{
  public:
    Private( SubscriptionModel* parent ) : q( parent ), showHiddenCollection(false) {}
    SubscriptionModel* q;
    QHash<Collection::Id, bool> subscriptions;
    QSet<Collection::Id> changes;
    bool showHiddenCollection;

    Collection::List changedSubscriptions( bool subscribed )
    {
      Collection::List list;
      foreach ( Collection::Id id, changes ) {
        if ( subscriptions.value( id ) == subscribed )
          list << Collection( id );
      }
      return list;
    }

    void listResult( KJob* job )
    {
      if ( job->error() ) {
        // TODO
        kWarning() << job->errorString();
        return;
      }
      Collection::List cols = static_cast<CollectionFetchJob*>( job )->collections();
      foreach ( const Collection &col, cols )
        if ( !CollectionUtils::isStructural( col ) )
          subscriptions[ col.id() ] = true;
      q->reset();
      emit q->loaded();
    }

    bool isSubscribable( Collection::Id id )
    {
      Collection col = q->collectionForId( id );
      if ( CollectionUtils::isStructural( col ) || col.isVirtual() )
        return false;
      if ( col.hasAttribute<SpecialCollectionAttribute>() )
        return false;
      if ( col.contentMimeTypes().isEmpty() )
        return false;
      return true;
    }
};

SubscriptionModel::SubscriptionModel(QObject * parent) :
    CollectionModel( parent ),
    d( new Private( this ) )
{
  includeUnsubscribed();
  CollectionFetchJob* job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, this );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(listResult(KJob*)) );
}

SubscriptionModel::~ SubscriptionModel()
{
  delete d;
}

QVariant SubscriptionModel::data(const QModelIndex & index, int role) const
{
  switch ( role ) {
    case Qt::CheckStateRole:
    {
      const Collection::Id col = index.data( CollectionIdRole ).toLongLong();
      if ( !d->isSubscribable( col ) )
        return QVariant();
      if ( d->subscriptions.value( col ) )
        return Qt::Checked;
      return Qt::Unchecked;
    }
    case SubscriptionChangedRole:
    {
      const Collection::Id col = index.data( CollectionIdRole ).toLongLong();
      if ( d->changes.contains( col ) )
        return true;
      return false;
    }
    case Qt::FontRole:
    {
      const Collection::Id col = index.data( CollectionIdRole ).toLongLong();

      QFont font = CollectionModel::data( index, role ).value<QFont>();
      font.setBold( d->changes.contains( col ) );

      return font;
    }
  }

  if ( role == CollectionIdRole ) {
    return CollectionModel::data( index, CollectionIdRole );
  } else {
    const Collection::Id collectionId = index.data( CollectionIdRole ).toLongLong();
    const Collection collection = collectionForId( collectionId );
    if( collection.hasAttribute<EntityHiddenAttribute>() ) {
      if(d->showHiddenCollection) {
        return CollectionModel::data( index, role );
      } else {
        return QVariant();
      }
    } else {
      return CollectionModel::data( index, role );
    }
  }
}

Qt::ItemFlags SubscriptionModel::flags(const QModelIndex & index) const
{
  Qt::ItemFlags flags = CollectionModel::flags( index );
  if ( d->isSubscribable( index.data( CollectionIdRole ).toLongLong() ) )
    return flags | Qt::ItemIsUserCheckable;
  return flags;
}

bool SubscriptionModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  if ( role == Qt::CheckStateRole ) {
    const Collection::Id col = index.data( CollectionIdRole ).toLongLong();
    if( !d->isSubscribable(col) ) {
      return true; //No change
    }
    if ( d->subscriptions.contains( col ) && d->subscriptions.value( col ) == (value == Qt::Checked) )
      return true; // no change
    d->subscriptions[ col ] = value == Qt::Checked;
    if ( d->changes.contains( col ) )
      d->changes.remove( col );
    else
      d->changes.insert( col );
    emit dataChanged( index, index );
    return true;
  }
  return CollectionModel::setData( index, value, role );
}

Akonadi::Collection::List SubscriptionModel::subscribed() const
{
  return d->changedSubscriptions( true );
}

Akonadi::Collection::List SubscriptionModel::unsubscribed() const
{
  return d->changedSubscriptions( false );
}

void SubscriptionModel::showHiddenCollection(bool showHidden)
{
  d->showHiddenCollection = showHidden;
}

#include "moc_subscriptionmodel_p.cpp"
