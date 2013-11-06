/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "changerecorder_p.h"
#include "idlejob_p.h"
#include "itemfetchjob.h"
#include "protocolhelper_p.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QFileInfo>
#include <akonadi/private/notificationmessagev2_p.h>
#include <boost/iterator/iterator_concepts.hpp>

using namespace Akonadi;

ChangeRecorderPrivate::ChangeRecorderPrivate( ChangeRecorder *parent )
  : MonitorPrivate( parent ),
    settings( 0 ),
    enableChangeRecording( true ),
    m_lastKnownNotificationsCount( 0 ),
    m_startOffset( 0 ),
    m_needFullSave( true ),
    m_fetchingLegacyNotifications( false )
{
}

void ChangeRecorderPrivate::slotNotify( const Akonadi::IdleNotification &notification )
{
  Q_Q( ChangeRecorder );
  const int oldChanges = pendingNotifications.size();
  // with change recording disabled this will automatically take care of dispatching notification messages and saving
  MonitorPrivate::slotNotify( notification );
  if ( enableChangeRecording && pendingNotifications.size() != oldChanges ) {
    emit q->changesAdded();
  }
}

/*
bool ChangeRecorderPrivate::emitNotification( const Akonadi::IdleNotification &notification )
{
  const bool someoneWasListening = MonitorPrivate::emitNotification( notification );
  if ( !someoneWasListening && enableChangeRecording ) {
    QMetaObject::invokeMethod( q_ptr, "replayNext", Qt::QueuedConnection ); // skip notifications no one was listening to
  }
  return someoneWasListening;
}
*/

// The QSettings object isn't actually used anymore, except for migrating old data
// and it gives us the base of the filename to use. This is all historical.
QString ChangeRecorderPrivate::notificationsFileName() const
{
  return settings->fileName() + QLatin1String( "_changes.dat" );
}

void ChangeRecorderPrivate::loadNotifications()
{
  pendingNotifications.clear();

  const QString changesFileName = notificationsFileName();

  /**
    * In an older version we recorded changes inside the settings object, however
    * for performance reasons we changed that to store them in a separated file.
    * If this file doesn't exists, it means we run the new version the first time,
    * so we have to read in the legacy list of changes first.
    */
  if ( !QFile::exists( changesFileName ) ) {
    pendingNotifications = loadFromSettingsFile( settings );
  } else {
    QFile file( changesFileName );
    if ( file.open( QIODevice::ReadOnly ) ) {
      m_needFullSave = false;
      pendingNotifications = loadFromFile( &file );
    } else {
      m_needFullSave = true;
    }
  }

  if ( m_missingLegacyNotifications.isEmpty() ) {
    notificationsLoaded();
  } else {
    fetchItemsForLegacyNotifications( m_missingLegacyNotifications.keys() );
  }
}

void ChangeRecorderPrivate::fetchItemsForLegacyNotifications( const QList< Entity::Id > &ids )
{
  Q_Q( ChangeRecorder );
  ItemFetchJob *fetchJob = new ItemFetchJob( ids, q );
  fetchJob->fetchScope().fetchFullPayload();
  fetchJob->fetchScope().fetchAllAttributes();
  QObject::connect( fetchJob, SIGNAL(finished(KJob*)),
                    q_ptr, SLOT(legacyNotificationsItemsFetched(KJob*)) );
}

void ChangeRecorderPrivate::legacyNotificationsItemsFetched( KJob* job )
{
  ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  Item::List items = fetchJob->items();

  for ( int i = 0; i < items.count(); ++i ) {
    const Item item = items.at( i );
    IdleNotification ntf = m_missingLegacyNotifications.take( item.id() );
    ntf.addItem( item );

    // Replace the placeholder notification with the real one
    pendingNotifications.replace( i, ntf );
  }

  m_missingLegacyNotifications.clear();
  notificationsLoaded();
}


