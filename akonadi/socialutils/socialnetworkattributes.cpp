/*
  Copyright (C) 2012  Martin Klapetek <martin.klapetek@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "socialnetworkattributes.h"

#include <QString>

#include <qjson/serializer.h>
#include <qjson/parser.h>

Akonadi::SocialNetworkAttributes::SocialNetworkAttributes()
{
}

Akonadi::SocialNetworkAttributes::SocialNetworkAttributes( const QString &userName,
                                                           const QString &networkName,
                                                           bool canPublish,
                                                           uint maxPostLength )
{
  mAttributes[QLatin1String( "userName" )] = userName;
  mAttributes[QLatin1String( "networkName" )] = networkName;
  mAttributes[QLatin1String( "canPublish" )] = canPublish;
  mAttributes[QLatin1String( "maxPostLength" )] = maxPostLength;
}

Akonadi::SocialNetworkAttributes::~SocialNetworkAttributes()
{
}

void Akonadi::SocialNetworkAttributes::deserialize( const QByteArray &data )
{
  QJson::Parser parser;
  mAttributes = parser.parse(data).toMap();
}

QByteArray Akonadi::SocialNetworkAttributes::serialized() const
{
  QJson::Serializer serializer;
  bool ok;						// dummy variable is required
  return serializer.serialize(mAttributes, &ok);
}

Akonadi::Attribute *Akonadi::SocialNetworkAttributes::clone() const
{
  return
    new SocialNetworkAttributes(
      mAttributes[QLatin1String( "userName" )].toString(),
      mAttributes[QLatin1String( "networkName" )].toString(),
      mAttributes[QLatin1String( "canPublish" )].toBool(),
      mAttributes[QLatin1String( "maxPostLength" )].toUInt() );
}

QByteArray Akonadi::SocialNetworkAttributes::type() const
{
  return QByteArray( "socialattributes" );
}

QString Akonadi::SocialNetworkAttributes::userName() const
{
  return mAttributes[QLatin1String( "userName" )].toString();
}

QString Akonadi::SocialNetworkAttributes::networkName() const
{
  return mAttributes[QLatin1String( "networkName" )].toString();
}

bool Akonadi::SocialNetworkAttributes::canPublish() const
{
  return mAttributes[QLatin1String( "canPublish" )].toBool();
}

uint Akonadi::SocialNetworkAttributes::maxPostLength() const
{
  return mAttributes[QLatin1String( "maxPostLength" )].toUInt();
}
