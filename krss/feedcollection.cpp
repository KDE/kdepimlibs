/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "feedcollection.h"
#include "feedpropertiescollectionattribute.h"

#include <akonadi/attributefactory.h>
#include <akonadi/cachepolicy.h>
#include <akonadi/entitydisplayattribute.h>
using namespace Akonadi;
using namespace KRss;


FeedCollection::FeedCollection()
{
}

void FeedCollection::registerAttributes() {
    Akonadi::AttributeFactory::registerAttribute<KRss::FeedPropertiesCollectionAttribute>();
}

FeedCollection::FeedCollection( const Akonadi::Collection &collection )
    : Collection( collection )
{
}

Akonadi::Collection FeedCollection::findFolder( const Akonadi::Collection& c ) {
    if ( FeedCollection( c ).isFolder() )
        return c;
    else
        return c.parentCollection();
}

bool FeedCollection::isFolder() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->isFolder();
    return false;
}

void FeedCollection::setIsFolder( bool isFolder )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setIsFolder( isFolder );
}

QString FeedCollection::xmlUrl() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->xmlUrl();

    return QString();
}

void FeedCollection::setXmlUrl( const QString &xmlUrl )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setXmlUrl( xmlUrl );
}

QString FeedCollection::title() const {
    EntityDisplayAttribute* attr = attribute<EntityDisplayAttribute>();
    if ( attr )
        return attr->displayName();
    else
        return QString();
}

void FeedCollection::setTitle( const QString& t ) {
    attribute<EntityDisplayAttribute>( AddIfMissing )->setDisplayName( t );
}

QString FeedCollection::htmlUrl() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->htmlUrl();

    return QString();
}

void FeedCollection::setHtmlUrl( const QString &htmlUrl )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setHtmlUrl( htmlUrl );
}

QString FeedCollection::description() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->description();

    return QString();
}

void FeedCollection::setDescription( const QString &description )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setDescription( description );
}


QString FeedCollection::imageUrl() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->imageUrl();
    else
      return QString();
}

void FeedCollection::setImageUrl( const QString& imageUrl )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setImageUrl( imageUrl );
}


QString FeedCollection::imageLink() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->imageLink();
    else
      return QString();
}

void FeedCollection::setImageLink( const QString& imageLink )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setImageLink( imageLink );
}


QString FeedCollection::imageTitle() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->imageTitle();
    else
      return QString();
}

void FeedCollection::setImageTitle( const QString& imageTitle )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setImageTitle( imageTitle );
}

QString FeedCollection::feedType() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->feedType();

    return QString();
}

void FeedCollection::setFeedType( const QString &feedType )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setFeedType( feedType );
}

bool FeedCollection::preferItemLinkForDisplay() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->preferItemLinkForDisplay();

    return false;
}

void FeedCollection::setPreferItemLinkForDisplay( bool b )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setPreferItemLinkForDisplay( b );
}

int FeedCollection::fetchInterval() const
{
    const FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->customFetchInterval();
    else
        return -1;
}

void FeedCollection::setFetchInterval( int interval )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setCustomFetchInterval( interval );
}

bool FeedCollection::fetchError() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->fetchError();
    else
        return false;
}

void FeedCollection::setFetchError( bool hasError )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setFetchError( hasError );
}

QString FeedCollection::fetchErrorString() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->fetchErrorString();
    else
        return QString();
}

void FeedCollection::setFetchErrorString( const QString& errorString )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setFetchErrorString( errorString );
}

FeedCollection::ArchiveMode FeedCollection::archiveMode() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    const FeedPropertiesCollectionAttribute::ArchiveMode am = attr ? attr->archiveMode() : FeedPropertiesCollectionAttribute::GlobalDefault;
    switch ( am ) {
    case FeedPropertiesCollectionAttribute::GlobalDefault:
        return GlobalDefault;
    case FeedPropertiesCollectionAttribute::KeepAllItems:
        return KeepAllItems;
    case FeedPropertiesCollectionAttribute::DisableArchiving:
        return DisableArchiving;
    case FeedPropertiesCollectionAttribute::LimitItemAge:
        return LimitItemAge;
    case FeedPropertiesCollectionAttribute::LimitItemNumber:
        return LimitItemNumber;
    }
    Q_ASSERT( !"unhandled archive mode" );
    return GlobalDefault;
}

void FeedCollection::setArchiveMode( ArchiveMode mode )
{
    FeedPropertiesCollectionAttribute::ArchiveMode am = FeedPropertiesCollectionAttribute::GlobalDefault;
    switch ( mode ) {
    case GlobalDefault:
        am = FeedPropertiesCollectionAttribute::GlobalDefault;
        break;
    case KeepAllItems:
        am = FeedPropertiesCollectionAttribute::KeepAllItems;
        break;
    case DisableArchiving:
        am = FeedPropertiesCollectionAttribute::DisableArchiving;
        break;
    case LimitItemAge:
        am = FeedPropertiesCollectionAttribute::LimitItemAge;
        break;
    case LimitItemNumber:
        am = FeedPropertiesCollectionAttribute::LimitItemNumber;
        break;
    }
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setArchiveMode( am );
}

int FeedCollection::maximumItemNumber() const
{
    const FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    return attr ? attr->maximumItemNumber() : -1;
}

void FeedCollection::setMaximumItemNumber( int m )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setMaximumItemNumber( m );
}

int FeedCollection::maximumItemAge() const
{
    const FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    return attr ? attr->maximumItemAge() : -1;
}

void FeedCollection::setMaximumItemAge( int m )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setMaximumItemAge( m );
}

bool FeedCollection::allowSubfolders() const
{
  const FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    return attr ? attr->allowSubfolders() : true;
}

void FeedCollection::setAllowSubfolders( bool allow )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setAllowSubfolders( allow );
}
