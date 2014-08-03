/*
    kmime_header_factory.h

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
  This file is part of the API for handling @ref MIME data and
  defines the HeaderFactory class.

  @brief
  Defines the HeaderFactory class.

  @authors Constantin Berzan \<exit3219@gmail.com\>
*/

#ifndef __KMIME_HEADERFACTORY_H__
#define __KMIME_HEADERFACTORY_H__

#include "kmime_export.h"

#include <QtCore/QByteArray>

namespace KMime
{

namespace Headers
{
class Base;
}

class HeaderMakerBase
{
public:
    virtual ~HeaderMakerBase() {}
    virtual Headers::Base *create() const = 0;
};

template <typename T>
class HeaderMaker : public HeaderMakerBase
{
public:
    virtual Headers::Base *create() const
    {
        return new T;
    }
};

class HeaderFactoryPrivate;

/**
  docu TODO
*/
class HeaderFactory
{
public:
    static HeaderFactory *self();

    template<typename T> inline bool registerHeader()
    {
        T dummy;
        return registerHeaderMaker(QByteArray(dummy.type()), new HeaderMaker<T>());
    }

    Headers::Base *createHeader(const QByteArray &type);

private:
    explicit HeaderFactory(HeaderFactoryPrivate *dd);
    HeaderFactory(const HeaderFactory &other);   // undefined
    HeaderFactory &operator=(const HeaderFactory &other);   // undefined
    ~HeaderFactory();

    bool registerHeaderMaker(const QByteArray &type, HeaderMakerBase *maker);

    friend class HeaderFactoryPrivate;
    HeaderFactoryPrivate *const d;
};

} // namespace KMime

#endif /* __KMIME_HEADERFACTORY_H__ */