static const quint64 s_currentVersion = Q_UINT64_C(0x000200000000);
static const quint64 s_versionMask    = Q_UINT64_C(0xFFFF00000000);
static const quint64 s_sizeMask       = Q_UINT64_C(0x0000FFFFFFFF);

QQueue<IdleNotification> ChangeRecorderPrivate::loadFromFile( QIODevice *device )
{
  QDataStream stream( device );
  stream.setVersion( QDataStream::Qt_4_6 );

  QQueue<IdleNotification> list;

  int operation, type;
  Entity::Id collection, destinationCollection;
  QByteArray resource, destinationResource;
  QList<QByteArray> addedFlags, removedFlags, changedParts;
  QList<Entity::Id> itemsIds;

  quint64 sizeAndVersion;
  stream >> sizeAndVersion;

  const quint64 size = sizeAndVersion & s_sizeMask;
  const quint64 version = (sizeAndVersion & s_versionMask) >> 32;

  quint64 startOffset = 0;
  if ( version >= 1 ) {
    stream >> startOffset;
  }

  // If we skip the first N items, then we'll need to rewrite the file on saving.
  // Also, if the file is old, it needs to be rewritten.
  m_needFullSave = startOffset > 0 || version == 0;

  for ( quint64 i = 0; i < size && !stream.atEnd(); ++i ) {
    QQueue<IdleNotification> ntfs;

    // Pre-IDLE notifications
    if ( version < 3 ) {
      if ( version == 1 ) {
        ntfs = fromNotificationV1( stream );
      } else if ( version == 2 ) {
        ntfs = fromNotificationV2( stream );
      }

      if ( i < startOffset ) {
        continue;
      }

      list << ntfs;

    // IDLE notifications
    } else {
      stream >> type;
      stream >> operation;
      stream >> collection;
      stream >> resource;
      stream >> destinationCollection;
      stream >> destinationResource;
      stream >> addedFlags;
      stream >> removedFlags;
      stream >> changedParts;
      stream >> itemsIds;

      IdleNotification msg;
      msg.setType( static_cast<Idle::Type>( type ) );
      msg.setOperation( static_cast<Idle::Operation>( operation ) );
      msg.setSourceCollection( collection );
      msg.setResource( resource );
      msg.setDestinationCollection( destinationCollection );
      msg.setDestinationResource( destinationResource );
      msg.setAddedFlags( addedFlags.toSet() );
      msg.setRemovedFlags( removedFlags.toSet() );
      msg.setChangedParts( changedParts.toSet() );

      Q_FOREACH ( Entity::Id id, itemsIds ) {
        m_missingLegacyNotifications.insert( id, msg );
      }

      ntfs << msg;
    }

    if ( i < startOffset ) {
      continue;
    }

    list << ntfs;
  }

  return list;
}

QQueue< IdleNotification > ChangeRecorderPrivate::loadFromSettingsFile( QSettings *settings )
{
  QStringList list;
  settings->beginGroup( QLatin1String( "ChangeRecorder" ) );

  QQueue<IdleNotification> notifications;

  const int size = settings->beginReadArray( QLatin1String( "change" ) );
  for ( int i = 0; i < size; ++i ) {
    settings->setArrayIndex( i );
    const Entity::Id id = settings->value( QLatin1String( "uid" ) ).toLongLong();

    IdleNotification ntf;
    // We kept Idle::Type and Idle::Operation compatible with NotificationMessageV2
    ntf.setType( static_cast<Idle::Type>( settings->value( QLatin1String( "type" ) ).toInt() ) );
    ntf.setOperation( static_cast<Idle::Operation>( settings->value( QLatin1String( "op" ) ).toInt() ) );
    ntf.setResource( settings->value( QLatin1String( "resource" ) ).toByteArray() );
    ntf.setSourceCollection( settings->value( QLatin1String( "parentCol" ) ).toLongLong() );
    ntf.setDestinationCollection( settings->value( QLatin1String( "parentDestCol" ) ).toLongLong() ) ;
    QSet<QByteArray> itemParts;
    Q_FOREACH ( const QString &entry, list ) {
      itemParts.insert( entry.toLatin1() );
    }
    ntf.setChangedParts( itemParts );

    // Enqueue one empty notification for each item
    notifications.enqueue( ntf );
    m_missingLegacyNotifications.insert( id, ntf );
  }
  settings->endArray();

  // ...delete the legacy list...
  settings->remove( QString() );
  settings->endGroup();

  return notifications;
}


