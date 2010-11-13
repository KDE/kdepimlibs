/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#include "itemmodifyjob.h"
#include "itemmodifyjob_p.h"

#include "collection.h"
#include "conflicthandling/conflicthandler_p.h"
#include "entity_p.h"
#include "imapparser_p.h"
#include "item_p.h"
#include "itemserializer_p.h"
#include "job_p.h"
#include "protocolhelper_p.h"

#include <kdebug.h>

using namespace Akonadi;

ItemModifyJobPrivate::ItemModifyJobPrivate( ItemModifyJob *parent, const Item &item )
  : JobPrivate( parent ),
    mItem( item ),
    mRevCheck( true ),
    mIgnorePayload( false ),
    mAutomaticConflictHandlingEnabled( true )
{
  mParts = mItem.loadedPayloadParts();
}

void ItemModifyJobPrivate::setClean()
{
  mOperations.insert( Dirty );
}

QByteArray ItemModifyJobPrivate::nextPartHeader()
{
  QByteArray command;
  if ( !mParts.isEmpty() ) {
    QSetIterator<QByteArray> it( mParts );
    const QByteArray label = it.next();
    mParts.remove( label );

    mPendingData.clear();
    int version = 0;
    ItemSerializer::serialize( mItem, label, mPendingData, version );
    command += ' ' + ProtocolHelper::encodePartIdentifier( ProtocolHelper::PartPayload, label, version );
    command += ".SILENT";
    if ( mPendingData.size() > 0 ) {
      command += " {" + QByteArray::number( mPendingData.size() ) + "}\n";
    } else {
      if ( mPendingData.isNull() )
        command += " NIL";
      else
        command += " \"\"";
      command += nextPartHeader();
    }
  } else {
    command += ")\n";
  }
  return command;
}

void ItemModifyJobPrivate::conflictResolved()
{
  Q_Q( ItemModifyJob );

  q->setError( KJob::NoError );
  q->setErrorText( QString() );
  q->emitResult();
}

void ItemModifyJobPrivate::conflictResolveError( const QString &message )
{
  Q_Q( ItemModifyJob );

  q->setErrorText( q->errorText() + message );
  q->emitResult();
}

void ItemModifyJobPrivate::doUpdateItemRevision( Akonadi::Item::Id itemId, int oldRevision, int newRevision )
{
  if ( mItem.id() == itemId && mItem.revision() == oldRevision )
    mItem.setRevision( newRevision );
}


ItemModifyJob::ItemModifyJob( const Item &item, QObject * parent )
  : Job( new ItemModifyJobPrivate( this, item ), parent )
{
  Q_D( ItemModifyJob );

  d->mOperations.insert( ItemModifyJobPrivate::RemoteId );
  d->mOperations.insert( ItemModifyJobPrivate::RemoteRevision );
}

ItemModifyJob::~ItemModifyJob()
{
}

void ItemModifyJob::doStart()
{
  Q_D( ItemModifyJob );

  QList<QByteArray> changes;
  foreach ( int op, d->mOperations ) {
    switch ( op ) {
      case ItemModifyJobPrivate::RemoteId:
        if ( !d->mItem.remoteId().isNull() ) {
          changes << "REMOTEID.SILENT";
          changes << ImapParser::quote( d->mItem.remoteId().toUtf8() );
        }
        break;
      case ItemModifyJobPrivate::RemoteRevision:
        if ( !d->mItem.remoteRevision().isNull() ) {
          changes << "REMOTEREVISION.SILENT";
          changes << ImapParser::quote( d->mItem.remoteRevision().toUtf8() );
        }
        break;
      case ItemModifyJobPrivate::Dirty:
        changes << "DIRTY.SILENT";
        changes << "false";
        break;
    }
  }

  if ( d->mItem.d_func()->mClearPayload )
    changes << "INVALIDATECACHE.SILENT";

  if ( d->mItem.d_func()->mFlagsOverwritten ) {
    changes << "FLAGS.SILENT";
    changes << '(' + ImapParser::join( d->mItem.flags(), " " ) + ')';
  } else {
    if ( !d->mItem.d_func()->mAddedFlags.isEmpty() ) {
      changes << "+FLAGS.SILENT";
      changes << '(' + ImapParser::join( d->mItem.d_func()->mAddedFlags, " " ) + ')';
    }
    if ( !d->mItem.d_func()->mDeletedFlags.isEmpty() ) {
      changes << "-FLAGS.SILENT";
      changes << '(' + ImapParser::join( d->mItem.d_func()->mDeletedFlags, " " ) + ')';
    }
  }

  if ( !d->mItem.d_func()->mDeletedAttributes.isEmpty() ) {
    changes << "-PARTS.SILENT";
    QList<QByteArray> attrs;
    foreach ( const QByteArray &attr, d->mItem.d_func()->mDeletedAttributes )
      attrs << ProtocolHelper::encodePartIdentifier( ProtocolHelper::PartAttribute, attr );
    changes << '(' + ImapParser::join( attrs, " " ) + ')';
  }

  // nothing to do
  if ( changes.isEmpty() && d->mParts.isEmpty() && d->mItem.attributes().isEmpty() ) {
    emitResult();
    return;
  }

  d->mTag = d->newTag();
  QByteArray command = d->mTag;
  command += " UID STORE " + QByteArray::number( d->mItem.id() ) + ' ';
  if ( !d->mRevCheck || d->mItem.revision() < 0 ) {
    command += "NOREV ";
  } else {
    command += "REV " + QByteArray::number( d->mItem.revision() ) + ' ';
  }

  if ( d->mItem.d_func()->mSizeChanged )
    command += "SIZE " + QByteArray::number( d->mItem.size() );

  command += " (" + ImapParser::join( changes, " " );
  const QByteArray attrs = ProtocolHelper::attributesToByteArray( d->mItem, true );
  if ( !attrs.isEmpty() )
    command += ' ' + attrs;
  command += d->nextPartHeader();
  d->writeData( command );
  d->newTag(); // hack to circumvent automatic response handling
}

