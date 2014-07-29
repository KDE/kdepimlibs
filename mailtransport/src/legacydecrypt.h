/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    KNode code:
    Copyright (c) 1999-2005 the KNode authors. // krazy:exclude=copyright

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

#ifndef MAILTRANSPORT_LEGACYDECRYPT_H
#define MAILTRANSPORT_LEGACYDECRYPT_H

#include <QtCore/QString>

namespace MailTransport
{

/**
  Methods to read passwords from config files still using legacy encryption.
*/
class Legacy
{
public:
    /**
      Read data encrypted using KMail's legacy encryption.
    */
    static QString decryptKMail(const QString &data);

    /**
      Read data encrypted using KNode's legacy encryption.
    */
    static QString decryptKNode(const QString &data);
};

} // namespace MailTransport

#endif // MAILTRANSPORT_LEGACYDECRYPT_H