QQueue<IdleNotification> ChangeRecorderPrivate::fromNotificationV1( QDataStream& stream )
{
  QByteArray dummyBA, resource;
  int operation, type;
  quint64 uid, sourceCollection, destinationCollection;;
  QString remoteId, mimeType;
  QSet<QByteArray> parts;

  stream >> dummyBA;
  stream >> type;
  stream >> operation;
  stream >> uid;
  stream >> remoteId;
  stream >> resource;
  stream >> sourceCollection;
  stream >> destinationCollection;
  stream >> mimeType;
  stream >> parts;

  IdleNotification ntf;
  ntf.setType( static_cast<Idle::Type>( type ) );
  ntf.setOperation( static_cast<Idle::Operation>( operation ) );
  ntf.setResource( resource );
  ntf.setSourceCollection( sourceCollection );
  ntf.setDestinationCollection( destinationCollection );
  ntf.setChangedParts( parts );

  Item item( uid );
  item.setRemoteId( remoteId );
  item.setMimeType( mimeType );

  ntf.addItem( item );

  m_missingLegacyNotifications.insert( uid, ntf );

  QQueue<IdleNotification> queue;
  queue << ntf;
  return queue;
}

QQueue<IdleNotification> ChangeRecorderPrivate::fromNotificationV2( QDataStream &stream )
{
  QByteArray dummyBA, resource, destinationResource;
  int type, operation, entityCnt;
  quint64 uid, parentCollection, destinationCollection;
  QString dummyString;
  QSet<QByteArray> parts, addedFlags, removedFlags;
  QQueue<IdleNotification> notifications;
  QList<Item::Id> itemsIds;

  stream >> dummyBA;
  stream >> type;
  stream >> operation;
  stream >> entityCnt;
  for ( int j = 0; j < entityCnt; ++j ) {
    stream >> uid;
    stream >> dummyString;
    stream >> dummyString;
    stream >> dummyString;
    itemsIds << uid;
  }
  stream >> resource;
  stream >> destinationResource;
  stream >> parentCollection;
  stream >> destinationCollection;
  stream >> parts;
  stream >> addedFlags;
  stream >> removedFlags;

  IdleNotification ntf;
  ntf.setType( static_cast<Idle::Type>( type ) );
  ntf.setOperation( static_cast<Idle::Operation>( operation ) );
  ntf.setResource( resource );
  ntf.setDestinationResource( destinationResource );
  ntf.setChangedParts( parts );
  ntf.setAddedFlags( addedFlags );
  ntf.setRemovedFlags( removedFlags );

  Q_FOREACH ( Entity::Id uid, itemsIds ) {
    m_missingLegacyNotifications.insert( uid, ntf );
  }

  notifications << ntf;

  return notifications;
}

