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

#include "dispatchmodeattribute.h"

#include <KDebug>

#include "akonadi/attributefactory.h"

using namespace Akonadi;
using namespace OutboxInterface;

class DispatchModeAttribute::Private
{
  public:
    DispatchMode mMode;
    QDateTime mDueDate;
};

DispatchModeAttribute::DispatchModeAttribute( DispatchMode mode, const QDateTime &date )
  : d( new Private )
{
  d->mMode = mode;
  d->mDueDate = date;
}

DispatchModeAttribute::~DispatchModeAttribute()
{
  delete d;
}

DispatchModeAttribute* DispatchModeAttribute::clone() const
{
  return new DispatchModeAttribute( d->mMode, d->mDueDate );
}

QByteArray DispatchModeAttribute::type() const
{
  static const QByteArray sType( "DispatchModeAttribute" );
  return sType;
}

QByteArray DispatchModeAttribute::serialized() const
{
  switch( d->mMode ) {
    case Immediately: return "immediately";
    case AfterDueDate: return "after" + d->mDueDate.toString(Qt::ISODate).toLatin1();
    case Never: return "never";
  }

  Q_ASSERT( false );
  return QByteArray(); // suppress control-reaches-end-of-non-void-function warning
}

void DispatchModeAttribute::deserialize( const QByteArray &data )
{
  d->mDueDate = QDateTime();
  if ( data == "immediately" ) {
    d->mMode = Immediately;
  } else if ( data == "never" ) {
    d->mMode = Never;
  } else if ( data.startsWith( QByteArray( "after" ) ) ) {
    d->mMode = AfterDueDate;
    d->mDueDate = QDateTime::fromString( data.mid(5), Qt::ISODate );
    // NOTE: 5 is the strlen of "after".
  } else {
    kWarning() << "Failed to deserialize data [" << data << "]";
  }
}

DispatchModeAttribute::DispatchMode DispatchModeAttribute::dispatchMode() const
{
  return d->mMode;
}

void DispatchModeAttribute::setDispatchMode( DispatchMode mode )
{
  d->mMode = mode;
}
  
QDateTime DispatchModeAttribute::dueDate() const
{
  return d->mDueDate;
}

void DispatchModeAttribute::setDueDate( const QDateTime &date )
{
  d->mDueDate = date;
}

