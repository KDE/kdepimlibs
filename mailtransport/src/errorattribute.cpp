/*
    Copyright 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "errorattribute.h"



using namespace Akonadi;
using namespace MailTransport;

class ErrorAttribute::Private
{
  public:
    QString mMessage;
};

ErrorAttribute::ErrorAttribute( const QString &msg )
  : d( new Private )
{
  d->mMessage = msg;
}

ErrorAttribute::~ErrorAttribute()
{
  delete d;
}

ErrorAttribute *ErrorAttribute::clone() const
{
  return new ErrorAttribute( d->mMessage );
}

QByteArray ErrorAttribute::type() const
{
  static const QByteArray sType( "ErrorAttribute" );
  return sType;
}

QByteArray ErrorAttribute::serialized() const
{
  return d->mMessage.toUtf8();
}

void ErrorAttribute::deserialize( const QByteArray &data )
{
  d->mMessage = QString::fromUtf8( data );
}

QString ErrorAttribute::message() const
{
  return d->mMessage;
}

void ErrorAttribute::setMessage( const QString &msg )
{
  d->mMessage = msg;
}

