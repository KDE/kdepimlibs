/*
    Copyright (C) 2009 Omat Holding B.V. <info@omat.nl>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "statusitem.h"
#include <qdebug.h>

#include <QDateTime>
#include <QDomElement>
#include <QHash>
#include <QStringList>

#include <linklocator.h>

using namespace Microblog;

class StatusItem::Private : public QSharedData
{
public:
    Private() {}
    Private( const Private& other ) : QSharedData( other ) {
        data = other.data;
        status = other.status;
        dateTime = other.dateTime;
    }

public:
    void init();
    QByteArray data;
    QHash<QString, QString> status;
    QDateTime dateTime;
};

void StatusItem::Private::init()
{
    QDomDocument document;
    document.setContent( data );
    QDomElement root = document.documentElement();
    QDomNode node = root.firstChild();
    while ( !node.isNull() ) {
        const QString key = node.toElement().tagName();
        if ( key == QLatin1String("user") || key == QLatin1String("sender") || key == QLatin1String("recipient") ) {
            QDomNode node2 = node.firstChild();
            while ( !node2.isNull() ) {
                const QString key2 = node2.toElement().tagName();
                const QString val2 = node2.toElement().text();
                status[ key + QLatin1String("_-_") + key2 ] = val2;
                node2 = node2.nextSibling();
            }
        } else {
            const QString value = node.toElement().text();
            status[key] = value;
        }
        node = node.nextSibling();
    }
    //qDebug() << status;

    dateTime = QDateTime::fromString( status.value( QLatin1String("created_at") ).toLower().mid( 4 ),
                                      QLatin1String("MMM dd H:mm:ss +0000 yyyy") );
    dateTime.setTimeSpec( Qt::UTC );
    dateTime = dateTime.toLocalTime();

    if ( !dateTime.isValid() ) {
        qDebug() << "Unable to parse" << status.value( QLatin1String("created_at") ).toLower().mid( 4 );
    }
    //qDebug() << dateTime;
}

StatusItem::StatusItem()  :  d( new Private )
{
}

StatusItem::StatusItem( const QByteArray &data ) : d( new Private )
{
    d->data = data;
    d->init();
}

StatusItem::StatusItem( const StatusItem& other ) : d( other.d )
{
}

StatusItem::~StatusItem()
{
}

StatusItem StatusItem::operator=( const StatusItem &other )
{
    if ( &other != this ) {
        d = other.d;
    }

    return *this;
}

void StatusItem::setData( const QByteArray &data )
{
    d->data = data;
    d->init();
}


qlonglong StatusItem::id() const
{
    return d->status.value( QLatin1String("id") ).toLongLong();
}

QByteArray StatusItem::data() const
{
    return d->data;
}

QString StatusItem::value( const QString& value ) const
{
    return d->status.value( value );
}

QStringList StatusItem::keys() const
{
    return d->status.keys();
}

QString StatusItem::text() const
{
    using KPIMUtils::LinkLocator;
    int flags = LinkLocator::PreserveSpaces | LinkLocator::HighlightText | LinkLocator::ReplaceSmileys;
    return KPIMUtils::LinkLocator::convertToHtml( d->status.value( QLatin1String("text") ), flags );
}

QDateTime StatusItem::date() const
{
    return d->dateTime;
}
