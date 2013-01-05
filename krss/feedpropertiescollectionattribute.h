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

#ifndef KRSS_FEEDPROPERTIESCOLLECTIONATTRIBUTE_H
#define KRSS_FEEDPROPERTIESCOLLECTIONATTRIBUTE_H

#include "krss_export.h"

#include <akonadi/attribute.h>

#include <QtCore/QString>
#include <QtCore/QHash>

namespace KRss {

class KRSS_EXPORT FeedPropertiesCollectionAttribute : public Akonadi::Attribute
{
public:
        FeedPropertiesCollectionAttribute();
        QByteArray type() const;
        FeedPropertiesCollectionAttribute* clone() const;
        QByteArray serialized() const;
        void deserialize( const QByteArray &data );

        bool isFolder() const;
        void setIsFolder( bool isFolder );
        QString xmlUrl() const;
        void setXmlUrl( const QString &xmlUrl );
        QString htmlUrl() const;
        void setHtmlUrl( const QString &htmlUrl );
        QString feedType() const;
        void setFeedType( const QString &feedType );
        QString description() const;
        void setDescription( const QString &description );
        bool preferItemLinkForDisplay() const;
        void setPreferItemLinkForDisplay( bool b );
        bool fetchError() const;
        void setFetchError( bool error );
        QString fetchErrorString() const;
        void setFetchErrorString( const QString& fetchErrorString );
        QString imageUrl() const;
        void setImageUrl( const QString& imageUrl );
        QString imageTitle() const;
        void setImageTitle( const QString& imageTitle );
        QString imageLink() const;
        void setImageLink( const QString& imageLink );
        /**
         * returns the custom fetch interval in minutes, or -1 if none is set
         */
        int customFetchInterval() const;
        void setCustomFetchInterval( int );

        enum ArchiveMode {
            GlobalDefault,
            KeepAllItems,
            DisableArchiving,
            LimitItemNumber,
            LimitItemAge
        };

        ArchiveMode archiveMode() const;
        void setArchiveMode( ArchiveMode mode );

        int maximumItemNumber() const;
        void setMaximumItemNumber( int );

        int maximumItemAge() const;
        void setMaximumItemAge( int );

private:
        void setProperty( const QString& key, const QString& value, const QString& defaultValue );
        int readIntProperty( const QString& key, int defaultValue ) const;

private:
        QHash<QString,QString> m_properties;
};

} // namespace KRss

#endif // KRSS_FEEDPROPERTIESCOLLECTIONATTRIBUTE_H
