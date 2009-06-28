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

#include <kmime/kmime_message.h>

#include <akonadi/collection.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/kmime/addressattribute.h>

using namespace Akonadi;
using namespace MailTransport;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class AkonadiJobPrivate
{
  public:
    AkonadiJob *q;
    Item::Id itemId;
    QDBusInterface *iface;

    // slots
    void itemCreateResult( KJob *job );
    void itemFetchResult( KJob *job );

    void doSend();
};

void AkonadiJobPrivate::itemCreateResult( KJob *job )
{
  if( job->error() ) {
    // KCompositeJob takes care of the error.
    return;
  }

  Q_ASSERT( dynamic_cast<ItemCreateJob*>( job ) );
  itemId = static_cast<ItemCreateJob*>( job )->item().id();
  kDebug() << "Created item with id" << itemId;
  doSend();
}

void AkonadiJobPrivate::itemFetchResult( KJob *job )
{
  if( job->error() ) {
    // KCompositeJob takes care of the error.
    return;
  }

  Q_ASSERT( dynamic_cast<ItemFetchJob*>( job ) );
  const ItemFetchJob *fjob = static_cast<ItemFetchJob*>( job );
  Q_ASSERT( fjob->items().count() == 1 );
  const Item item = fjob->items().first();
  if( !item.hasAttribute<AddressAttribute>() ) {
    kWarning() << "Item does not have AddressAttribute.";
    q->setError( KJob::UserDefinedError );
    q->setErrorText( i18n( "Item does not have address information." ) );
    q->emitResult();
  } else {
    kDebug() << "Good, item" << itemId << "has AddressAttribute.";
    itemId = item.id();
    doSend();
  }
}

void AkonadiJobPrivate::doSend()
{
  Q_ASSERT( itemId >= 0 );

  iface = new QDBusInterface(
      QLatin1String( "org.freedesktop.Akonadi.Resource." ) + q->transport()->host(),
      QLatin1String( "/" ), QLatin1String( "org.freedesktop.Akonadi.Resource.Transport" ),
      QDBusConnection::sessionBus(), q );
  if( !iface->isValid() ) {
    q->setError( KJob::UserDefinedError );
    q->setErrorText( i18n( "Failed to get D-Bus interface of resource %1.", q->transport()->host() ) );
    q->emitResult();
    return;
  }

  QObject::connect( iface, SIGNAL(transportResult(qlonglong,bool,QString)),
      q, SLOT(resourceResult(qlonglong,bool,QString)) );

  QDBusReply<void> reply = iface->call( QLatin1String( "send" ), itemId );
  if( !reply.isValid() ) {
    q->setError( KJob::UserDefinedError );
    q->setErrorText( i18n( "Invalid D-Bus reply from resource %1.", q->transport()->host() ) );
    q->emitResult();
    return;
  }
}

AkonadiJob::AkonadiJob( Transport *transport, QObject *parent )
  : TransportJob( transport, parent ), d( new AkonadiJobPrivate )
{
  d->q = this;
  d->itemId = -1;
  d->iface = 0;
}

AkonadiJob::~AkonadiJob()
{
  delete d;
}

Akonadi::Item::Id AkonadiJob::itemId() const
{
  if( d->itemId < 0 ) {
    kWarning() << "Invalid item.";
  }
  return d->itemId;
}

void AkonadiJob::setItemId( Akonadi::Item::Id id )
{
  Q_ASSERT( id >= 0 );
  d->itemId = id;
}

void AkonadiJob::doStart()
{
  if( d->itemId < 0 ) {
    // Create the item from TransportJob data.
    using namespace KMime;
    Item item;
    item.setMimeType( QString::fromLatin1( "message/rfc822" ) );
    Message::Ptr msg = Message::Ptr( new Message );
    msg->setContent( data() );
    item.setPayload<Message::Ptr>( msg );
    AddressAttribute *attr = new AddressAttribute( sender(), to(), cc(), bcc() );
    item.addAttribute( attr );
    // FIXME Where should this item be created???
    // And it should probably be deleted afterwards???
    ItemCreateJob *cjob = new ItemCreateJob( item, Collection::root(), this );
    connect( cjob, SIGNAL(result(KJob*)), this, SLOT(itemCreateResult(KJob*)) );
    addSubjob( cjob );
  } else {
    // We have a ready-made item.  Check that it has an AddressAttribute.
    ItemFetchJob *fjob = new ItemFetchJob( Item( d->itemId ), this );
    fjob->fetchScope().fetchFullPayload( false );
    fjob->fetchScope().fetchAttribute<AddressAttribute>();
    connect( fjob, SIGNAL(result(KJob*)), this, SLOT(itemFetchResult(KJob*)) );
    addSubjob( fjob );
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
