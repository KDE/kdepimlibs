/*
  This file is part of the kcal library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
  This file is part of the API for handling calendar data and
  defines the CalFormat base class.

  @brief
  Base class providing an interface to various calendar formats.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "calformat.h"

#include <klocale.h>
#include <kdebug.h>
#include <krandom.h>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::CalFormat::Private
{
  public:
    Private() : mException( 0 ) {}
    ~Private() { delete mException; }
    static QString mApplication; // Name of application, for creating unique ID strings
    static QString mProductId;   // PRODID string to write to calendar files
    QString mLoadedProductId;    // PRODID string loaded from calendar file
    ErrorFormat *mException;
};

QString CalFormat::Private::mApplication = QLatin1String( "libkcal" );
QString CalFormat::Private::mProductId =
  QLatin1String( "-//K Desktop Environment//NONSGML libkcal 4.3//EN" );
//@endcond

CalFormat::CalFormat()
  : d( new KCal::CalFormat::Private )
{
}

CalFormat::~CalFormat()
{
  delete d;
}

void CalFormat::clearException()
{
  delete d->mException;
  d->mException = 0;
}

void CalFormat::setException( ErrorFormat *exception )
{
  delete d->mException;
  d->mException = exception;
}

ErrorFormat *CalFormat::exception()
{
  return d->mException;
}

void CalFormat::setApplication( const QString &application,
                                const QString &productID )
{
  Private::mApplication = application;
  Private::mProductId = productID;
}

const QString &CalFormat::application()
{
  return Private::mApplication;
}

const QString &CalFormat::productId()
{
  return Private::mProductId;
}

const QString &CalFormat::loadedProductId()
{
  return d->mLoadedProductId;
}

void CalFormat::setLoadedProductId( const QString &id )
{
  d->mLoadedProductId = id;
}

QString CalFormat::createUniqueId()
{
  int hashTime = QTime::currentTime().hour() +
                 QTime::currentTime().minute() + QTime::currentTime().second() +
                 QTime::currentTime().msec();
  QString uidStr = QString( "%1-%2.%3" ).
                   arg( Private::mApplication ).
                   arg( KRandom::random() ).
                   arg( hashTime );
  return uidStr;
}
