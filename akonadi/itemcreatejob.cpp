/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2007        Robert Zwerus <arzie@dds.nl>

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

#include "itemcreatejob.h"

#include "collection.h"
#include "imapparser_p.h"
#include "item.h"
#include "itemserializer_p.h"
#include "job_p.h"
#include "protocolhelper_p.h"
#include "gid/gidextractor_p.h"

#include <QtCore/QDateTime>

#include <kdebug.h>

using namespace Akonadi;

class Akonadi::ItemCreateJobPrivate : public JobPrivate
{
  public:
    ItemCreateJobPrivate( ItemCreateJob *parent )
      : JobPrivate( parent )
    {
    }

    QByteArray nextPartHeader();

    Collection mCollection;
    Item mItem;
    QSet<QByteArray> mParts;
    Item::Id mUid;
    QDateTime mDatetime;
    QByteArray mPendingData;
};

QByteArray ItemCreateJobPrivate::nextPartHeader()
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

ItemCreateJob::ItemCreateJob( const Item &item, const Collection &collection, QObject * parent )
  : Job( new ItemCreateJobPrivate( this ), parent )
{
  Q_D( ItemCreateJob );

  Q_ASSERT( !item.mimeType().isEmpty() );
  d->mItem = item;
  d->mParts = d->mItem.loadedPayloadParts();
  d->mCollection = collection;
}

ItemCreateJob::~ItemCreateJob()
{
}

void ItemCreateJob::doStart()
{
  Q_D( ItemCreateJob );

  QByteArray remoteId;

  QList<QByteArray> flags;
  flags.append( "\\MimeType[" + d->mItem.mimeType().toLatin1() + ']' );
  const QString gid = GidExtractor::getGid( d->mItem );
  if ( !gid.isNull() ) {
    flags.append( ImapParser::quote( "\\Gid[" + gid.toUtf8() + ']' ) );
  }
  if ( !d->mItem.remoteId().isEmpty() )
    flags.append( ImapParser::quote( "\\RemoteId[" + d->mItem.remoteId().toUtf8() + ']' ) );
  if ( !d->mItem.remoteRevision().isEmpty() )
    flags.append( ImapParser::quote( "\\RemoteRevision[" + d->mItem.remoteRevision().toUtf8() + ']' ) );
  flags += d->mItem.flags().toList();

  QByteArray command = d->newTag() + " X-AKAPPEND " + QByteArray::number( d->mCollection.id() )
      + ' ' + QByteArray::number( d->mItem.size() )
      + " (" + ImapParser::join( flags, " " ) + ")"
      + " ("; // list of parts
  const QByteArray attrs = ProtocolHelper::attributesToByteArray( d->mItem, true );
  if ( !attrs.isEmpty() ) {
    command += attrs;
  }

  command += d->nextPartHeader();

  d->writeData( command );
}

void ItemCreateJob::doHandleResponse( const QByteArray & tag, const QByteArray & data )
{
  Q_D( ItemCreateJob );

  if ( tag == "+" ) { // ready for literal data
    d->writeData( d->mPendingData );
    d->writeData( d->nextPartHeader() );
    return;
  }
  if ( tag == d->tag() ) {
    int uidNextPos = data.indexOf( "UIDNEXT" );
    if ( uidNextPos != -1 ) {
      bool ok = false;
      ImapParser::parseNumber( data, d->mUid, &ok, uidNextPos + 7 );
      if ( !ok ) {
        kDebug() << "Invalid UIDNEXT response to APPEND command: "
                 << tag << data;
      }
    }
    int dateTimePos = data.indexOf( "DATETIME" );
    if ( dateTimePos != -1 ) {
      int resultPos = ImapParser::parseDateTime( data, d->mDatetime, dateTimePos + 8 );
      if ( resultPos == (dateTimePos + 8) ) {
        kDebug() << "Invalid DATETIME response to APPEND command: "
                 << tag << data;
      }
    }
  }
}

Item ItemCreateJob::item() const
{
  Q_D( const ItemCreateJob );

  if ( d->mUid == 0 )
    return Item();

  Item item( d->mItem );
  item.setId( d->mUid );
  item.setRevision( 0 );
  item.setModificationTime( d->mDatetime );
  item.setParentCollection( d->mCollection );
  item.setStorageCollectionId( d->mCollection.id() );

  return item;
}

