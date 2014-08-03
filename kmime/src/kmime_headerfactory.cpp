/*
    kmime_header_factory.cpp

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

/**
  @file
  This file is part of the API for handling MIME data and
  defines the HeaderFactory class.

  @brief
  Defines the HeaderFactory class.

  @authors Constantin Berzan \<exit3219@gmail.com\>
*/

#include "kmime_headerfactory_p.h"
#include "kmime_headers.h"

#include <QHash>

#include <QDebug>

using namespace KMime;

/**
 * @internal
 * Private class that helps to provide binary compatibility between releases.
 */
class KMime::HeaderFactoryPrivate
{
public:
    HeaderFactoryPrivate();
    ~HeaderFactoryPrivate();

    HeaderFactory *const instance;
    QHash<QByteArray, HeaderMakerBase *> headerMakers; // Type->obj mapping; with lower-case type.
};

Q_GLOBAL_STATIC(HeaderFactoryPrivate, sInstance)

HeaderFactoryPrivate::HeaderFactoryPrivate()
    : instance(new HeaderFactory(this))
{
}

HeaderFactoryPrivate::~HeaderFactoryPrivate()
{
    qDeleteAll(headerMakers);
    delete instance;
}

HeaderFactory *HeaderFactory::self()
{
    return sInstance->instance;
}

Headers::Base *HeaderFactory::createHeader(const QByteArray &type)
{
    Q_ASSERT(!type.isEmpty());
    const HeaderMakerBase *maker = d->headerMakers.value(type.toLower());
    if (maker) {
        return maker->create();
    } else {
        //qCritical() << "Unknown header type" << type;
        //return new Headers::Generic;
        return 0;
    }
}

HeaderFactory::HeaderFactory(HeaderFactoryPrivate *dd)
    : d(dd)
{
}

HeaderFactory::~HeaderFactory()
{
}

bool HeaderFactory::registerHeaderMaker(const QByteArray &type, HeaderMakerBase *maker)
{
    if (type.isEmpty()) {
        // This is probably a generic (but not abstract) header,
        // like Address or MailboxList.  We cannot register those.
        qWarning() << "Tried to register header with empty type.";
        return false;
    }
    const QByteArray ltype = type.toLower();
    if (d->headerMakers.contains(ltype)) {
        qWarning() << "Header of type" << type << "already registered.";
        // TODO should we make this an error?
        return false;
    }
    d->headerMakers.insert(ltype, maker);
    return true;
}
