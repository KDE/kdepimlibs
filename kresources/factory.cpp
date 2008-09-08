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
/**
  @file
  This file is part of the KDE resource framework and defines the
  Factory class.

  @brief
  A class for loading resource plugins.

  @author Tobias Koenig
  @author Jan-Pascal van Best
  @author Cornelius Schumacher
*/

#include "factory.h"

#include <QtCore/QFile>

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kservicetypetrader.h>
#include <kpluginloader.h>

#include "resource.h"

using namespace KRES;

class Factory::Private
{
  public:
    Resource *resourceInternal ( const QString &type, const KConfigGroup *group );
    QString mResourceFamily;
    QMap<QString, KService::Ptr> mTypeMap;
};

typedef QMap<QString, Factory*> factoryMap;
K_GLOBAL_STATIC( factoryMap, mSelves )

Factory *Factory::self( const QString &resourceFamily )
{
  kDebug();

  Factory *factory = 0;

  factory = mSelves->value( resourceFamily, 0 );

  if ( !factory ) {
    factory = new Factory( resourceFamily );
    mSelves->insert( resourceFamily, factory );
  }

  return factory;
}

Factory *Factory::recreateSelf( const QString &resourceFamily )
{
  kDebug();
  Factory *factory = 0;
  factory = mSelves->value( resourceFamily, 0 );
  if ( factory ) {
    delete factory;
    mSelves->remove( resourceFamily );
  }

  return self( resourceFamily );
}

Factory::Factory( const QString &resourceFamily ) :
  d( new KRES::Factory::Private )
{
  d->mResourceFamily = resourceFamily;
  const KService::List plugins =
    KServiceTypeTrader::self()->query(
      "KResources/Plugin",
      QString( "[X-KDE-ResourceFamily] == '%1'" ).arg( d->mResourceFamily ) );

  KService::List::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    const QVariant type = (*it)->property( "X-KDE-ResourceType" );
    if ( !type.toString().isEmpty() ) {
      d->mTypeMap.insert( type.toString(), *it );
    }
  }
}

Factory::~Factory()
{
  delete d;
}

QStringList Factory::typeNames() const
{
  return d->mTypeMap.keys();
}

ConfigWidget *Factory::configWidget( const QString &type, QWidget *parent )
{
  if ( type.isEmpty() || !d->mTypeMap.contains( type ) ) {
    return 0;
  }

  KService::Ptr ptr = d->mTypeMap[ type ];
  KPluginLoader loader( ptr->library() );
  KPluginFactory *factory = loader.factory();
  if ( !factory ) {
    kDebug() << "Factory creation failed: " << loader.errorString();
    return 0;
  }

  PluginFactoryBase *pluginFactory = static_cast<PluginFactoryBase *>( factory );

  if ( !pluginFactory ) {
    kDebug() << "no plugin factory.";
    return 0;
  }

  ConfigWidget *wdg = pluginFactory->configWidget( parent );
  if ( !wdg ) {
    kDebug() << "'" << ptr->library() << "' doesn't provide a ConfigWidget";
    return 0;
  }

  return wdg;
}

QString Factory::typeName( const QString &type ) const
{
  if ( type.isEmpty() || !d->mTypeMap.contains( type ) ) {
    return QString();
  }

  KService::Ptr ptr = d->mTypeMap[ type ];
  return ptr->name();
}

QString Factory::typeDescription( const QString &type ) const
{
  if ( type.isEmpty() || !d->mTypeMap.contains( type ) ) {
    return QString();
  }

  KService::Ptr ptr = d->mTypeMap[ type ];
  return ptr->comment();
}

Resource *Factory::Private::resourceInternal( const QString &type, const KConfigGroup *group )
{
  kDebug() << "(" << type << ", config )";

  if ( type.isEmpty() || !mTypeMap.contains( type ) ) {
    kDebug() << "no such type" << type;
    return 0;
  }

  KService::Ptr ptr = mTypeMap[ type ];
  KPluginLoader loader( ptr->library() );
  KPluginFactory *factory = loader.factory();
  if ( !factory ) {
    kDebug() << "Factory creation failed" << loader.errorString();
    return 0;
  }

  PluginFactoryBase *pluginFactory = static_cast<PluginFactoryBase *>( factory );

  if ( !pluginFactory ) {
    kDebug() << "no plugin factory.";
    return 0;
  }

  Resource *resource;
  if ( group ) {
    resource = pluginFactory->resource( *group );
  } else {
    resource = pluginFactory->resource();
  }

  if ( !resource ) {
    kDebug() << "'" << ptr->library()
             << "' is not a" << mResourceFamily << "plugin.";
    return 0;
  }

  resource->setType( type );

  return resource;
}

Resource *Factory::resource( const QString &type, const KConfigGroup &group )
{
  return d->resourceInternal( type, &group );
}

Resource *Factory::resource( const QString &type )
{
  return d->resourceInternal( type, 0 );
}
