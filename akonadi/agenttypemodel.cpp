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

#include "agenttypemodel.h"
#include "agenttype.h"
#include "agentmanager.h"

#include <QtCore/QStringList>
#include <QIcon>

using namespace Akonadi;

/**
 * @internal
 */
class AgentTypeModel::Private
{
  public:
    Private( AgentTypeModel *parent )
      : mParent( parent )
    {
      mTypes = AgentManager::self()->types();
    }

    AgentTypeModel *mParent;
    AgentType::List mTypes;

    void typeAdded( const AgentType &agentType );
    void typeRemoved( const AgentType &agentType );
};

void AgentTypeModel::Private::typeAdded( const AgentType &agentType )
{
  mTypes.append( agentType );

  emit mParent->layoutChanged();
}

void AgentTypeModel::Private::typeRemoved( const AgentType &agentType )
{
  mTypes.removeAll( agentType );

  emit mParent->layoutChanged();
}

AgentTypeModel::AgentTypeModel( QObject *parent )
  : QAbstractItemModel( parent ), d( new Private( this ) )
{
  connect( AgentManager::self(), SIGNAL(typeAdded(Akonadi::AgentType)),
           this, SLOT(typeAdded(Akonadi::AgentType)) );
  connect( AgentManager::self(), SIGNAL(typeRemoved(Akonadi::AgentType)),
           this, SLOT(typeRemoved(Akonadi::AgentType)) );
}

AgentTypeModel::~AgentTypeModel()
{
  delete d;
}

int AgentTypeModel::columnCount( const QModelIndex& ) const
{
  return 1;
}

int AgentTypeModel::rowCount( const QModelIndex& ) const
{
  return d->mTypes.count();
}

QVariant AgentTypeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() ) {
    return QVariant();
  }

  if ( index.row() < 0 || index.row() >= d->mTypes.count() ) {
    return QVariant();
  }

  const AgentType &type = d->mTypes[ index.row() ];

  switch ( role ) {
    case Qt::DisplayRole:
      return type.name();
      break;
    case Qt::DecorationRole:
      return type.icon();
      break;
    case TypeRole:
      {
        QVariant var;
        var.setValue( type );
        return var;
      }
      break;
    case IdentifierRole:
      return type.identifier();
      break;
    case DescriptionRole:
      return type.description();
      break;
    case MimeTypesRole:
      return type.mimeTypes();
      break;
    case CapabilitiesRole:
      return type.capabilities();
      break;
    default:
      break;
  }
  return QVariant();
}

QModelIndex AgentTypeModel::index( int row, int column, const QModelIndex& ) const
{
  if ( row < 0 || row >= d->mTypes.count() ) {
    return QModelIndex();
  }

  if ( column != 0 ) {
    return QModelIndex();
  }

  return createIndex( row, column, 0 );
}

QModelIndex AgentTypeModel::parent( const QModelIndex& ) const
{
  return QModelIndex();
}

Qt::ItemFlags AgentTypeModel::flags(const QModelIndex& index) const
{
  if ( !index.isValid() || index.row() < 0 || index.row() >= d->mTypes.count() ) {
    return QAbstractItemModel::flags( index );
  }

  const AgentType &type = d->mTypes[ index.row() ];
  if ( type.capabilities().contains( QLatin1String( "Unique" ) ) &&
       AgentManager::self()->instance( type.identifier() ).isValid() ) {
    return QAbstractItemModel::flags( index ) & ~( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  }
  return QAbstractItemModel::flags( index );
}

#include "moc_agenttypemodel.cpp"
