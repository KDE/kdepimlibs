/*
  This file is part of the kblog library.

  Copyright (c) 2007 Mike Arthur <mike@mikearthur.co.uk>

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

#include "movabletype.h"
#include "movabletype_p.h"
#include "blogposting.h"

#include <kxmlrpcclient/client.h>

#include <KDebug>
#include <KLocale>
#include <KDateTime>

#include <QtCore/QStringList>

using namespace KBlog;

MovableType::MovableType( const KUrl &server, QObject *parent )
  : MetaWeblog( server, *new MovableTypePrivate, parent )
{
  setUrl( server );
}

MovableType::~MovableType()
{
}

void MovableType::createPosting( KBlog::BlogPosting *posting )
{
  Q_D(MovableType);
  if ( !posting ) {
    kDebug(5323) << "MovableType::createPosting: posting is a null pointer";
    emit error ( Other, i18n( "Posting is a null pointer." ) );
    return;
  }
  unsigned int i = d->callCounter++;
  d->callMap[ i ] = posting;
  kDebug(5323) << "Creating new Posting with blogId" << blogId();
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  QMap<QString, QVariant> map;
  map["categories"] = posting->categories();
  map["description"] = posting->content();
  map["title"] = posting->title();
  map["dateCreated"] = posting->creationDateTime().toUtc().dateTime();
  map["mt_allow_comments"] = (int)posting->isCommentAllowed();
  map["mt_allow_pings"] = (int)posting->isTrackBackAllowed();
  map["mt_text_more"] = posting->summary();
  //map["mt_tb_ping_urls"] check for that, i think this should only be done on the server.
  args << map;
  args << QVariant( posting->isPublished() );
  d->mXmlRpcClient->call (
    "metaWeblog.newPost", args,
    this, SLOT( slotCreatePosting( const QList<QVariant>&, const QVariant& ) ),
    this, SLOT ( slotError( int, const QString&, const QVariant& ) ), QVariant( i ) );
}

void MovableType::modifyPosting( KBlog::BlogPosting *posting )
{
  Q_D(MovableType);
  if ( !posting ) {
    kDebug(5323) << "MovableType::modifyPosting: posting is a null pointer";
    emit error ( Other, i18n( "Posting is a null pointer." ) );
    return;
  }
  unsigned int i = d->callCounter++;
  d->callMap[ i ] = posting;
  kDebug(5323) << "Uploading Posting with postId" << posting->postingId();

  QList<QVariant> args( d->defaultArgs( posting->postingId() ) );
  QMap<QString, QVariant> map;
  map["categories"] = posting->categories();
  map["description"] = posting->content();
  map["title"] = posting->title();
  map["lastModified"] = posting->modificationDateTime().toUtc().dateTime();
  map["mt_allow_comments"] = (int)posting->isCommentAllowed();
  map["mt_allow_pings"] = (int)posting->isTrackBackAllowed();
  map["mt_text_more"] = posting->summary();
  args << map;
  args << QVariant( posting->isPublished() );
  d->mXmlRpcClient->call(
    "metaWeblog.editPost", args,
     this, SLOT( slotModifyPosting( const QList<QVariant>&, const QVariant& ) ),
     this, SLOT ( slotError( int, const QString&, const QVariant& ) ), QVariant( i ) );
}

void MovableType::fetchPosting( KBlog::BlogPosting *posting )
{
  Q_D(MovableType);
  if ( !posting ) {
    kDebug(5323) << "MovableType::fetchPosting: posting is a null pointer";
    emit error ( Other, i18n( "Posting is a null pointer." ) );
    return;
  }
  unsigned int i = d->callCounter++;
  d->callMap[ i ] = posting;
  kDebug(5323) << "Fetching Posting with url" << posting->postingId();
  QList<QVariant> args( d->defaultArgs( posting->postingId() ) );
  d->mXmlRpcClient->call(
    "metaWeblog.getPost", args,
    this, SLOT( slotFetchPosting( const QList<QVariant>&, const QVariant& ) ),
    this, SLOT( slotError( int, const QString&, const QVariant& ) ), QVariant( i ) );
}

QString MovableType::interfaceName() const
{
  return QLatin1String( "Movable Type" );
}

void MovableType::listRecentPostings( const int number )
{
  //TODO
}

void MovableType::listTrackbackPings( KBlog::BlogPosting *posting ) {
  //TODO
  /*
  Q_D(MovableType);
  d->mXmlRpcClient->call( "mt.getTracebackPings", args,
    d, SLOT( slotListTrackbackPings(
              const QList<QVariant>&, const QVariant& ) ),
    d, SLOT( slotError( int, const QString&, const QVariant& ) ) );
  */
}

MovableTypePrivate::MovableTypePrivate()
{
  mXmlRpcClient = 0;
}

MovableTypePrivate::~MovableTypePrivate()
{
  delete mXmlRpcClient;
}

QList<QVariant> MovableTypePrivate::defaultArgs( const QString &id )
{
  Q_Q(MovableType);
  QList<QVariant> args;

  if ( id.toInt() ) {
    args << QVariant( id.toInt() );
  }
  if ( !id.toInt() && !id.isNull() ){
    args << QVariant( id );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

bool MovableTypePrivate::readPostingFromMap(
    BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  //TODO
  return false;
}

void MovableTypePrivate::slotCreatePosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO 7 new keys are:
  // int mt_allow_comments: the value for the allow_comments field
  // int mt_allow_pings, the value for the allow_pings field
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // String mt_excerpt, the value for the excerpt field
  // String mt_keywords, the value for the keywords field
  // array mt_tb_ping_urls, the list of TrackBack ping URLs for this entry
}

void MovableTypePrivate::slotError( int number,
    const QString &errorString, const QVariant &id )
{
  //TODO
}

void MovableTypePrivate::slotFetchPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO 6 new keys are:
  // int mt_allow_comments: the value for the allow_comments field
  // int mt_allow_pings, the value for the allow_pings field
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // String mt_excerpt, the value for the excerpt field
  // String mt_keywords, the value for the keywords field
}

void MovableTypePrivate::slotListRecentPostings(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO 6 new keys are:
  // int mt_allow_comments: the value for the allow_comments field
  // int mt_allow_pings, the value for the allow_pings field
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // String mt_excerpt, the value for the excerpt field
  // String mt_keywords, the value for the keywords field
}

void MovableTypePrivate::slotListTrackbackPings(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO Contains:
  // String pingTitle: the title of the entry sent in the ping
  // String pingURL: the URL of the entry
  // String pingIP: the IP address of the host that sent the ping
}

void MovableTypePrivate::slotModifyPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO 5 new keys are:
  // int mt_allow_comments: the value for the allow_comments field
  // int mt_allow_pings, the value for the allow_pings field
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // String mt_excerpt, the value for the excerpt field
  // String mt_keywords, the value for the keywords field
  // array mt_tb_ping_urls, the list of TrackBack ping URLs for this entry
}

#include "movabletype.moc"
