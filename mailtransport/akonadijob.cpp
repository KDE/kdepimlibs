/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "akonadijob.h"
#include "transport.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QString>

#include <KLocalizedString>

using namespace Akonadi;
using namespace MailTransport;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class AkonadiJobPrivate
{
  public:
    Item::Id itemId;
    QDBusInterface *iface;
};

AkonadiJob::AkonadiJob( Transport *transport, QObject *parent )
  : TransportJob( transport, parent ), d( new AkonadiJobPrivate )
{
  d->itemId = -1;
  d->iface = 0;
}

AkonadiJob::~ AkonadiJob()
{
  delete d;
}

Akonadi::Item::Id AkonadiJob::itemId() const
{
  return d->itemId;
}

void AkonadiJob::setItemId( Akonadi::Item::Id id )
{
  d->itemId = id;
}

void AkonadiJob::doStart()
{
  if( !d->itemId < 0 ) {
    // TODO should this check be performed here or somewhere on a higher level?
    setError( UserDefinedError );
    setErrorText( i18n( "Asked to send invalid item with id %1.", d->itemId ) );
    emitResult();
    return;
  }

  d->iface = new QDBusInterface(
      QLatin1String( "org.freedesktop.Akonadi.Resource." ) + transport()->host(),
      QLatin1String( "/" ), QLatin1String( "org.freedesktop.Akonadi.Resource.Transport" ),
      QDBusConnection::sessionBus(), this );
  if( !d->iface->isValid() ) {
    setError( UserDefinedError );
    setErrorText( i18n( "Failed to get D-Bus interface of resource %1.", transport()->host() ) );
    emitResult();
    return;
  }

  connect( d->iface, SIGNAL(transportResult(qlonglong,bool,QString)),
      this, SLOT(resourceResult(qlonglong,bool,QString)) );

  // What TODO about timeouts?  It is quite possible that the result D-Bus signal
  // will get lost, and then what?

  QDBusReply<void> reply = d->iface->call( QLatin1String( "send" ), d->itemId );
  if( !reply.isValid() ) {
    setError( UserDefinedError );
    setErrorText( i18n( "Invalid D-Bus reply from resource %1.", transport()->host() ) );
    emitResult();
    return;
  }
}

void AkonadiJob::resourceResult( qlonglong itemId, bool success, const QString &message )
{
  Q_ASSERT( itemId == d->itemId );
  if( !success ) {
    setError( UserDefinedError );
    setErrorText( message );
  }
  emitResult();
}

#include "akonadijob.moc"
