/*
 Copyright (c) 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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
#include "trashfilterproxymodel.h"

#include <akonadi/entity.h>
#include <akonadi/entitydeletedattribute.h>
#include <akonadi/item.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/resourcesettings.h>

using namespace Akonadi;

class TrashFilterProxyModel::TrashFilterProxyModelPrivate
{
  public:
    TrashFilterProxyModelPrivate() : mTrashIsShown( false ) {};
    bool showEntity( const Entity & ) const;
    bool mTrashIsShown;
};

bool TrashFilterProxyModel::TrashFilterProxyModelPrivate::showEntity( const Akonadi::Entity &entity ) const
{
  if ( entity.hasAttribute<EntityDeletedAttribute>() ) {
    return true;
  }
  if ( entity.id() == Collection::root().id() ) {
    return false;
  }
  return showEntity( entity.parentCollection() );
}


TrashFilterProxyModel::TrashFilterProxyModel( QObject* parent )
    : KRecursiveFilterProxyModel( parent ),
    d_ptr( new TrashFilterProxyModelPrivate() )
{

}

TrashFilterProxyModel::~TrashFilterProxyModel()
{
  delete d_ptr;
}


void TrashFilterProxyModel::showTrash( bool enable )
{
  Q_D( TrashFilterProxyModel );
  d->mTrashIsShown = enable;
  invalidateFilter();
}

bool TrashFilterProxyModel::trashIsShown() const
{
  Q_D( const TrashFilterProxyModel );
  return d->mTrashIsShown;
}


bool TrashFilterProxyModel::acceptRow( int sourceRow, const QModelIndex& sourceParent ) const
{
  Q_D( const TrashFilterProxyModel );
  const QModelIndex &index = sourceModel()->index( sourceRow, 0, sourceParent );
  const Item &item = index.data( EntityTreeModel::ItemRole ).value<Item>();
  //kDebug() << item.id();
  if ( item.isValid() ) {
    if ( item.hasAttribute<EntityDeletedAttribute>()/* d->showEntity(item)*/ ) {
      return d->mTrashIsShown;
    }
  }
  const Collection &collection = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
  if ( collection.isValid() ) {
    if ( collection.hasAttribute<EntityDeletedAttribute>()/*d->showEntity(collection) */ ) {
      return d->mTrashIsShown;
    }
  }
  return !d->mTrashIsShown;
}



