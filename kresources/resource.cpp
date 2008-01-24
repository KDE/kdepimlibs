/*
    This file is part of libkresources.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include "resource.h"

#include <kdebug.h>
#include <krandom.h>
#include <kconfig.h>
#include <klocale.h>
#include <kconfiggroup.h>

using namespace KRES;

class Resource::ResourcePrivate
{
  public:
#ifdef QT_THREAD_SUPPORT
    QMutex mMutex;
#endif
    int mOpenCount;
    QString mType;
    QString mIdentifier;
    bool mReadOnly;
    QString mName;
    bool mActive;
    bool mIsOpen;
};

/*
Resource::Resource( const KConfig* config )
  : QObject( 0 ), d( new ResourcePrivate )
{
  d->mOpenCount = 0;
  d->mIsOpen = false;

  if ( config ) {
    d->mType = config->readEntry( "ResourceType" );
    d->mName = config->readEntry( "ResourceName" );
    d->mReadOnly = config->readEntry("ResourceIsReadOnly", false);
    d->mActive = config->readEntry("ResourceIsActive", true);
    d->mIdentifier = config->readEntry( "ResourceIdentifier" );
  } else {
    d->mType = "type";
    d->mName = i18n("resource");
    d->mReadOnly = false;
    d->mActive = true;
    d->mIdentifier = KRandom::randomString( 10 );
  }
}
*/

Resource::Resource()
  : QObject( 0 ), d( new ResourcePrivate )
{
  d->mOpenCount = 0;
  d->mIsOpen = false;

  d->mType = "type";
  d->mName = i18n( "resource" );
  d->mReadOnly = false;
  d->mActive = true;
  d->mIdentifier = KRandom::randomString( 10 );
}

Resource::Resource( const KConfigGroup &group )
  : QObject( 0 ), d( new ResourcePrivate )
{
  d->mOpenCount = 0;
  d->mIsOpen = false;

  d->mType = group.readEntry( "ResourceType" );
  d->mName = group.readEntry( "ResourceName" );
  d->mReadOnly = group.readEntry( "ResourceIsReadOnly", false );
  d->mActive = group.readEntry( "ResourceIsActive", true );
  d->mIdentifier = group.readEntry( "ResourceIdentifier" );
}

Resource::~Resource()
{
  delete d;
}

void Resource::writeConfig( KConfigGroup &group )
{
  kDebug(5650) << "Resource::writeConfig()";

  group.writeEntry( "ResourceType", d->mType );
  group.writeEntry( "ResourceName", d->mName );
  group.writeEntry( "ResourceIsReadOnly", d->mReadOnly );
  group.writeEntry( "ResourceIsActive", d->mActive );
  group.writeEntry( "ResourceIdentifier", d->mIdentifier );
}

bool Resource::open()
{
  d->mIsOpen = true;
#ifdef QT_THREAD_SUPPORT
  QMutexLocker guard( &(d->mMutex) );
#endif
  if ( !d->mOpenCount ) {
    kDebug(5650) << "Opening resource" << resourceName();
    d->mIsOpen = doOpen();
  }
  d->mOpenCount++;
  return d->mIsOpen;
}

void Resource::close()
{
#ifdef QT_THREAD_SUPPORT
  QMutexLocker guard( &(d->mMutex) );
#endif
  if ( !d->mOpenCount ) {
    kDebug(5650) << "ERROR: Resource" << resourceName()
                 << "closed more times than previously opened";
    return;
  }
  d->mOpenCount--;
  if ( !d->mOpenCount ) {
    kDebug(5650) << "Closing resource" << resourceName();
    doClose();
    d->mIsOpen = false;
  } else {
    kDebug(5650) << "Not yet closing resource" << resourceName()
                 << ", open count =" << d->mOpenCount;
  }
}

bool Resource::isOpen() const
{
  return d->mIsOpen;
}

void Resource::setIdentifier( const QString &identifier )
{
  d->mIdentifier = identifier;
}

QString Resource::identifier() const
{
  return d->mIdentifier;
}

void Resource::setType( const QString &type )
{
  d->mType = type;
}

QString Resource::type() const
{
  return d->mType;
}

void Resource::setReadOnly( bool value )
{
  d->mReadOnly = value;
}

bool Resource::readOnly() const
{
  return d->mReadOnly;
}

void Resource::setResourceName( const QString &name )
{
  d->mName = name;
}

QString Resource::resourceName() const
{
  return d->mName;
}

void Resource::setActive( bool value )
{
  d->mActive = value;
}

bool Resource::isActive() const
{
  return d->mActive;
}

void Resource::dump() const
{
  kDebug(5650) << "Resource:";
  kDebug(5650) << "  Name:" << d->mName;
  kDebug(5650) << "  Identifier:" << d->mIdentifier;
  kDebug(5650) << "  Type:" << d->mType;
  kDebug(5650) << "  OpenCount:" << d->mOpenCount;
  kDebug(5650) << "  ReadOnly:" << ( d->mReadOnly ? "yes" : "no" );
  kDebug(5650) << "  Active:" << ( d->mActive ? "yes" : "no" );
  kDebug(5650) << "  IsOpen:" << ( d->mIsOpen ? "yes" : "no" );
}

bool Resource::doOpen()
{
  return true;
}

void Resource::doClose()
{
}

QObject *PluginFactoryBase::createObject( QObject *parent,
                                          const char *className,
                                          const QStringList &args )
{
  Q_UNUSED( parent );
  Q_UNUSED( className );
  Q_UNUSED( args );
  return 0;
}

#include "resource.moc"
