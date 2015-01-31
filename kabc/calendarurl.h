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

#ifndef CALENDARURL_H
#define CALENDARURL_H
#include "kabc_export.h"

#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QMap>

/** @short Class that holds a Calendar Url (FBURL/CALADRURI/CALURI)
 *  @since 4.14.6
 */

namespace KABC {

class KABC_EXPORT CalendarUrl
{
    friend KABC_EXPORT QDataStream &operator<<( QDataStream &, const CalendarUrl & );
    friend KABC_EXPORT QDataStream &operator>>( QDataStream &, CalendarUrl & );
public:
    CalendarUrl();
    CalendarUrl(const CalendarUrl &other);
    CalendarUrl(const QString &mail);

    ~CalendarUrl();

    typedef QList<CalendarUrl> List;

    void setEmail(const QString &mail);
    QString mail() const;

    bool isValid() const;

    void setParameters(const QMap<QString, QStringList> &params);
    QMap<QString, QStringList> parameters() const;

    bool operator==( const CalendarUrl &other ) const;
    bool operator!=( const CalendarUrl &other ) const;

    CalendarUrl &operator=( const CalendarUrl &other );


    QString toString() const;
private:
    class Private;
    QSharedDataPointer<Private> d;
};

KABC_EXPORT QDataStream &operator<<( QDataStream &stream, const CalendarUrl &object );

KABC_EXPORT QDataStream &operator>>( QDataStream &stream, CalendarUrl &object );

}
#endif // CALENDARURL_H
