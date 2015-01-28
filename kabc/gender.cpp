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

#include "gender.h"

#include <QMap>
#include <qstringlist.h>

using namespace KABC;

class Gender::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private( const Private &other )
        : QSharedData( other )
    {
        parameters = other.parameters;
        gender = other.gender;
    }
    QMap<QString, QStringList> parameters;
    QString gender;
};

Gender::Gender()
    : d( new Private )
{

}

Gender::Gender(const QString &gender)
    : d( new Private )
{
    d->gender = gender;
}

Gender::Gender( const Gender &other )
  : d( other.d )
{
}

Gender::~Gender()
{

}

QMap<QString, QStringList> Gender::parameters() const
{
    return d->parameters;
}

bool Gender::operator==(const Gender &other) const
{
    return (d->parameters == other.parameters()) && (d->gender == other.gender());
}

bool Gender::operator!=(const Gender &other) const
{
    return !( other == *this );
}

Gender &Gender::operator=(const Gender &other)
{
    if ( this != &other ) {
      d = other.d;
    }

    return *this;
}

QString Gender::toString() const
{
    QString str;
    str += QString::fromLatin1( "Gender {\n" );
    str += QString::fromLatin1( "    gender: %1\n" ).arg( d->gender );
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

void Gender::setParameters(const QMap<QString, QStringList> &params)
{
    d->parameters = params;
}

void Gender::setGender(const QString &gender)
{
    d->gender = gender;
}

QString Gender::gender() const
{
    return d->gender;
}

bool Gender::isValid() const
{
  return !d->gender.isEmpty();
}

QDataStream &KABC::operator<<(QDataStream &s, const Gender &gender)
{
    return s << gender.d->parameters << gender.d->gender;
}

QDataStream &KABC::operator>>(QDataStream &s, Gender &gender)
{
    s >> gender.d->parameters >> gender.d->gender;
    return s;
}

