/*
  This file is part of the kcal library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001-2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "resourcecalendar.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "resourcecalendar.moc"

using namespace KCal;

//@cond PRIVATE
class ResourceCalendar::Private
{
  public:
    Private()
      : mResolveConflict( false ),
        mNoReadOnlyOnLoad( false ),
        mInhibitSave( false )
    {}
    bool mResolveConflict;
    bool mNoReadOnlyOnLoad;
    bool mInhibitSave;     // true to prevent saves
    bool mReceivedLoadError;
    bool mReceivedSaveError;
    QString mLastError;

};
//@endcond

ResourceCalendar::ResourceCalendar()
  : KRES::Resource(), d( new Private )
{
}

ResourceCalendar::ResourceCalendar( const KConfigGroup &group )
  : KRES::Resource( group ),
    d( new Private )
{
}

ResourceCalendar::~ResourceCalendar()
{
  delete d;
}

bool ResourceCalendar::isResolveConflictSet() const
{
  return d->mResolveConflict;
}

void ResourceCalendar::setResolveConflict( bool b )
{
  d->mResolveConflict = b;
}

QString ResourceCalendar::infoText() const
{
  QString txt;

  txt += "<b>" + resourceName() + "</b>";
  txt += "<br>";

  KRES::Factory *factory = KRES::Factory::self( "calendar" );
  QString t = factory->typeName( type() );
  txt += i18n( "Type: %1", t );

  addInfoText( txt );

  return txt;
}

void ResourceCalendar::writeConfig( KConfigGroup &group )
{
  KRES::Resource::writeConfig( group );
}

Incidence *ResourceCalendar::incidence( const QString &uid )
{
  Incidence *i = event( uid );
  if ( i ) {
    return i;
  }

  i = todo( uid );
  if ( i ) {
    return i;
  }

  i = journal( uid );
  return i;
}

bool ResourceCalendar::addIncidence( Incidence *incidence )
{
  Incidence::AddVisitor<ResourceCalendar> v( this );
  return incidence->accept( v );
}

bool ResourceCalendar::deleteIncidence( Incidence *incidence )
{
  Incidence::DeleteVisitor<ResourceCalendar> v( this );
  return incidence->accept( v );
}

Incidence::List ResourceCalendar::rawIncidences()
{
  return Calendar::mergeIncidenceList( rawEvents(), rawTodos(), rawJournals() );
}

void ResourceCalendar::setSubresourceActive( const QString &, bool )
{
}

bool ResourceCalendar::removeSubresource( const QString &resource )
{
    Q_UNUSED( resource )
    return true;
}

bool ResourceCalendar::addSubresource( const QString &resource, const QString &parent )
{
    Q_UNUSED( resource )
    Q_UNUSED( parent )
    return true;
}

QString ResourceCalendar::subresourceType( const QString &resource )
{
    Q_UNUSED( resource )
    return QString();
}

bool ResourceCalendar::load()
{
  kDebug() << resourceName();

  d->mReceivedLoadError = false;

  bool success = true;
  if ( !isOpen() ) {
    success = open(); //krazy:exclude=syscalls open is a class method
  }
  if ( success ) {
    success = doLoad( false );
  }
  if ( !success && !d->mReceivedLoadError ) {
    loadError();
  }

  // If the resource is read-only, we need to set its incidences to read-only,
  // too. This can't be done at a lower-level, since the read-only setting
  // happens at this level
  if ( !d->mNoReadOnlyOnLoad && readOnly() ) {
    Incidence::List incidences( rawIncidences() );
    Incidence::List::Iterator it;
    for ( it = incidences.begin(); it != incidences.end(); ++it ) {
      (*it)->setReadOnly( true );
    }
  }

  kDebug() << "Done loading resource" << resourceName();

  return success;
}

void ResourceCalendar::loadError( const QString &err )
{
  kDebug() << "Error loading resource:" << err;

  d->mReceivedLoadError = true;

  QString msg = i18n( "Error while loading %1.\n", resourceName() );
  if ( !err.isEmpty() ) {
    msg += err;
  }
  emit resourceLoadError( this, msg );
}

bool ResourceCalendar::receivedLoadError() const
{
  return d->mReceivedLoadError;
}

void ResourceCalendar::setReceivedLoadError( bool b )
{
  d->mReceivedLoadError = b;
}

bool ResourceCalendar::save( Incidence *incidence )
{
  if ( d->mInhibitSave ) {
    return true;
  }

  if ( !readOnly() ) {
    kDebug() << resourceName();

    d->mReceivedSaveError = false;

    if ( !isOpen() ) {
      kDebug() << "Trying to save into a closed resource" << resourceName();
      return true;
    }
    bool success = incidence ? doSave( false, incidence ) : doSave( false );
    if ( !success && !d->mReceivedSaveError ) {
      saveError();
    }
    return success;
  } else {
    // Read-only, just don't save...
    kDebug() << "Don't save read-only resource" << resourceName();
    return true;
  }
}

bool ResourceCalendar::save( QString &err, Incidence *incidence )
{
  d->mLastError.clear();
  bool ret = save( incidence ); // a new mLastError may be set in here
  err = d->mLastError;
  return ret;
}

bool ResourceCalendar::isSaving()
{
  return false;
}

bool ResourceCalendar::doSave( bool syncCache, Incidence *incidence )
{
  Q_UNUSED( incidence );
  return doSave( syncCache );
}

void ResourceCalendar::saveError( const QString &err )
{
  kDebug() << "Error saving resource:" << err;

  d->mReceivedSaveError = true;
  QString msg = i18n( "Error while saving %1.\n", resourceName() );
  if ( !err.isEmpty() ) {
    msg += err;
  }
  d->mLastError = err;
  emit resourceSaveError( this, msg );
}

QStringList ResourceCalendar::subresources() const
{
  return QStringList();
}

bool ResourceCalendar::canHaveSubresources() const
{
  return false;
}

bool ResourceCalendar::subresourceActive( const QString &resource ) const
{
  Q_UNUSED( resource );
  return true;
}

QString ResourceCalendar::labelForSubresource( const QString &resource ) const
{
  // the resource identifier is a sane fallback
  return resource;
}

QString ResourceCalendar::subresourceIdentifier( Incidence *incidence )
{
  Q_UNUSED( incidence );
  return QString();
}

bool ResourceCalendar::receivedSaveError() const
{
  return d->mReceivedSaveError;
}

void ResourceCalendar::setReceivedSaveError( bool b )
{
  d->mReceivedSaveError = b;
}

void ResourceCalendar::setInhibitSave( bool inhibit )
{
  d->mInhibitSave = inhibit;
}

bool ResourceCalendar::saveInhibited() const
{
  return d->mInhibitSave;
}

bool ResourceCalendar::setValue( const QString &key, const QString &value )
{
  Q_UNUSED( key );
  Q_UNUSED( value );
  return false;
}

void ResourceCalendar::setNoReadOnlyOnLoad( bool noReadOnly )
{
  d->mNoReadOnlyOnLoad = noReadOnly;
}

bool ResourceCalendar::noReadOnlyOnLoad() const
{
  return d->mNoReadOnlyOnLoad;
}
