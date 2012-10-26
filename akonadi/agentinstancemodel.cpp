/*
    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

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

#include "agentinstancemodel.h"

#include "agentinstance.h"
#include "agentmanager.h"

#include <QtCore/QStringList>
#include <QIcon>

#include <klocale.h>

using namespace Akonadi;

/**
 * @internal
 */
class AgentInstanceModel::Private
{
  public:
    Private( AgentInstanceModel *parent )
      : mParent( parent )
    {
    }

    AgentInstanceModel *mParent;
    AgentInstance::List mInstances;

    void instanceAdded( const AgentInstance& );
    void instanceRemoved( const AgentInstance& );
    void instanceChanged( const AgentInstance& );
};

void AgentInstanceModel::Private::instanceAdded( const AgentInstance &instance )
{
  mParent->beginInsertRows( QModelIndex(), mInstances.count(), mInstances.count() );
  mInstances.append( instance );
  mParent->endInsertRows();
}

void AgentInstanceModel::Private::instanceRemoved( const AgentInstance &instance )
{
  const int index = mInstances.indexOf( instance );
  if ( index == -1 ) {
    return;
  }

  mParent->beginRemoveRows( QModelIndex(), index, index );
  mInstances.removeAll( instance );
  mParent->endRemoveRows();
}

void AgentInstanceModel::Private::instanceChanged( const AgentInstance &instance )
{
  const int numberOfInstance( mInstances.count() );
  for ( int i = 0; i < numberOfInstance; ++i ) {
    if ( mInstances[ i ] == instance ) {
      mInstances[ i ] = instance;

      const QModelIndex idx = mParent->index( i, 0 );
      emit mParent->dataChanged( idx, idx );

      return;
    }
  }
}


AgentInstanceModel::AgentInstanceModel( QObject *parent )
  : QAbstractItemModel( parent ), d( new Private( this ) )
{
  d->mInstances = AgentManager::self()->instances();

  QHash<int, QByteArray> roles = roleNames();
  roles.insert( StatusRole, "status" );
  roles.insert( StatusMessageRole, "statusMessage" );
  roles.insert( ProgressRole, "progress" );
  roles.insert( OnlineRole, "online" );
  setRoleNames( roles );

  connect( AgentManager::self(), SIGNAL(instanceAdded(Akonadi::AgentInstance)),
           this, SLOT(instanceAdded(Akonadi::AgentInstance)) );
  connect( AgentManager::self(), SIGNAL(instanceRemoved(Akonadi::AgentInstance)),
           this, SLOT(instanceRemoved(Akonadi::AgentInstance)) );
  connect( AgentManager::self(), SIGNAL(instanceStatusChanged(Akonadi::AgentInstance)),
           this, SLOT(instanceChanged(Akonadi::AgentInstance)) );
  connect( AgentManager::self(), SIGNAL(instanceProgressChanged(Akonadi::AgentInstance)),
           this, SLOT(instanceChanged(Akonadi::AgentInstance)) );
  connect( AgentManager::self(), SIGNAL(instanceNameChanged(Akonadi::AgentInstance)),
           this, SLOT(instanceChanged(Akonadi::AgentInstance)) );
  connect( AgentManager::self(), SIGNAL(instanceOnline(Akonadi::AgentInstance,bool)),
           this, SLOT(instanceChanged(Akonadi::AgentInstance)) );
}

AgentInstanceModel::~AgentInstanceModel()
{
  delete d;
}

int AgentInstanceModel::columnCount( const QModelIndex& ) const
{
  return 1;
}

int AgentInstanceModel::rowCount( const QModelIndex& ) const
{
  return d->mInstances.count();
}

QVariant AgentInstanceModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() ) {
    return QVariant();
  }

  if ( index.row() < 0 || index.row() >= d->mInstances.count() ) {
    return QVariant();
  }

  const AgentInstance &instance = d->mInstances[ index.row() ];

  switch ( role ) {
    case Qt::DisplayRole:
      return instance.name();
    case Qt::DecorationRole:
      return instance.type().icon();
    case InstanceRole:
      {
        QVariant var;
        var.setValue( instance );
        return var;
      }
    case InstanceIdentifierRole:
      return instance.identifier();
    case Qt::ToolTipRole:
      return QString::fromLatin1( "<qt><h4>%1</h4>%2</qt>" ).arg( instance.name(), instance.type().description() );
    case StatusRole:
      return instance.status();
    case StatusMessageRole:
      return instance.statusMessage();
    case ProgressRole:
      return instance.progress();
    case OnlineRole:
      return instance.isOnline();
    case TypeRole:
      {
        QVariant var;
        var.setValue( instance.type() );
        return var;
      }
    case TypeIdentifierRole:
      return instance.type().identifier();
    case DescriptionRole:
      return instance.type().description();
    case CapabilitiesRole:
      return instance.type().capabilities();
    case MimeTypesRole:
      return instance.type().mimeTypes();
  }
  return QVariant();
}

QVariant AgentInstanceModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Vertical ) {
    return QVariant();
  }

  if ( role != Qt::DisplayRole ) {
    return QVariant();
  }

  switch ( section ) {
    case 0:
      return i18nc( "@title:column, name of a thing", "Name" );
      break;
    default:
      return QVariant();
      break;
  }
}

QModelIndex AgentInstanceModel::index( int row, int column, const QModelIndex& ) const
{
  if ( row < 0 || row >= d->mInstances.count() ) {
    return QModelIndex();
  }

  if ( column != 0 ) {
    return QModelIndex();
  }

  return createIndex( row, column, 0 );
}

QModelIndex AgentInstanceModel::parent( const QModelIndex& ) const
{
  return QModelIndex();
}

Qt::ItemFlags AgentInstanceModel::flags( const QModelIndex & index ) const
{
  if ( !index.isValid() || index.row() < 0 || index.row() >= d->mInstances.count() ) {
    return QAbstractItemModel::flags( index );
  }

  return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable;
}

bool AgentInstanceModel::setData( const QModelIndex & index, const QVariant & value, int role )
{
  if ( !index.isValid() ) {
    return false;
  }

  if ( index.row() < 0 || index.row() >= d->mInstances.count() ) {
    return false;
  }

  AgentInstance &instance = d->mInstances[ index.row() ];

  switch ( role ) {
    case OnlineRole:
      instance.setIsOnline( value.toBool() );
      emit dataChanged( index, index );
      return true;
    default:
      return false;
  }

  return false;
}

#include "moc_agentinstancemodel.cpp"
