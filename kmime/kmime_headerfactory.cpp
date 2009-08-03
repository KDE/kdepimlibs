/*
    kmime_header_factory.cpp

    KMime, the KDE internet mail/usenet news message library.
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

#include "kmime_headerfactory.h"
#include "kmime_headers.h"

#include <QHash>

#include <KDebug>
#include <KGlobal>

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
    QHash<QByteArray, Headers::Base*> headers; // Type->obj mapping; with lower-case type.
};

K_GLOBAL_STATIC( HeaderFactoryPrivate, sInstance )

HeaderFactoryPrivate::HeaderFactoryPrivate()
  : instance( new HeaderFactory( this ) )
{
}

HeaderFactoryPrivate::~HeaderFactoryPrivate()
{
  qDeleteAll( headers.values() );
  delete instance;
}



HeaderFactory* HeaderFactory::self()
{
  return sInstance->instance;
}

Headers::Base *HeaderFactory::createHeader( const QByteArray &type )
{
  Q_ASSERT( !type.isEmpty() );
  Headers::Base *h = d->headers.value( type.toLower() );
  if( h ) {
    return h->clone();
  } else {
    kError() << "Unknown header type" << type;
    //return new Headers::Generic;
    return 0;
  }
}

HeaderFactory::HeaderFactory( HeaderFactoryPrivate *dd )
  : d( dd )
{
}

HeaderFactory::~HeaderFactory()
{
}

bool HeaderFactory::registerHeader( Headers::Base *header )
{
  if( QByteArray( header->type() ).isEmpty() ) {
    // This is probably a generic (but not abstract) header,
    // like Address or MailboxList.  We cannot register those.
    kWarning() << "Tried to register header with empty type.";
    return false;
  }
  QByteArray ltype = QByteArray( header->type() ).toLower();
  if( d->headers.contains( ltype ) ) {
    kWarning() << "Header of type" << header->type() << "already registered.";
    // TODO should we make this an error?
    return false;
  }
  d->headers.insert( ltype, header );
  kDebug() << "registered type" << header->type();
  return true;
}

