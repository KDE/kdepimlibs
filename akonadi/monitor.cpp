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

#include "itemfetchjob.h"
#include "notificationmessage_p.h"
#include "session.h"

#include <kdebug.h>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>

#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include "collectionfetchscope.h"
#include <iterator>

using namespace Akonadi;

#define d d_ptr

Monitor::Monitor( QObject *parent ) :
    QObject( parent ),
    d_ptr( new MonitorPrivate( this ) )
{
  d_ptr->init();
  d_ptr->connectToNotificationManager();
}

//@cond PRIVATE
Monitor::Monitor(MonitorPrivate * d, QObject *parent) :
    QObject( parent ),
    d_ptr( d )
{
}
//@endcond

Monitor::~Monitor()
{
  // :TODO: Unsubscribe from the notification manager. That means having some kind of reference
  // counting on the server side.
  delete d;
}

void Monitor::setCollectionMonitored( const Collection &collection, bool monitored )
{
  if ( monitored ) {
    d->collections << collection;
  } else {
    d->collections.removeAll( collection );
    d->cleanOldNotifications();
  }
  emit collectionMonitored( collection, monitored );
}

void Monitor::setItemMonitored( const Item & item, bool monitored )
{
  if ( monitored ) {
    d->items.insert( item.id() );
  } else {
    d->items.remove( item.id() );
    d->cleanOldNotifications();
  }
  emit itemMonitored( item,  monitored );
}

void Monitor::setResourceMonitored( const QByteArray & resource, bool monitored )
{
  if ( monitored ) {
    d->resources.insert( resource );
  } else {
    d->resources.remove( resource );
    d->cleanOldNotifications();
  }
  emit resourceMonitored( resource, monitored );
}

void Monitor::setMimeTypeMonitored( const QString & mimetype, bool monitored )
{
  if ( monitored ) {
    d->mimetypes.insert( mimetype );
  } else {
    d->mimetypes.remove( mimetype );
    d->cleanOldNotifications();
  }

  emit mimeTypeMonitored( mimetype, monitored );
}

void Akonadi::Monitor::setAllMonitored( bool monitored )
{
  d->monitorAll = monitored;

  if ( !monitored ) {
    d->cleanOldNotifications();
  }

  emit allMonitored( monitored );
}

void Monitor::ignoreSession(Session * session)
{
  d->sessions << session->sessionId();
  connect( session, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotSessionDestroyed( QObject* ) ) );
}

void Monitor::fetchCollection(bool enable)
{
  d->fetchCollection = enable;
}

void Monitor::fetchCollectionStatistics(bool enable)
{
  d->fetchCollectionStatistics = enable;
}

void Monitor::setItemFetchScope( const ItemFetchScope &fetchScope )
{
  d->mItemFetchScope = fetchScope;
}

ItemFetchScope &Monitor::itemFetchScope()
{
  return d->mItemFetchScope;
}

void Monitor::setCollectionFetchScope( const CollectionFetchScope &fetchScope )
{
  d->mCollectionFetchScope = fetchScope;
}

CollectionFetchScope& Monitor::collectionFetchScope()
{
  return d->mCollectionFetchScope;
}

Collection::List Monitor::collectionsMonitored() const
{
  return d->collections;
}

QList<Item::Id> Monitor::itemsMonitored() const
{
  return d->items.toList();
}

QVector<Item::Id> Monitor::itemsMonitoredEx() const
{
  QVector<Item::Id> result;
  result.reserve( d->items.size() );
  qCopy( d->items.begin(), d->items.end(), std::back_inserter( result ) );
  return result;
}

QStringList Monitor::mimeTypesMonitored() const
{
  return d->mimetypes.toList();
}

QList<QByteArray> Monitor::resourcesMonitored() const
{
  return d->resources.toList();
}

bool Monitor::isAllMonitored() const
{
  return d->monitorAll;
}

void Monitor::setSession( Akonadi::Session *session )
{
  if (session == d->session)
    return;

  if (!session)
    d->session = Session::defaultSession();
  else
    d->session = session;

  d->itemCache.setSession(d->session);
  d->collectionCache.setSession(d->session);
}

Session* Monitor::session() const
{
  return d->session;
}

#undef d

#include "monitor.moc"
