/*
 * Copyright 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "idlejob_p.h"
#include "job_p.h"
#include "session.h"
#include "protocolhelper_p.h"

#include <akonadi/private/imapparser_p.h>
#include <akonadi/private/protocol_p.h>
#include <QTimer>

namespace Akonadi {

class IdleJobPrivate: public Akonadi::JobPrivate
{
  public:
    IdleJobPrivate( IdleJob *parent );
    virtual ~IdleJobPrivate();

    QByteArray convertAndClearSet( const QByteArray &operation,
                                   QList<Entity::Id> &set );
    QByteArray convertAndClearSet( const QByteArray &operation,
                                   QList<QByteArray> &set );
    void scheduleFilterUpdate();
    void _k_updateFilter();

    Session *session;

    IdleNotification notification;
    ProtocolHelperValuePool *valuePool;

    QList<Entity::Id> addedCollections, removedCollections;
    QList<Entity::Id> addedItems, removedItems;
    QList<QByteArray> addedMimeTypes, removedMimeTypes;
    QList<QByteArray> addedResources, removedResources;
    QList<QByteArray> addedSessions, removedSessions;

    QTimer *updateFilterTimer;
};

}

using namespace Akonadi;

class IdleNotification::Private: public QSharedData
{
  public:
    Private();
    Private( const Private &other );

    Idle::Type type;
    Idle::Operation operation;
    QList<Item> items;
    QSet<QByteArray> changedParts;
    QSet<QByteArray> addedFlags;
    QSet<QByteArray> removedFlags;
    Entity::Id destinationCollection;
    Entity::Id sourceCollection;
    QByteArray resource;
    QByteArray destinationResource;
};

IdleNotification::Private::Private()
  : QSharedData()
  , type( Idle::InvalidType )
  , operation( Idle::InvalidOperation )
  , destinationCollection( -1 )
  , sourceCollection( - 1 )
{
}

IdleNotification::Private::Private( const Private &other )
  : QSharedData( other )
  , type( other.type )
  , operation( other.operation )
  , items( other.items )
  , changedParts( other.changedParts )
  , addedFlags( other.addedFlags )
  , removedFlags( other.removedFlags )
  , destinationCollection( other.destinationCollection )
  , sourceCollection( other.sourceCollection )
  , resource( other.resource )
  , destinationResource( other.destinationResource )
{
}

IdleNotification::IdleNotification()
  : d( new Private )
{
}

IdleNotification::IdleNotification( const IdleNotification &other )
  : d( other.d )
{
}

IdleNotification::~IdleNotification()
{
}

IdleNotification &IdleNotification::operator=( const IdleNotification &other )
{
  if ( this != &other ) {
    d = other.d;
  }

  return *this;
}

bool IdleNotification::isValid() const
{
  return d->type != Idle::InvalidType &&
         d->operation != Idle::InvalidOperation;
}

Idle::Type IdleNotification::type() const
{
  return d->type;
}

void IdleNotification::setType( Idle::Type type )
{
  d->type = type;
}

Idle::Operation IdleNotification::operation() const
{
  return d->operation;
}

void IdleNotification::setOperation( Idle::Operation operation )
{
  d->operation = operation;
}

QSet<QByteArray> IdleNotification::changedParts() const
{
  return d->changedParts;
}

void IdleNotification::setChangedParts( const QSet<QByteArray> &changedParts )
{
  d->changedParts = changedParts;
}

QSet<QByteArray> IdleNotification::addedFlags() const
{
  return d->addedFlags;
}

void IdleNotification::setAddedFlags( const QSet<QByteArray> &addedFlags )
{
  d->addedFlags = addedFlags;
}

QSet<QByteArray> IdleNotification::removedFlags() const
{
  return d->removedFlags;
}

void IdleNotification::setRemovedFlags( const QSet<QByteArray> &removedFlags )
{
  d->removedFlags = removedFlags;
}

Entity::Id IdleNotification::sourceCollection() const
{
  return d->sourceCollection;
}

void IdleNotification::setSourceCollection( Entity::Id sourceCollection )
{
  d->sourceCollection = sourceCollection;
}

Entity::Id IdleNotification::destinationCollection() const
{
  return d->destinationCollection;
}

void IdleNotification::setDestinationCollection( Entity::Id destinationCollection )
{
  d->destinationCollection = destinationCollection;
}

QList<Item> IdleNotification::items() const
{
  return d->items;
}

void IdleNotification::setItems( const QList<Item> &items )
{
  d->items = items;
}

void IdleNotification::addItem( const Item &item )
{
  d->items << item;
}

QByteArray IdleNotification::resource() const
{
  return d->resource;
}

void IdleNotification::setResource( const QByteArray &resource )
{
  d->resource = resource;
}

QByteArray IdleNotification::destinationResource() const
{
  return d->destinationResource;
}

void IdleNotification::setDestinationResource( const QByteArray &resource )
{
  d->destinationResource = resource;
}





IdleJobPrivate::IdleJobPrivate( IdleJob *parent )
 : JobPrivate( parent )
 , updateFilterTimer( 0 )
{
}

IdleJobPrivate::~IdleJobPrivate()
{
  updateFilterTimer->deleteLater();
}

void IdleJobPrivate::scheduleFilterUpdate()
{
  if ( !updateFilterTimer || updateFilterTimer->isActive() ) {
    return;
  }

  updateFilterTimer->start();
}


QByteArray IdleJobPrivate::convertAndClearSet( const QByteArray &operation,
                                               QList<Entity::Id> &set )
{
  if ( set.isEmpty() ) {
    return QByteArray();
  }

  ImapSet imapSet;
  imapSet.add( set );
  const QByteArray r =  operation + " (" + imapSet.toImapSequenceSet() + ")";
  set.clear();
  return r;
}

QByteArray IdleJobPrivate::convertAndClearSet( const QByteArray &operation,
                                               QList<QByteArray> &set )
{
  if ( set.isEmpty() ) {
    return QByteArray();
  }

  kDebug() << operation;
  const QByteArray r =  operation + " (" + ImapParser::join( set, " " ) + ")";
  set.clear();
  return r;
}

void IdleJobPrivate::_k_updateFilter()
{
  QByteArray command;
  command += ' ' + convertAndClearSet( "+" AKONADI_PARAM_COLLECTIONS, addedCollections );
  command += ' ' + convertAndClearSet( "-" AKONADI_PARAM_COLLECTIONS, removedCollections );
  command += ' ' + convertAndClearSet( "+" AKONADI_PARAM_ITEMS, addedItems );
  command += ' ' + convertAndClearSet( "-" AKONADI_PARAM_ITEMS, removedItems );
  command += ' ' + convertAndClearSet( "+" AKONADI_PARAM_MIMETYPES, addedMimeTypes );
  command += ' ' + convertAndClearSet( "-" AKONADI_PARAM_MIMETYPES, removedMimeTypes );
  command += ' ' + convertAndClearSet( "+" AKONADI_PARAM_RESOURCES, addedResources );
  command += ' ' + convertAndClearSet( "-" AKONADI_PARAM_RESOURCES, removedResources );
  command += ' ' + convertAndClearSet( "+" AKONADI_PARAM_IGNOREDSESSIONS, addedSessions );
  command += ' ' + convertAndClearSet( "-" AKONADI_PARAM_IGNOREDSESSIONS, removedSessions );

  if ( command.isEmpty() ) {
    return;
  }

  kDebug() << "IDLE FILTER" << command;
  static_cast<IdleJob*>( q_ptr )->sendData( "IDLE FILTER" + command );
}

IdleJob::IdleJob( Akonadi::Session *session )
  : Job(  new IdleJobPrivate( this ), session )
{
  Q_D( IdleJob );
  d->session = session;
  d->valuePool = new ProtocolHelperValuePool;
}

IdleJob::~IdleJob()
{
}

void IdleJob::addMonitoredCollection( const Collection &collection )
{
  Q_D( IdleJob );
  if ( !d->addedCollections.contains( collection.id() ) ) {
      d->addedCollections << collection.id();
  }
  d->scheduleFilterUpdate();
}

void IdleJob::removeMonitoredCollection( const Collection &collection )
{
  Q_D( IdleJob );
  if ( !d->removedCollections.contains( collection.id() ) ) {
    d->removedCollections << collection.id();
  }
  d->addedCollections.removeAll( collection.id() );
  d->scheduleFilterUpdate();
}

void IdleJob::addMonitoredItem( Entity::Id item )
{
  Q_D( IdleJob );
  if ( !d->addedItems.contains( item ) ) {
      d->addedItems << item;
  }
  d->scheduleFilterUpdate();
}

void IdleJob::removeMonitoredItem( Entity::Id item )
{
  Q_D( IdleJob );
  if ( !d->removedItems.contains( item ) ) {
    d->removedItems << item;
  }
  d->addedItems.removeAll( item );
  d->scheduleFilterUpdate();
}

void IdleJob::addMonitoredMimeType( const QString &mimeType )
{
  Q_D( IdleJob );
  if ( !d->addedMimeTypes.contains( mimeType.toLatin1() ) ) {
      d->addedMimeTypes << mimeType.toLatin1();
  }
  d->scheduleFilterUpdate();
}

void IdleJob::removeMonitoredMimeType( const QString &mimeType )
{
  Q_D( IdleJob );
  if ( !d->removedMimeTypes.contains( mimeType.toLatin1() ) ) {
    d->removedMimeTypes << mimeType.toLatin1();
  }
  d->addedMimeTypes.removeAll( mimeType.toLatin1() );
  d->scheduleFilterUpdate();
}

void IdleJob::addMonitoredResource( const QByteArray &resource )
{
  Q_D( IdleJob );
  if ( !d->addedResources.contains( resource ) ) {
      d->addedResources << resource;
  }
  d->scheduleFilterUpdate();
}

void IdleJob::removeMonitoredResource( const QByteArray &resource )
{
  Q_D( IdleJob );
  if ( !d->removedResources.contains( resource ) ) {
    d->removedResources << resource;
  }
  d->addedResources.removeAll( resource );
  d->scheduleFilterUpdate();
}

void IdleJob::addIgnoredSession( const QByteArray &session )
{
  Q_D( IdleJob );
  if ( !d->addedSessions.contains( session ) ) {
      d->addedSessions << session;
  }
  d->scheduleFilterUpdate();
}

void IdleJob::removeIgnoredSession( const QByteArray &session )
{
  Q_D( IdleJob );
  if ( !d->removedSessions.contains( session ) ) {
    d->removedSessions << session;
  }
  d->addedSessions.removeAll( session );
  d->scheduleFilterUpdate();
}

void IdleJob::setAllMonitored( bool allMonitored )
{
  // TODO
}


void IdleJob::doStart()
{
  Q_D( IdleJob );

  d->updateFilterTimer = new QTimer( this );
  d->updateFilterTimer->setInterval( 0 );
  d->updateFilterTimer->setSingleShot( true );
  connect( d->updateFilterTimer, SIGNAL(timeout()),
           this, SLOT(_k_updateFilter()) );

  sendData("IDLE START CLIENTID " + d->session->sessionId());
}

void IdleJob::sendData( const QByteArray &data )
{
  Q_D( IdleJob );

  const QByteArray command = d->newTag() + " " + data;
  d->writeData( command );
}

void IdleJob::doHandleResponse( const QByteArray &tag, const QByteArray &data )
{
  Q_D( IdleJob );

  // FIXME: We don't have collection notifications implemented on server yet

  if ( tag == "*" ) {
    QList<QByteArray> list;
    QByteArray res;
    qint64 id;
    bool ok =- false;
    int pos = 0;

    // UID
    pos = ImapParser::parseNumber( data, id );

    // Command
    pos = ImapParser::parseString( data, res, pos );
    if ( res != "NOTIFY" ) {
      kWarning() << "Unknown command" << res;
      return;
    }

    pos = ImapParser::parseString( data, res, pos );
    d->notification.setOperation( Idle::commandToOperation( res ) );
    if ( d->notification.operation() == Idle::InvalidOperation ) {
      kWarning() << "Invalid operation" << res;
      return;
    }

    pos = ImapParser::parseString( data, res, pos );
    if ( res == "ITEM" ) {
      d->notification.setType( Idle::Item );
    } else if ( res == "COLLECTION") {
      d->notification.setType( Idle::Collection );
    } else {
      kWarning() << "Invalid notification type" << res;
      return;
    }

    if ( d->notification.operation() == Idle::Add ||
         d->notification.operation() == Idle::Link ||
         d->notification.operation() == Idle::Unlink )
    {
      pos = ImapParser::parseString( data, res, pos );
      if ( res != "DESTINATION" ) {
        kWarning() << "Invalid argument" << res;
        return;
      }
      pos = ImapParser::parseNumber( data, id );
      d->notification.setDestinationCollection( id );
    } else if ( d->notification.operation() == Idle::ModifyFlags ) {
      int i = ImapParser::parseString( data, res, pos );
      if ( res == "ADDED" ) {
        pos = ImapParser::parseParenthesizedList( data, list, i );
        d->notification.setAddedFlags( list.toSet() );
      }
      i = ImapParser::parseString( data, res, pos );
      if ( res == "REMOVED" ) {
        pos = ImapParser::parseParenthesizedList( data, list, i );
        d->notification.setRemovedFlags( list.toSet() );
      }
    } else if ( d->notification.operation() == Idle::Modify ) {
      pos = ImapParser::parseString( data, res, pos );
      if ( res != "PARTS" ) {
        kWarning() << "Invalid argument" << res;
        return;
      }
      pos = ImapParser::parseParenthesizedList( data, list, pos );
      d->notification.setChangedParts( list.toSet() );
    } else if ( d->notification.operation() == Idle::Move ) {
      pos = ImapParser::parseString( data, res, pos );
      if ( res != "SOURCE" ) {
        kWarning() << "Invalid argument" << res;
        return;
      }
      pos = ImapParser::parseNumber( data, id, &ok, pos );
      d->notification.setSourceCollection( id );

      pos = ImapParser::parseString( data, res, pos );
      if ( res != "DESTINATION" ) {
        kWarning() << "Invalid argument" << res;
        return;
      }
      pos = ImapParser::parseNumber( data, id, &ok, pos );
      d->notification.setDestinationCollection( id );

      pos = ImapParser::parseString( data, res, pos );
      if ( res != "RESOURCE" ) {
        kWarning() << "Invalid argument" << res;
        return;
      }
      pos = ImapParser::parseString( data, res, pos );
      d->notification.setResource( res );

      pos = ImapParser::parseString( data, res, pos );
      if ( res != "DESTRESOURCE" ) {
        kWarning() << "Invalid argument" << res;
        return;
      }
      pos = ImapParser::parseString( data, res, pos );
      d->notification.setDestinationResource( res );
    }

    Item item;
    pos += ImapParser::parseParenthesizedList( data, list, pos );
    ProtocolHelper::parseItemFetchResult( list, item, d->valuePool );
    d->notification.addItem( item );

  } else if ( tag == "+" ) {
    const int index = data.indexOf( "DONE" );
    bool ok = false;
    QList<QByteArray> list;
    ImapParser::parseParenthesizedList( data, list, index + 4 );
    if ( list.size() == 0 ) {
      kWarning() << "Error while parsing number line '" << data << "'. Skipping this batch";
      return;
    }

    const int batchSize = list.first().toInt( &ok );
    const int msgCount = d->notification.items().count();
    Q_ASSERT( batchSize == msgCount );
    if ( batchSize != msgCount ) {
      kWarning() << "Server claims batch contained" << batchSize << "notifications, but we got" << msgCount << ". Skipping this batch";
      return;
    }

    kDebug() << "Emitting notification for" << msgCount << "items!";
    Q_EMIT notify( d->notification );
    d->notification = IdleNotification();
  }
}

#include "moc_idlejob_p.cpp"