void ItemModifyJob::doHandleResponse(const QByteArray &_tag, const QByteArray & data)
{
  Q_D( ItemModifyJob );

  if ( _tag == "+" ) { // ready for literal data
    d->writeData( d->mPendingData );
    d->writeData( d->nextPartHeader() );
    return;
  }
  if ( _tag == d->mTag ) {
    if ( data.startsWith( "OK" ) ) { //krazy:exclude=strings
      QDateTime modificationDateTime;
      int dateTimePos = data.indexOf( "DATETIME" );
      if ( dateTimePos != -1 ) {
        int resultPos = ImapParser::parseDateTime( data, modificationDateTime, dateTimePos + 8 );
        if ( resultPos == (dateTimePos + 8) ) {
          kDebug() << "Invalid DATETIME response to STORE command: " << _tag << data;
        }
      }

      if ( d->mItem.modificationTime() != modificationDateTime )
      {
        // increase item revision of own copy of item
        const int oldRevision = d->mItem.revision();
        if ( oldRevision >= 0 ) {
          d->itemRevisionChanged( d->mItem.id(), oldRevision, oldRevision + 1 );
          d->mItem.setRevision( oldRevision + 1 );
        }
        d->mItem.setModificationTime( modificationDateTime );
      } else {
        kDebug() << "No changes on item" << d->mItem.id();
      }
      d->mItem.d_ptr->resetChangeLog();
    } else {
      setError( Unknown );
      setErrorText( QString::fromUtf8( data ) );

      if ( data.contains( "[LLCONFLICT]" ) ) {
        if ( d->mAutomaticConflictHandlingEnabled ) {
          ConflictHandler *handler = new ConflictHandler( ConflictHandler::LocalLocalConflict, this );
          handler->setConflictingItems( d->mItem, d->mItem );
          connect( handler, SIGNAL( conflictResolved() ), SLOT( conflictResolved() ) );
          connect( handler, SIGNAL( error( const QString& ) ), SLOT( conflictResolveError( const QString& ) ) );

          QMetaObject::invokeMethod( handler, "start", Qt::QueuedConnection );
          return;
        }
      }
    }
    emitResult();
    return;
  }
  kDebug() << "Unhandled response: " << _tag << data;
}

void ItemModifyJob::setIgnorePayload( bool ignore )
{
  Q_D( ItemModifyJob );

  if ( d->mIgnorePayload == ignore )
    return;

  d->mIgnorePayload = ignore;
  if ( d->mIgnorePayload )
    d->mParts = QSet<QByteArray>();
  else {
    Q_ASSERT( !d->mItem.mimeType().isEmpty() );
    d->mParts = d->mItem.loadedPayloadParts();
  }
}

bool ItemModifyJob::ignorePayload() const
{
  Q_D( const ItemModifyJob );

  return d->mIgnorePayload;
}

void ItemModifyJob::disableRevisionCheck()
{
  Q_D( ItemModifyJob );

  d->mRevCheck = false;
}

void ItemModifyJob::disableAutomaticConflictHandling()
{
  Q_D( ItemModifyJob );

  d->mAutomaticConflictHandlingEnabled = false;
}

Item ItemModifyJob::item() const
{
  Q_D( const ItemModifyJob );

  return d->mItem;
}

#include "itemmodifyjob.moc"
