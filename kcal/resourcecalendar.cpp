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

#include "resourcecalendar.moc"

#include "calendar.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

using namespace KCal;

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

};


ResourceCalendar::ResourceCalendar()
  : KRES::Resource(),
    d( new Private )
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

void ResourceCalendar::setResolveConflict( bool b)
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
  txt += i18n("Type: %1", t );

  addInfoText( txt );

  return txt;
}

void ResourceCalendar::writeConfig( KConfigGroup &group )
{
//  kDebug(5800) << "ResourceCalendar::writeConfig()";

  KRES::Resource::writeConfig( group );
}

Incidence *ResourceCalendar::incidence( const QString &uid )
{
  Incidence *i = event( uid );
  if ( i ) return i;
  i = todo( uid );
  if ( i ) return i;
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


/*virtual*/
bool ResourceCalendar::removeSubresource( const QString& resource )
{
    Q_UNUSED(resource)
    return true;
}

/*virtual*/
bool ResourceCalendar::addSubresource( const QString& resource, const QString& parent )
{
    Q_UNUSED(resource)
    Q_UNUSED(parent)
    return true;
}

QString ResourceCalendar::subresourceType( const QString& resource )
{
    Q_UNUSED(resource)
    return QString();
}

bool ResourceCalendar::load()
{
  kDebug(5800) << "Loading resource" << resourceName();

  d->mReceivedLoadError = false;

  bool success = true;
  if ( !isOpen() ) success = open();
  if ( success ) {
    success = doLoad();
  }
  if ( !success && !d->mReceivedLoadError ) loadError();

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

  kDebug(5800) << "Done loading resource" << resourceName();

  return success;
}

void ResourceCalendar::loadError( const QString &err )
{
  kDebug(5800) << "Error loading resource:" << err;

  d->mReceivedLoadError = true;

  QString msg = i18n("Error while loading %1.\n", resourceName() );
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
  if ( d->mInhibitSave )
    return true;
  if ( !readOnly() ) {
    kDebug(5800) << "Save resource" << resourceName();

    d->mReceivedSaveError = false;

    if ( !isOpen() ) return true;
    bool success = incidence ? doSave(incidence) : doSave();
    if ( !success && !d->mReceivedSaveError ) saveError();

    return success;
  } else {
    // Read-only, just don't save...
    kDebug(5800) << "Don't save read-only resource" << resourceName();
    return true;
  }
}

bool ResourceCalendar::doSave( Incidence * )
{
  return doSave();
}

void ResourceCalendar::saveError( const QString &err )
{
  kDebug(5800) << "Error saving resource:" << err;

  d->mReceivedSaveError = true;

  QString msg = i18n("Error while saving %1.\n", resourceName() );
  if ( !err.isEmpty() ) {
    msg += err;
  }
  emit resourceSaveError( this, msg );
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

void ResourceCalendar::setNoReadOnlyOnLoad(bool noReadOnly)
{
  d->mNoReadOnlyOnLoad = noReadOnly;
}

bool ResourceCalendar::noReadOnlyOnLoad() const
{
  return d->mNoReadOnlyOnLoad;
}