QString ChangeRecorderPrivate::dumpNotificationListToString() const
{
  if ( !settings )
    return QString::fromLatin1( "No settings set in ChangeRecorder yet." );
  QString result;
  const QString changesFileName = notificationsFileName();
  QFile file( changesFileName );
  if ( !file.open( QIODevice::ReadOnly ) )
    return QString::fromLatin1( "Error reading " ) + changesFileName;

  QDataStream stream( &file );
  stream.setVersion( QDataStream::Qt_4_6 );

  QByteArray sessionId, resource, destResource;
  int type, operation, entityCnt;
  quint64 parentCollection, parentDestCollection;
  QString remoteId, remoteRevision, mimeType;
  QSet<QByteArray> itemParts, addedFlags, removedFlags;
  QVariantList items;

  QStringList list;

  quint64 sizeAndVersion;
  stream >> sizeAndVersion;

  const quint64 size = sizeAndVersion & s_sizeMask;
  const quint64 version = (sizeAndVersion & s_versionMask) >> 32;

  quint64 startOffset = 0;
  if ( version >= 1 ) {
    stream >> startOffset;
  }

  for ( quint64 i = 0; i < size && !stream.atEnd(); ++i ) {
    stream >> sessionId;
    stream >> type;
    stream >> operation;
    stream >> entityCnt;
    for ( int j = 0; j < entityCnt; ++j ) {
      QVariantMap map;
      stream >> map[ QLatin1String( "uid" ) ];
      stream >> map[ QLatin1String( "remoteId" ) ];
      stream >> map[ QLatin1String( "remoteRevision" ) ];
      stream >> map[ QLatin1String( "mimeType" ) ];
      items << map;
    }
    stream >> resource;
    stream >> destResource;
    stream >> parentCollection;
    stream >> parentDestCollection;
    stream >> itemParts;
    stream >> addedFlags;
    stream >> removedFlags;

    if ( i < startOffset )
        continue;

    QString typeString;
    switch ( type ) {
    case NotificationMessageV2::Collections:
      typeString = QLatin1String( "Collections" );
      break;
    case NotificationMessageV2::Items:
      typeString = QLatin1String( "Items" );
      break;
    default:
      typeString = QLatin1String( "InvalidType" );
      break;
    };

    QString operationString;
    switch ( operation ) {
    case NotificationMessageV2::Add:
      operationString = QLatin1String( "Add" );
      break;
    case NotificationMessageV2::Modify:
      operationString = QLatin1String( "Modify" );
      break;
    case NotificationMessageV2::ModifyFlags:
      operationString = QLatin1String( "ModifyFlags" );
      break;
    case NotificationMessageV2::Move:
      operationString = QLatin1String( "Move" );
      break;
    case NotificationMessageV2::Remove:
      operationString = QLatin1String( "Remove" );
      break;
    case NotificationMessageV2::Link:
      operationString = QLatin1String( "Link" );
      break;
    case NotificationMessageV2::Unlink:
      operationString = QLatin1String( "Unlink" );
      break;
    case NotificationMessageV2::Subscribe:
      operationString = QLatin1String( "Subscribe" );
      break;
    case NotificationMessageV2::Unsubscribe:
      operationString = QLatin1String( "Unsubscribe" );
      break;
    default:
      operationString = QLatin1String( "InvalidOp" );
      break;
    };

    QStringList itemPartsList, addedFlagsList, removedFlagsList;
    foreach( const QByteArray &b, itemParts )
      itemPartsList.push_back( QString::fromLatin1(b) );
    foreach( const QByteArray &b, addedFlags )
      addedFlagsList.push_back( QString::fromLatin1(b) );
    foreach( const QByteArray &b, removedFlags )
      removedFlagsList.push_back( QString::fromLatin1(b) );

    const QString entry = QString::fromLatin1("session=%1 type=%2 operation=%3 items=%4 resource=%5 destResource=%6 parentCollection=%7 parentDestCollection=%8 itemParts=%9 addedFlags=%10 removedFlags=%11")
                          .arg( QString::fromLatin1( sessionId ) )
                          .arg( typeString )
                          .arg( operationString )
                          .arg( QVariant(items).toString() )
                          .arg( QString::fromLatin1( resource ) )
                          .arg( QString::fromLatin1( destResource ) )
                          .arg( parentCollection )
                          .arg( parentDestCollection )
                          .arg( itemPartsList.join(QLatin1String(", " )) )
                          .arg( addedFlagsList.join(QLatin1String( ", " )) )
                          .arg( removedFlagsList.join(QLatin1String( ", " )) );

    result += entry + QLatin1Char('\n');
  }

  return result;
}

