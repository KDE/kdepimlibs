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

#include "monitor.h"
#include "monitor_p.h"

#include "collectionfetchscope.h"
#include "itemfetchjob.h"
#include "session.h"
#include "idlejob_p.h"

#include <kdebug.h>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>

#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <iterator>

using namespace Akonadi;

Monitor::Monitor( QObject *parent ) :
    QObject( parent ),
    d_ptr( new MonitorPrivate( this ) )
{
  d_ptr->init();
}

//@cond PRIVATE
Monitor::Monitor(MonitorPrivate * d, QObject *parent) :
    QObject( parent ),
    d_ptr( d )
{
  d_ptr->init();
}
//@endcond

Monitor::~Monitor()
{
  delete d_ptr;
}

void Monitor::setCollectionMonitored( const Collection &collection, bool monitored )
{
  Q_D( Monitor );
  if ( !collection.isValid() ) {
    return;
  }

  if ( monitored ) {
    d->idleJob->addMonitoredCollection( collection );
    d->monitoredCollections << collection;
  } else {
    d->idleJob->removeMonitoredCollection( collection );
    d->monitoredCollections.removeAll( collection );
  }

  emit collectionMonitored( collection, monitored );
}

void Monitor::setItemMonitored( const Item &item, bool monitored )
{
  Q_D( Monitor );
  if ( !item.isValid() ) {
    return;
  }

  if ( monitored ) {
    d->idleJob->addMonitoredItem( item.id() );
    d->monitoredItems.insert( item.id() );
  } else {
    d->idleJob->removeMonitoredItem( item.id() );
    d->monitoredItems.remove( item.id() );
  }

  emit itemMonitored( item,  monitored );
}

void Monitor::setResourceMonitored( const QByteArray &resource, bool monitored )
{
  Q_D( Monitor );

  if ( monitored ) {
    d->idleJob->addMonitoredResource( resource );
    d->monitoredResources.insert( resource );
  } else {
    d->idleJob->removeMonitoredResource( resource );
    d->monitoredResources.remove( resource );
  }

  emit resourceMonitored( resource, monitored );
}

void Monitor::setMimeTypeMonitored( const QString &mimetype, bool monitored )
{
  Q_D( Monitor );

  if ( monitored ) {
    d->idleJob->addMonitoredMimeType( mimetype );
    d->monitoredMimetypes.insert( mimetype );
  } else {
    d->idleJob->removeMonitoredMimeType( mimetype );
    d->monitoredMimetypes.remove( mimetype );
  }

  emit mimeTypeMonitored( mimetype, monitored );
}

void Akonadi::Monitor::setAllMonitored( bool monitored )
{
  Q_D( Monitor );

  d->idleJob->setAllMonitored( monitored );
  emit allMonitored( monitored );
}

void Monitor::ignoreSession( Session * session )
{
  Q_D( Monitor );

  d->idleJob->addIgnoredSession( session->sessionId() );
  d->ignoredSessions.insert( session, session->sessionId() );
  connect( session, SIGNAL(destroyed(QObject*)), this, SLOT(slotSessionDestroyed(QObject*)) );
}

void Monitor::fetchCollection( bool enable )
{
  Q_D( Monitor );
  d->fetchCollection = enable;
}

void Monitor::fetchCollectionStatistics( bool enable )
{
  Q_D( Monitor );
  d->fetchCollectionStatistics = enable;
}

void Monitor::setItemFetchScope( const ItemFetchScope &fetchScope )
{
  Q_D( Monitor );
  d->mItemFetchScope = fetchScope;
}

ItemFetchScope &Monitor::itemFetchScope()
{
  Q_D( Monitor );
  return d->mItemFetchScope;
}

void Monitor::fetchChangedOnly( bool enable )
{
  Q_D( Monitor );
  d->mFetchChangedOnly = enable;
}

void Monitor::setCollectionFetchScope( const CollectionFetchScope &fetchScope )
{
  Q_D( Monitor );
  d->mCollectionFetchScope = fetchScope;
}

CollectionFetchScope& Monitor::collectionFetchScope()
{
  Q_D( Monitor );
  return d->mCollectionFetchScope;
}

Akonadi::Collection::List Monitor::collectionsMonitored() const
{
  Q_D( const Monitor );
  return d->monitoredCollections;
}

QList<Item::Id> Monitor::itemsMonitored() const
{
  Q_D( const Monitor );
  return d->monitoredItems.toList();
}

QVector<Item::Id> Monitor::itemsMonitoredEx() const
{
  Q_D( const Monitor );
  QVector<Item::Id> result;
  result.reserve( d->monitoredItems.size() );
  qCopy( d->monitoredItems.begin(), d->monitoredItems.end(), std::back_inserter( result ) );
  return result;
}

QStringList Monitor::mimeTypesMonitored() const
{
  Q_D( const Monitor );
  return d->monitoredMimetypes.toList();
}

QList<QByteArray> Monitor::resourcesMonitored() const
{
  Q_D( const Monitor );
  return d->monitoredResources.toList();
}

bool Monitor::isAllMonitored() const
{
  Q_D( const Monitor );
  return d->monitorAll;
}

// TODO Remove in KF5
void Monitor::setSession( Akonadi::Session *session )
{
  Q_UNUSED( session );
}

Session* Monitor::session() const
{
  return 0L;
}

void Monitor::setCollectionMoveTranslationEnabled( bool enabled )
{
  Q_D( Monitor );
  d->collectionMoveTranslationEnabled = enabled;
}

#include "moc_monitor.cpp"
