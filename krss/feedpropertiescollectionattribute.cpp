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

#include "feedpropertiescollectionattribute.h"
#include "helper_p.h"

#include <QtCore/QStringList>

using namespace KRss;

FeedPropertiesCollectionAttribute::FeedPropertiesCollectionAttribute( )
        : Akonadi::Attribute()
{
}

QByteArray FeedPropertiesCollectionAttribute::type() const
{
    return "FeedProperties";
}

FeedPropertiesCollectionAttribute* FeedPropertiesCollectionAttribute::clone() const
{
    return new FeedPropertiesCollectionAttribute( *this );
}

QByteArray FeedPropertiesCollectionAttribute::serialized() const
{
    return encodeProperties( m_properties );
}

void FeedPropertiesCollectionAttribute::deserialize( const QByteArray &data )
{
    m_properties = decodeProperties( data );
}

void FeedPropertiesCollectionAttribute::setIsFolder( bool isFolder )
{
    m_properties.insert( QLatin1String("IsFolder"), isFolder ? QLatin1String("true") : QString() );
}

bool FeedPropertiesCollectionAttribute::isFolder() const
{
    return !m_properties.value( QLatin1String("IsFolder") ).isEmpty();
}

bool FeedPropertiesCollectionAttribute::preferItemLinkForDisplay() const
{
    return m_properties.value( QLatin1String("PreferItemLinkForDisplay") ) == QLatin1String( "true" );
}

void FeedPropertiesCollectionAttribute::setPreferItemLinkForDisplay( bool b )
{
    m_properties.insert( QLatin1String("PreferItemLinkForDisplay"), b ? QLatin1String("true") : QString() );
}

QString FeedPropertiesCollectionAttribute::xmlUrl() const
{
    return m_properties.value( QLatin1String("XmlUrl" ) );
}

void FeedPropertiesCollectionAttribute::setXmlUrl( const QString &xmlUrl )
{
    m_properties.insert( QLatin1String("XmlUrl" ), xmlUrl );
}

QString FeedPropertiesCollectionAttribute::htmlUrl() const
{
    return m_properties.value( QLatin1String("HtmlUrl" ) );
}

void FeedPropertiesCollectionAttribute::setHtmlUrl( const QString &htmlUrl )
{
    m_properties.insert( QLatin1String("HtmlUrl" ), htmlUrl );
}

QString FeedPropertiesCollectionAttribute::feedType() const
{
    return m_properties.value( QLatin1String("FeedType" ) );
}

void FeedPropertiesCollectionAttribute::setFeedType( const QString &feedType )
{
    m_properties.insert( QLatin1String("FeedType" ), feedType );
}

QString FeedPropertiesCollectionAttribute::description() const
{
    return m_properties.value( QLatin1String("Description" ) );
}

void FeedPropertiesCollectionAttribute::setDescription( const QString &description )
{
    m_properties.insert( QLatin1String("Description" ), description );
}

bool FeedPropertiesCollectionAttribute::fetchError() const
{
    return !m_properties.value("FetchError").isEmpty();
}

void FeedPropertiesCollectionAttribute::setFetchError( bool error )
{
    m_properties.insert( QLatin1String("FetchError"), QLatin1String( error ? "true" : "" ) );
}

QString FeedPropertiesCollectionAttribute::fetchErrorString() const
{
    return m_properties.value( QLatin1String("FetchErrorString") );
}

void FeedPropertiesCollectionAttribute::setFetchErrorString( const QString& fetchErrorString )
{
    m_properties.insert( QLatin1String("FetchErrorString"), fetchErrorString );
}

QString FeedPropertiesCollectionAttribute::imageUrl() const
{
    return m_properties.value( QLatin1String("ImageUrl") );
}

void  FeedPropertiesCollectionAttribute::setImageUrl( const QString& imageUrl )
{
    m_properties.insert( QLatin1String("ImageUrl"), imageUrl );
}

QString FeedPropertiesCollectionAttribute::imageTitle() const
{
    return m_properties.value( QLatin1String("ImageTitle") );
}

void  FeedPropertiesCollectionAttribute::setImageTitle( const QString& imageTitle )
{
    m_properties.insert( QLatin1String("ImageTitle"), imageTitle );
}

QString FeedPropertiesCollectionAttribute::imageLink() const
{
    return m_properties.value( QLatin1String("ImageLink") );
}

void  FeedPropertiesCollectionAttribute::setImageLink( const QString& imageLink )
{
    m_properties.insert( QLatin1String("ImageLink"), imageLink );
}

static const QString CustomFetchIntervalKey = QLatin1String("CustomFetchIntervalKey");

int FeedPropertiesCollectionAttribute::customFetchInterval() const
{
    return readIntProperty( CustomFetchIntervalKey, -1 );
}

void FeedPropertiesCollectionAttribute::setCustomFetchInterval( int interval )
{
    setProperty( CustomFetchIntervalKey, QString::number( interval ), QLatin1String("-1") );
}

static const QString MaximumItemNumberKey = QLatin1String("MaximumItemNumber");

int FeedPropertiesCollectionAttribute::maximumItemNumber() const
{
    return readIntProperty( MaximumItemNumberKey, -1 );
}

void FeedPropertiesCollectionAttribute::setMaximumItemNumber( int mv )
{
    setProperty( MaximumItemNumberKey, QString::number( mv ), QLatin1String("-1") );
}

static const QString MaximumItemAgeKey = QLatin1String("MaximumItemAge");

int FeedPropertiesCollectionAttribute::maximumItemAge() const
{
    return readIntProperty( MaximumItemAgeKey, -1 );
}

void FeedPropertiesCollectionAttribute::setMaximumItemAge( int ma )
{
    setProperty( MaximumItemAgeKey, QString::number( ma ), QLatin1String("-1") );
}

static QString ArchiveModeKey = QLatin1String("ArchiveModeKey");

FeedPropertiesCollectionAttribute::ArchiveMode FeedPropertiesCollectionAttribute::archiveMode() const
{
    const QString str = m_properties.value( ArchiveModeKey );
    if (str == "keepAllItems")
        return KeepAllItems;
    else if (str == "disableArchiving")
        return DisableArchiving;
    else if (str == "limitItemNumber")
        return LimitItemNumber;
    else if (str == "limitItemAge")
        return LimitItemAge;
    else
        return GlobalDefault;
}

void FeedPropertiesCollectionAttribute::setProperty( const QString& key, const QString& value, const QString& defaultValue )
{
    if ( value == defaultValue )
        m_properties.remove( key );
    else
        m_properties.insert( key, value );
}

int FeedPropertiesCollectionAttribute::readIntProperty( const QString& key, int defaultValue ) const
{
    const QString s = m_properties.value( key );
    bool ok = false;
    const int v = s.toInt( &ok );
    return ok ? v : defaultValue;
}

void FeedPropertiesCollectionAttribute::setArchiveMode( ArchiveMode mode )
{
    QString s;
    switch ( mode ) {
    case GlobalDefault:
        break;
    case KeepAllItems:
        s = QLatin1String("keepAllItems");
        break;
    case DisableArchiving:
        s = QLatin1String("disableArchiving");
        break;
    case LimitItemNumber:
        s = QLatin1String("limitItemNumber");
        break;
    case LimitItemAge:
        s = QLatin1String("limitItemAge");
    }

    setProperty( ArchiveModeKey, s, QString() );
}
