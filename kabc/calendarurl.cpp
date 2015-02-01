/*
    This file is part of libkabc.
    Copyright (c) 2015 Laurent Montel <montel@kde.org>

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

#include "calendarurl.h"

#include <QMap>
#include <qstringlist.h>

using namespace KABC;

class CalendarUrl::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private( const Private &other )
        : QSharedData( other )
    {
        parameters = other.parameters;
        type = other.type;
    }
    QMap<QString, QStringList> parameters;
    CalendarUrl::CalendarType type;
};

CalendarUrl::CalendarUrl()
    : d( new Private )
{

}

CalendarUrl::CalendarUrl( const CalendarUrl &other )
  : d( other.d )
{
}

CalendarUrl::~CalendarUrl()
{

}

QMap<QString, QStringList> CalendarUrl::parameters() const
{
    return d->parameters;
}

bool CalendarUrl::operator==(const CalendarUrl &other) const
{
    return (d->parameters == other.parameters()) && (d->type == other.type());
}

bool CalendarUrl::operator!=(const CalendarUrl &other) const
{
    return !( other == *this );
}

CalendarUrl &CalendarUrl::operator=(const CalendarUrl &other)
{
    if ( this != &other ) {
      d = other.d;
    }

    return *this;
}

QString CalendarUrl::toString() const
{
    QString str;
    str += QString::fromLatin1( "CalendarUrl {\n" );
    //TODO str += QString::fromLatin1( "    mail: %1\n" ).arg( d->mail );
    if (!d->parameters.isEmpty()) {
        QMapIterator<QString, QStringList> i(d->parameters);
        QString param;
        while (i.hasNext()) {
            i.next();
            param += QString::fromLatin1("%1 %2").arg(i.key()).arg(i.value().join(QLatin1String(",")));
        }
        str += QString::fromLatin1( "    parameters: %1\n" ).arg( param );
    }
    str += QString::fromLatin1( "}\n" );
    return str;
}

void CalendarUrl::setParameters(const QMap<QString, QStringList> &params)
{
    d->parameters = params;
}

bool CalendarUrl::isValid() const
{
    //TODO
    return /*!d->mail.isEmpty()*/true;
}

void CalendarUrl::setType(CalendarUrl::CalendarType type)
{
    d->type = type;
}

CalendarUrl::CalendarType CalendarUrl::type() const
{
    return d->type;
}

QDataStream &KABC::operator<<(QDataStream &s, const CalendarUrl &calUrl)
{
    return s << calUrl.d->parameters << calUrl.d->type;
}

QDataStream &KABC::operator>>(QDataStream &s, CalendarUrl &calUrl)
{
    s >> calUrl.d->parameters >> calUrl.d->type;
    return s;
}

