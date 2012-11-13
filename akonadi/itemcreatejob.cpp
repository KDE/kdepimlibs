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

    Collection mCollection;
    Item mItem;
    QSet<QByteArray> mParts;
    Item::Id mUid;
    QDateTime mDatetime;
    QByteArray mData;
};

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
  if ( !d->mItem.remoteId().isEmpty() )
    flags.append( ImapParser::quote( "\\RemoteId[" + d->mItem.remoteId().toUtf8() + ']' ) );
  if ( !d->mItem.remoteRevision().isEmpty() )
    flags.append( ImapParser::quote( "\\RemoteRevision[" + d->mItem.remoteRevision().toUtf8() + ']' ) );
  flags += d->mItem.flags().toList();

  // switch between a normal APPEND and a multipart X-AKAPPEND, based on the number of parts
  if ( d->mItem.attributes().isEmpty() && ( d->mParts.isEmpty() || (d->mParts.size() == 1 && d->mParts.contains( Item::FullPayload )) ) ) {
    if ( d->mItem.hasPayload() ) {
      int version = 0;
      ItemSerializer::serialize( d->mItem, Item::FullPayload, d->mData, version );
    }
    int dataSize = d->mData.size();

    d->writeData( d->newTag() + " APPEND " + QByteArray::number( d->mCollection.id() )
        + ' ' + QByteArray::number( d->mItem.size() )
        + " (" + ImapParser::join( flags, " " ) + ") {"
        + QByteArray::number( dataSize ) + "}\n" );
  }
  else { // do a multipart X-AKAPPEND
    QByteArray command = d->newTag() + " X-AKAPPEND " + QByteArray::number( d->mCollection.id() )
        + ' ' + QByteArray::number( d->mItem.size() )
        + " (" + ImapParser::join( flags, " " ) + ") ";

    QList<QByteArray> partSpecs;
    int totalSize = 0;
    foreach ( const QByteArray &partName, d->mParts ) {
      QByteArray partData;
      int version = 0;
      ItemSerializer::serialize( d->mItem, partName, partData, version );
      totalSize += partData.size();
      const QByteArray partId = ProtocolHelper::encodePartIdentifier( ProtocolHelper::PartPayload, partName, version );
      partSpecs.append( ImapParser::quote( partId ) + ':' + QByteArray::number( partData.size() ) );
      d->mData += partData;
    }
    foreach ( const Attribute* attr, d->mItem.attributes() ) {
      const QByteArray data = attr->serialized();
      totalSize += data.size();
      const QByteArray partId = ProtocolHelper::encodePartIdentifier( ProtocolHelper::PartAttribute, attr->type() );
      partSpecs.append( ImapParser::quote( partId ) + ':' + QByteArray::number( data.size() ) );
      d->mData += data;
    }
    command += '(' + ImapParser::join( partSpecs, "," ) + ") " +
      '{' + QByteArray::number( totalSize ) + "}\n";

    d->writeData( command );
  }
}

void ItemCreateJob::doHandleResponse( const QByteArray & tag, const QByteArray & data )
{
  Q_D( ItemCreateJob );

  if ( tag == "+" ) { // ready for literal data
    d->writeData( d->mData );
    if ( !d->mData.endsWith( '\n' ) )
      d->writeData( "\n" );
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

