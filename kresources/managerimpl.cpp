/*
    This file is part of libkresources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "managerimpl.h"

#include <kaboutdata.h>
#include <krandom.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <QtDBus/QtDBus>

#include "resource.h"
#include "factory.h"
#include "manager.h"
#include "kresourcesmanageradaptor.h"

using namespace KRES;

ManagerImpl::ManagerImpl( ManagerNotifier *notifier, const QString &family )
  : mNotifier( notifier ),
    mFamily( family ), mConfig( 0 ), mStdConfig( 0 ), mStandard( 0 ),
    mFactory( 0 ), mConfigRead( false )
{
  new KResourcesManagerAdaptor(this);
  const QString dBusPath = QLatin1String("/ManagerIface_") + family;
  QDBusConnection::sessionBus().registerObject(dBusPath, this);
  kDebug(5650) << "ManagerImpl::ManagerImpl()" << endl;

  mId = KRandom::randomString( 8 );

  // Register with D-Bus
  QDBusConnection::sessionBus().registerService("org.kde.KResourcesManager");

  QDBusConnection::sessionBus().connect("", dBusPath, "org.kde.KResourcesManager", "signalKResourceAdded",
      this, SLOT(dbusKResourceAdded(QString,QString)));
  QDBusConnection::sessionBus().connect("", dBusPath, "org.kde.KResourcesManager", "signalKResourceModified",
      this, SLOT(dbusKResourceModified(QString,QString)));
  QDBusConnection::sessionBus().connect("", dBusPath, "org.kde.KResourcesManager", "signalKResourceDeleted",
      this, SLOT(dbusKResourceDeleted(QString,QString)));
}

ManagerImpl::~ManagerImpl()
{
  kDebug(5650) << "ManagerImpl::~ManagerImpl()" << endl;

  qDeleteAll(mResources);
  delete mStdConfig;
}

void ManagerImpl::createStandardConfig()
{
  if ( !mStdConfig ) {
    QString file = defaultConfigFile( mFamily );
    mStdConfig = new KConfig( file );
  }

  mConfig = mStdConfig;
}

void ManagerImpl::readConfig( KConfig *cfg )
{
  kDebug(5650) << "ManagerImpl::readConfig()" << endl;

  delete mFactory;
  mFactory = Factory::self( mFamily );

  if ( !cfg ) {
    createStandardConfig();
  } else {
    mConfig = cfg;
  }

  mStandard = 0;
  KConfigGroup group = mConfig->group( "General" );

  QStringList keys = group.readEntry( "ResourceKeys", QStringList() );
  keys += group.readEntry( "PassiveResourceKeys", QStringList() );

  QString standardKey = group.readEntry( "Standard" );

  for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
    readResourceConfig( *it, false );
  }

  mConfigRead = true;
}

void ManagerImpl::writeConfig( KConfig *cfg )
{
  kDebug(5650) << "ManagerImpl::writeConfig()" << endl;

  if ( !cfg ) {
    createStandardConfig();
  } else {
    mConfig = cfg;
  }

  QStringList activeKeys;
  QStringList passiveKeys;

  // First write all keys, collect active and passive keys on the way
  Resource::List::Iterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    writeResourceConfig( *it, false );

    QString key = (*it)->identifier();
    if( (*it)->isActive() )
      activeKeys.append( key );
    else
      passiveKeys.append( key );
  }

  // And then the general group

  kDebug(5650) << "Saving general info" << endl;
  KConfigGroup group = mConfig->group( "General" );
  group.writeEntry( "ResourceKeys", activeKeys );
  group.writeEntry( "PassiveResourceKeys", passiveKeys );
  if ( mStandard )
    group.writeEntry( "Standard", mStandard->identifier() );
  else
    group.writeEntry( "Standard", "" );

  group.sync();
  kDebug(5650) << "ManagerImpl::save() finished" << endl;
}

void ManagerImpl::add( Resource *resource )
{
  resource->setActive( true );

  if ( mResources.isEmpty() ) {
    mStandard = resource;
  }

  mResources.append( resource );

  if ( mConfigRead )
    writeResourceConfig( resource, true );

  signalKResourceAdded( mId, resource->identifier() );
}

void ManagerImpl::remove( Resource *resource )
{
  if ( mStandard == resource ) mStandard = 0;
  removeResource( resource );

  mResources.removeAll( resource );

  signalKResourceDeleted( mId, resource->identifier() );

  delete resource;

  kDebug(5650) << "Finished ManagerImpl::remove()" << endl;
}

void ManagerImpl::change( Resource *resource )
{
  writeResourceConfig( resource, true );

  signalKResourceModified( mId, resource->identifier() );
}

void ManagerImpl::setActive( Resource *resource, bool active )
{
  if ( resource && resource->isActive() != active ) {
    resource->setActive( active );
  }
}

Resource *ManagerImpl::standardResource()
{
  return mStandard;
}

void ManagerImpl::setStandardResource( Resource *resource )
{
  mStandard = resource;
}

// DCOP asynchronous functions

void ManagerImpl::dbusKResourceAdded( const QString& managerId,
                                      const QString& resourceId )
{
  if ( managerId == mId ) {
    kDebug(5650) << "Ignore D-Bus notification to myself" << endl;
    return;
  }
  kDebug(5650) << "Receive D-Bus call: added resource " << resourceId << endl;

  if ( getResource( resourceId ) ) {
    kDebug(5650) << "This resource is already known to me." << endl;
  }

  if ( !mConfig ) createStandardConfig();

  mConfig->reparseConfiguration();
  Resource *resource = readResourceConfig( resourceId, true );

  if ( resource ) {
    mNotifier->notifyResourceAdded( resource );
  } else
    kError() << "Received D-Bus: resource added for unknown resource "
              << resourceId << endl;
}

void ManagerImpl::dbusKResourceModified( const QString& managerId,
                                         const QString& resourceId )
{
  if ( managerId == mId ) {
    kDebug(5650) << "Ignore D-Bus notification to myself" << endl;
    return;
  }
  kDebug(5650) << "Receive D-Bus call: modified resource " << resourceId << endl;

  Resource *resource = getResource( resourceId );
  if ( resource ) {
    mNotifier->notifyResourceModified( resource );
  } else
    kError() << "Received D-Bus: resource modified for unknown resource "
              << resourceId << endl;
}

void ManagerImpl::dbusKResourceDeleted( const QString& managerId,
                                        const QString& resourceId )
{
  if ( managerId == mId ) {
    kDebug(5650) << "Ignore D-Bus notification to myself" << endl;
    return;
  }
  kDebug(5650) << "Receive D-Bus call: deleted resource " << resourceId << endl;

  Resource *resource = getResource( resourceId );
  if ( resource ) {
    mNotifier->notifyResourceDeleted( resource );

    kDebug(5650) << "Removing item from mResources" << endl;
    // Now delete item
    if ( mStandard == resource )
      mStandard = 0;
    mResources.removeAll( resource );
  } else
    kError() << "Received D-Bus: resource deleted for unknown resource "
              << resourceId << endl;
}

QStringList ManagerImpl::resourceNames()
{
  QStringList result;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    result.append( (*it)->resourceName() );
  }
  return result;
}

Resource::List *ManagerImpl::resourceList()
{
  return &mResources;
}

QList<Resource *> ManagerImpl::resources()
{
  return QList<Resource *>( mResources );
}

QList<Resource *> ManagerImpl::resources( bool active )
{
  QList<Resource *> result;

  for(int i = 0; i < mResources.size(); ++i) {
    if( mResources.at(i)->isActive() == active) {
      result.append( mResources.at(i) );
    }
  }
  return result;
}

Resource *ManagerImpl::readResourceConfig( const QString &identifier,
                                           bool checkActive )
{
  kDebug(5650) << "ManagerImpl::readResourceConfig() " << identifier << endl;

  if ( !mFactory ) {
    kError(5650) << "ManagerImpl::readResourceConfig: mFactory is 0. Did the app forget to call readConfig?" << endl;
    return 0;
  }

  KConfigGroup group = mConfig->group( "Resource_" + identifier );

  QString type = group.readEntry( "ResourceType" );
  QString name = group.readEntry( "ResourceName" );
  Resource *resource = mFactory->resource( type, group );
  if ( !resource ) {
    kDebug(5650) << "Failed to create resource with id " << identifier << endl;
    return 0;
  }

  if ( resource->identifier().isEmpty() )
    resource->setIdentifier( identifier );

  group = mConfig->group( "General" );

  QString standardKey = group.readEntry( "Standard" );
  if ( standardKey == identifier ) {
    mStandard = resource;
  }

  if ( checkActive ) {
    QStringList activeKeys = group.readEntry( "ResourceKeys", QStringList() );
    resource->setActive( activeKeys.contains( identifier ) );
  }
  mResources.append( resource );

  return resource;
}

void ManagerImpl::writeResourceConfig( Resource *resource, bool checkActive )
{
  QString key = resource->identifier();

  kDebug(5650) << "Saving resource " << key << endl;

  if ( !mConfig ) createStandardConfig();

  KConfigGroup group( mConfig, "Resource_" + key );
  resource->writeConfig( group );

  group = mConfig->group( "General" );
  QString standardKey = group.readEntry( "Standard" );

  if ( resource == mStandard  && standardKey != key )
    group.writeEntry( "Standard", resource->identifier() );
  else if ( resource != mStandard && standardKey == key )
    group.writeEntry( "Standard", "" );

  if ( checkActive ) {
    QStringList activeKeys = group.readEntry( "ResourceKeys", QStringList() );
    QStringList passiveKeys = group.readEntry( "PassiveResourceKeys", QStringList() );
    if ( resource->isActive() ) {
      if ( passiveKeys.contains( key ) ) { // remove it from passive list
        passiveKeys.removeAll( key );
        group.writeEntry( "PassiveResourceKeys", passiveKeys );
      }
      if ( !activeKeys.contains( key ) ) { // add it to active list
        activeKeys.append( key );
        group.writeEntry( "ResourceKeys", activeKeys );
      }
    } else if ( !resource->isActive() ) {
      if ( activeKeys.contains( key ) ) { // remove it from active list
        activeKeys.removeAll( key );
        group.writeEntry( "ResourceKeys", activeKeys );
      }
      if ( !passiveKeys.contains( key ) ) { // add it to passive list
        passiveKeys.append( key );
        group.writeEntry( "PassiveResourceKeys", passiveKeys );
      }
    }
  }

  mConfig->sync();
}

void ManagerImpl::removeResource( Resource *resource )
{
  QString key = resource->identifier();

  if ( !mConfig ) createStandardConfig();

  KConfigGroup group = mConfig->group( "General" );
  QStringList activeKeys = group.readEntry( "ResourceKeys", QStringList() );
  if ( activeKeys.contains( key ) ) {
    activeKeys.removeAll( key );
    group.writeEntry( "ResourceKeys", activeKeys );
  } else {
    QStringList passiveKeys= group.readEntry( "PassiveResourceKeys", QStringList() );
    passiveKeys.removeAll( key );
    group.writeEntry( "PassiveResourceKeys", passiveKeys );
  }

  QString standardKey = group.readEntry( "Standard" );
  if ( standardKey == key ) {
    group.writeEntry( "Standard", "" );
  }

  mConfig->deleteGroup( "Resource_" + resource->identifier() );
  group.sync();
}

Resource *ManagerImpl::getResource( const QString &identifier )
{
  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    if ( (*it)->identifier() == identifier )
      return *it;
  }
  return 0;
}

QString ManagerImpl::defaultConfigFile( const QString &family )
{
  return KStandardDirs::locateLocal( "config",
                      QString( "kresources/%1/stdrc" ).arg( family ) );
}

#include "managerimpl.moc"
