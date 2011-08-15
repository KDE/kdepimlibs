/*  -*- c++ -*-
    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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

#include "pluginloader_p.h"

#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <KPluginLoader>

#include <QtCore/QDebug>

#ifdef Q_OS_WINCE
#include <KMessageBox>
#endif

using namespace Akonadi;

PluginMetaData::PluginMetaData()
{
}

PluginMetaData::PluginMetaData( const QString & lib, const QString & name, const QString & comment, const QString & cname )
  : library( lib ), nameLabel( name ),
    descriptionLabel( comment ),
    className(cname), loaded( false )
{
}


PluginLoader* PluginLoader::mSelf = 0;

PluginLoader::PluginLoader()
{
  scan();
}

PluginLoader::~PluginLoader()
{
  qDeleteAll( mPluginLoaders );
  mPluginLoaders.clear();
}

PluginLoader* PluginLoader::self()
{
  if ( !mSelf )
    mSelf = new PluginLoader();

  return mSelf;
}

QStringList PluginLoader::names() const
{
  return mPluginInfos.keys();
}

QObject* PluginLoader::createForName( const QString & name )
{
  if ( !mPluginInfos.contains( name ) ) {
    kWarning( 5300 ) << "plugin name \"" << name << "\" is unknown to the plugin loader." << endl;
    return 0;
  }

  PluginMetaData &info = mPluginInfos[ name ];

  //First try to load it staticly
  foreach (QObject *plugin, QPluginLoader::staticInstances()) {
    if(QLatin1String(plugin->metaObject()->className()) == info.className) {
      info.loaded = true;
      return plugin;
      break;
    }
  }

  if ( !info.loaded ) {
    KPluginLoader* loader = new KPluginLoader( info.library );
    if ( loader->fileName().isEmpty() ) {
      kWarning( 5300 ) << loader->errorString();
      delete loader;
      return 0;
    }

    mPluginLoaders.insert( name, loader );
    info.loaded = true;
  }

  QPluginLoader *loader = mPluginLoaders.value( name );
  Q_ASSERT(loader);

  QObject *object = loader->instance();
  if ( !object ) {
#ifdef Q_OS_WINCE
    //Maybe filter out the default plugins, they should be found but...
    //if ( !name.endsWith( QLatin1String( "@default" ) ) ) {
      QString errMessage =
        i18n( "Plugin \"%1\" is not builtin static, "
              "please specify this information in the bug report.", info.className );
      KMessageBox::critical( 0, i18n( "Plugin Not Built Statically" ), errMessage );
    //}
#endif
    kWarning( 5300 ) << "unable to load plugin for plugin name \"" << name << "\"." << endl;
    kWarning( 5300 ) << "Error was:\"" << loader->errorString() << "\"." << endl;
    return 0;
  }

  return object;
}

PluginMetaData PluginLoader::infoForName( const QString & name ) const
{
  if ( !mPluginInfos.contains( name ) )
    return PluginMetaData();

  return mPluginInfos.value( name );
}

void PluginLoader::scan()
{
  const QStringList list = KGlobal::dirs()->findAllResources( "data", QLatin1String( "akonadi/plugins/serializer/*.desktop" ),
                                                              KStandardDirs::Recursive | KStandardDirs::NoDuplicates );
  for ( int i = 0; i < list.count(); ++i ) {
    const QString entry = list.at( i );

    KConfig config( entry, KConfig::SimpleConfig );
    if ( config.hasGroup( "Misc" ) && config.hasGroup( "Plugin" ) ) {
      KConfigGroup group( &config, "Plugin" );

      const QString type = group.readEntry( "Type" ).toLower();
      if ( type.isEmpty() ) {
        kWarning( 5300 ) << "missing or empty [Plugin]Type value in \"" << entry << "\" - skipping" << endl;
        continue;
      }

      // read Class entry as a list so that types like QPair<A,B> are
      // properly escaped and don't end up being split into QPair<A
      // and B>.
      const QStringList classes = group.readXdgListEntry( "X-Akonadi-Class" );
      if ( classes.isEmpty() ) {
        kWarning( 5300 ) << "missing or empty [Plugin]X-Akonadi-Class value in \"" << entry << "\" - skipping" << endl;
        continue;
      }

      const QString library = group.readEntry( "X-KDE-Library" );
      if ( library.isEmpty() ) {
        kWarning( 5300 ) << "missing or empty [Plugin]X-KDE-Library value in \"" << entry << "\" - skipping" << endl;
        continue;
      }

      KConfigGroup group2( &config, "Misc" );

      QString name = group2.readEntry( "Name" );
      if ( name.isEmpty() ) {
        kWarning( 5300 ) << "missing or empty [Misc]Name value in \"" << entry << "\" - inserting default name" << endl;
        name = i18n( "Unnamed plugin" );
      }

      QString comment = group2.readEntry( "Comment" );
      if ( comment.isEmpty() ) {
        kWarning( 5300 ) << "missing or empty [Misc]Comment value in \"" << entry << "\" - inserting default name" << endl;
        comment = i18n( "No description available" );
      }

      QString cname      = group.readEntry( "X-KDE-ClassName" );
      if ( cname.isEmpty() ) {
        kWarning( 5300 ) << "missing or empty X-KDE-ClassName value in \"" << entry << "\"" << endl;
      }

      const QStringList mimeTypes = type.split( QLatin1Char( ',' ), QString::SkipEmptyParts );

      kDebug( 5300 ) << "registering Desktop file" << entry << "for" << mimeTypes << '@' << classes;
      Q_FOREACH( const QString & mimeType, mimeTypes )
        Q_FOREACH( const QString & classType, classes )
          mPluginInfos.insert( mimeType + QLatin1Char('@') + classType, PluginMetaData( library, name, comment, cname ) );

    } else {
      kWarning( 5300 ) << "Desktop file \"" << entry << "\" doesn't seem to describe a plugin " << "(misses Misc and/or Plugin group)" << endl;
    }
  }
}