void ChangeRecorderPrivate::addToStream(QDataStream &stream, const IdleNotification &msg)
{
  /*
  stream << msg.sessionId();
  stream << int(msg.type());
  stream << int(msg.operation());
  stream << msg.entities().count();
  Q_FOREACH( const NotificationMessageV2::Entity &entity, msg.entities() ) {
    stream << quint64(entity.id);
    stream << entity.remoteId;
    stream << entity.remoteRevision;
    stream << entity.mimeType;
  }
  stream << msg.resource();
  stream << msg.destinationResource();
  stream << quint64(msg.parentCollection());
  stream << quint64(msg.parentDestCollection());
  stream << msg.itemParts();
  stream << msg.addedFlags();
  stream << msg.removedFlags();
  */
}

void ChangeRecorderPrivate::writeStartOffset()
{
  if ( !settings )
    return;

  QFile file( notificationsFileName() );
  if ( !file.open( QIODevice::ReadWrite ) ) {
    qWarning() << "Could not update notifications in file" << file.fileName();
    return;
  }

  // Skip "countAndVersion"
  file.seek( 8 );

  //kDebug() << "Writing start offset=" << m_startOffset;

  QDataStream stream( &file );
  stream.setVersion( QDataStream::Qt_4_6 );
  stream << static_cast<quint64>(m_startOffset);

  // Everything else stays unchanged
}

void ChangeRecorderPrivate::saveNotifications()
{
  if ( !settings )
    return;

  QFile file( notificationsFileName() );
  QFileInfo info( file );
  if ( !QFile::exists( info.absolutePath() ) ) {
    QDir dir;
    dir.mkpath( info.absolutePath() );
  }
  if ( !file.open( QIODevice::WriteOnly ) ) {
    qWarning() << "Could not save notifications to file" << file.fileName();
    return;
  }
  saveTo(&file);
  m_needFullSave = false;
  m_startOffset = 0;
}

void ChangeRecorderPrivate::saveTo(QIODevice *device)
{
  // Version 0 of this file format was writing a quint64 count, followed by the notifications.
  // Version 1 bundles a version number into that quint64, to be able to detect a version number at load time.

  const quint64 countAndVersion = static_cast<quint64>(pendingNotifications.count()) | s_currentVersion;

  QDataStream stream( device );
  stream.setVersion( QDataStream::Qt_4_6 );

  stream << countAndVersion;
  stream << quint64(0); // no start offset

  //kDebug() << "Saving" << pendingNotifications.count() << "notifications (full save)";

  for ( int i = 0; i < pendingNotifications.count(); ++i ) {
    const IdleNotification msg = pendingNotifications.at( i );
    addToStream( stream, msg );
  }
}

void ChangeRecorderPrivate::notificationsEnqueued( int count )
{
  // Just to ensure the contract is kept, and these two methods are always properly called.
  if (enableChangeRecording) {
    m_lastKnownNotificationsCount += count;
    if ( m_lastKnownNotificationsCount != pendingNotifications.count() ) {
      kWarning() << this << "The number of pending notifications changed without telling us! Expected"
                 << m_lastKnownNotificationsCount << "but got" << pendingNotifications.count()
                 << "Caller just added" << count;
      Q_ASSERT( pendingNotifications.count() == m_lastKnownNotificationsCount );
    }

    saveNotifications();
  }
}

void ChangeRecorderPrivate::dequeueNotification()
{
  pendingNotifications.dequeue();

  if (enableChangeRecording) {

    Q_ASSERT( pendingNotifications.count() == m_lastKnownNotificationsCount - 1 );
    --m_lastKnownNotificationsCount;

    if ( m_needFullSave || pendingNotifications.isEmpty() ) {
      saveNotifications();
    } else {
      ++m_startOffset;
      writeStartOffset();
    }
  }
}

void ChangeRecorderPrivate::notificationsErased()
{
  if (enableChangeRecording) {
    m_lastKnownNotificationsCount = pendingNotifications.count();
    m_needFullSave = true;
    saveNotifications();
  }
}

void ChangeRecorderPrivate::notificationsLoaded()
{
  m_lastKnownNotificationsCount = pendingNotifications.count();
  m_startOffset = 0;
}
