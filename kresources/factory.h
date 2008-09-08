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

  @author Tobias Koenig
  @author Jan-Pascal van Best
  @author Cornelius Schumacher
*/

#ifndef KRESOURCES_FACTORY_H
#define KRESOURCES_FACTORY_H

#include <QtCore/QMap>
#include <QtCore/QString>

#include <kconfig.h>
#include <kservice.h>

#include "resource.h"
#include "configwidget.h"

namespace KRES {

/**
  @brief
  A class for loading resource plugins.

  Use this class if you need resources with special
  settings, otherwise use KRES::Manager::createResource()
  to get resources with the default settings.

  Example:

  \code
  KRES::Factory *factory = KRES::Factory::self( "contact" );

  // to allow a transparent configuration of resources, we have
  // to use a kconfig object.
  KConfig config;
  KConfigGroup group( &config, "General" );
  group.writePathEntry( "FileName", "/home/foobar/test.vcf" );// resource dependent
  group.writeEntry( "FileFormat", "vcard" );                  // resource dependent

  KRES::Resource *res = factory->resource( "file", group );

  // do something with resource

  \endcode
*/
class KRESOURCES_EXPORT Factory
{
  public:

    /**
      Returns the global resource factory.
    */
    static Factory *self( const QString &resourceFamily );

    ~Factory();

    /**
      Returns the config widget for the given resource type,
      or a null pointer if resource type doesn't exist.

      @param type   The type of the resource, returned by typeNames()
      @param parent The parent widget
    */
    ConfigWidget *configWidget( const QString &type, QWidget *parent = 0 );

    /**
     * Reload the configuration. This reloads the plugin type map.
     * Useful to call after resources have been added or removed.
     * @since 4.2
     */
    void reloadConfig();

    /**
      Returns a pointer to a resource object or a null pointer
      if resource type doesn't exist.

      @param type   The type of the resource, returned by typeNames()
      @param config The configuration group where the resource should
      get its settings from.
    */
    Resource *resource( const QString &type, const KConfigGroup &group );

    /**
      Creates and returns a resource object with default values,
      or a null pointer if resource type doesn't exist.

      @param type   The type of the resource, returned by typeNames()
    */
    Resource *resource( const QString &type );

    /**

      Returns a list of all available resource types.
    */
    QStringList typeNames() const;

    /**
      Returns the name for a special type.
    */
    QString typeName( const QString &type ) const;

    /**
      Returns the description for a special type.
    */
    QString typeDescription( const QString &type ) const;

  protected:
    Factory( const QString &resourceFamily );

  private:
    class Private;
    Private *const d;
};

}
#endif
