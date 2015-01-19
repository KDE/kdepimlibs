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

#ifndef LANG_H
#define LANG_H

#include "kabc_export.h"
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QMap>

/** @short Class that holds a Language for a contact.
 *  @since 4.14.5
 */
namespace KABC {
class KABC_EXPORT Lang
{
    friend KABC_EXPORT QDataStream &operator<<( QDataStream &, const Lang & );
    friend KABC_EXPORT QDataStream &operator>>( QDataStream &, Lang & );
public:
    Lang();
    Lang(const Lang &other);
    Lang(const QString &mail);

    ~Lang();

    typedef QList<Lang> List;

    void setLanguage(const QString &lang);
    QString language() const;

    bool isValid() const;

    void setParameters(const QMap<QString, QStringList> &params);
    QMap<QString, QStringList> parameters() const;

    bool operator==( const Lang &other ) const;
    bool operator!=( const Lang &other ) const;

    Lang &operator=( const Lang &other );

    QString toString() const;
private:
    class Private;
    QSharedDataPointer<Private> d;
};
KABC_EXPORT QDataStream &operator<<( QDataStream &stream, const Lang &object );

KABC_EXPORT QDataStream &operator>>( QDataStream &stream, Lang &object );

}

#endif // LANG_H
