/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
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
  //TODO 3 new keys are:
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // array mt_tb_ping_urls, the list of TrackBack ping URLs for this entry
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
  map["mt_excerpt"] = posting->summary();
  map["mt_keywords"] = posting->tags(); // TODO some convertion needed?
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
  //TODO 3 new keys are:
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // array mt_tb_ping_urls, the list of TrackBack ping URLs for this entry
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
  map["mt_excerpt"] = posting->summary();
  map["mt_keywords"] = posting->tags(); // TODO some convertion needed?
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
    Q_D(MovableType);
    kDebug(5323) << "Fetching List of Posts...";
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( number );
    d->mXmlRpcClient->call(
      "metaWeblog.getRecentPosts", args,
      this, SLOT( slotListRecentPostings( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT( slotError( int, const QString&, const QVariant& ) ), QVariant( number ) );
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

  // FIXME: integrate error handling
  if ( !post ) {
    return false;
  }
  QStringList mapkeys = postInfo.keys();
  kDebug(5323) << endl << "Keys:" << mapkeys.join( ", " );
  kDebug(5323) << endl;

  KDateTime dt =
    KDateTime( postInfo["dateCreated"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt );
  }

  dt =
    KDateTime( postInfo["lastModified"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt );
  }

  post->setPostingId( postInfo["postid"].toString() );

  QString title( postInfo["title"].toString() );
  QString description( postInfo["description"].toString() );
  QStringList categories( postInfo["categories"].toStringList() );
  //TODO 2 new keys are:
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  post->setTitle( title );
  post->setContent( description );
  post->setCommentAllowed( (bool)postInfo["mt_allow_comments"].toInt() );
  post->setTrackBackAllowed( (bool)postInfo["mt_allow_pings"].toInt() );
  post->setSummary( postInfo["mt_excerpt"].toString() );
  post->setTags( postInfo["mt_keywords"].toString() );

  if ( !categories.isEmpty() ){
    kDebug(5323) << "Categories:" << categories;
    post->setCategories( categories );
  }
  return true;
}

void MovableTypePrivate::slotCreatePosting(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q(MovableType);

  KBlog::BlogPosting* posting = callMap[ id.toInt() ];
  callMap.remove( id.toInt() );

  kDebug(5323) << "MovableType::slotCreatePosting";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::String ) {
    kDebug(5323) << "Could not read the postingId, not a string.";
    emit q->error( MovableType::ParsingError,
                       i18n( "Could not read the postingId, not a string." ), posting );
  } else {
     posting->setPostingId( result[0].toString() );
     posting->setStatus( BlogPosting::Created );
     emit q->createdPosting( posting );
    kDebug(5323) << "emitting createdPosting(" << result[0].toString() << ")";
  }
}

void MovableTypePrivate::slotError( int number,
    const QString &errorString, const QVariant &id )
{
  Q_Q(MovableType);
  Q_UNUSED( number );
  Q_UNUSED( id );

  emit q->error( MovableType::XmlRpc, errorString );
}

void MovableTypePrivate::slotFetchPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q(MovableType);

  KBlog::BlogPosting* posting = callMap[ id.toInt() ];
  callMap.remove( id.toInt() );

  kDebug(5323) << "MovableType::slotFetchPosting";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Map ) {
    kDebug(5323) << "Could not fetch posting out of the result from the server.";
    emit q->error( MovableType::ParsingError,
                       i18n( "Could not fetch posting out of the "
                             "result from the server." ), posting );
  } else {
    const QMap<QString, QVariant> postInfo = result[0].toMap();
    if ( readPostingFromMap( posting, postInfo ) ) {
      kDebug(5323) << "Emitting fetchedPosting( posting.postingId()="
                   << posting->postingId() << ");";
      posting->setStatus( BlogPosting::Fetched );
      emit q->fetchedPosting( posting );
    } else {
      kDebug(5323) << "readPostingFromMap failed!";
      emit q->error( MovableType::ParsingError,
                         i18n( "Could not read posting." ), posting );
    }
  }
}

void MovableTypePrivate::slotListRecentPostings(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q(MovableType);

  int count = id.toInt();

  QList <BlogPosting> fetchedPostingList;

  kDebug(5323) << "MovableType::slotListRecentPostings";
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::List ) {
    kDebug(5323) << "Could not fetch list of postings out of the"
                 << "result from the server.";
    emit q->error( MovableType::ParsingError,
                        i18n( "Could not fetch list of postings out of the "
                              "result from the server." ) );
  } else {
    const QList<QVariant> postReceived = result[0].toList();
    QList<QVariant>::ConstIterator it = postReceived.begin();
    QList<QVariant>::ConstIterator end = postReceived.end();
    for ( ; it != end; ++it ) {
      BlogPosting posting;
      kDebug(5323) << "MIDDLE:" << ( *it ).typeName();
      const QMap<QString, QVariant> postInfo = ( *it ).toMap();
      if ( readPostingFromMap( &posting, postInfo ) ) {
        kDebug(5323) << "Emitting listedPosting( posting.postingId()="
                     << posting.postingId() << ");";
        fetchedPostingList << posting;
      } else {
        kDebug(5323) << "readPostingFromMap failed!";
        emit q->error( MovableType::ParsingError, i18n( "Could not read posting." ) );
      }
      if( --count == 0 ) break;
    }
  } //FIXME should we emit here? (see below, too)
  kDebug(5323) << "Emitting listRecentPostingsFinished()";
  emit q->listedRecentPostings( fetchedPostingList );
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
  Q_Q(MovableType);

  KBlog::BlogPosting* posting = callMap[ id.toInt() ];
  callMap.remove( id.toInt() );

  kDebug(5323) << "MovableType::slotModifyPosting";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Bool ) {
    kDebug(5323) << "Could not read the result, not a boolean.";
    emit q->error( MovableType::ParsingError,
                       i18n( "Could not read the result, not a boolean." ), posting );
  } else {
    posting->setStatus( BlogPosting::Modified );
    emit q->modifiedPosting( posting );
    kDebug(5323) << "emitting modifiedPosting()";
  }
}

#include "movabletype.moc"
