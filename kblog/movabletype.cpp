/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  Copyright (c) 2007-2008 Mike Arthur <mike@mikearthur.co.uk>

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
#include "blogpost.h"

#include <kxmlrpcclient/client.h>

#include <KDebug>
#include <KLocale>
#include <KDateTime>

#include <QtCore/QStringList>

using namespace KBlog;

MovableType::MovableType( const KUrl &server, QObject *parent )
  : MetaWeblog( server, *new MovableTypePrivate, parent )
{
  kDebug() << "MovableType()";
}

MovableType::MovableType( const KUrl &server, MovableTypePrivate &dd,
                        QObject *parent )
  : MetaWeblog( server, dd, parent )
{
  kDebug() << "MovableType()";
}

MovableType::~MovableType()
{
  kDebug() << "~MovableType()";
}

QString MovableType::interfaceName() const
{
  return QLatin1String( "Movable Type" );
}

void MovableType::listRecentPosts( int number )
{
    Q_D( MovableType );
    kDebug() << "Fetching List of Posts...";
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( number );
    d->mXmlRpcClient->call(
      "metaWeblog.getRecentPosts", args,
      this, SLOT(slotListRecentPosts(const QList<QVariant>&,const QVariant&)),
      this, SLOT(slotError(int,const QString&,const QVariant&)),
      QVariant( number ) );
}

void MovableType::listTrackBackPings( KBlog::BlogPost *post )
{
  Q_D( MovableType );
  kDebug() << "List trackback pings...";
  QList<QVariant> args;
  args << QVariant( post->postId() );
  unsigned int i = d->mCallCounter++;
  d->mCallMap[ i ] = post;
  d->mXmlRpcClient->call(
    "mt.getTrackbackPings", args,
    this, SLOT(slotListTrackbackPings(const QList<QVariant>&,const QVariant&)),
    this, SLOT(slotError(int,const QString&,const QVariant&)),
    QVariant( i ) );
}

MovableTypePrivate::MovableTypePrivate()
{
}

MovableTypePrivate::~MovableTypePrivate()
{
  kDebug() << "~MovableTypePrivate()";
}

QList<QVariant> MovableTypePrivate::defaultArgs( const QString &id )
{
  Q_Q( MovableType );
  QList<QVariant> args;
  if( !id.isEmpty() ) {
    args << QVariant( id );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

bool MovableTypePrivate::readPostFromMap( BlogPost *post, const QMap<QString, QVariant> &postInfo )
{

  // FIXME: integrate error handling
  kDebug() << "readPostFromMap()";
  if ( !post ) {
    return false;
  }
  QStringList mapkeys = postInfo.keys();
  kDebug() << endl << "Keys:" << mapkeys.join( ", " );
  kDebug() << endl;

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

  post->setPostId( postInfo["postid"].toString() );

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
  post->setTags( postInfo["mt_keywords"].toStringList() );
  post->setLink( postInfo["link"].toString() );
  post->setPermaLink( postInfo["permaLink"].toString() );

  if ( !categories.isEmpty() ){
    kDebug() << "Categories:" << categories;
    post->setCategories( categories );
  }
  return true;
}

void MovableTypePrivate::slotListTrackBackPings(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( MovableType );
  kDebug() << "slotTrackbackPings()";
  BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );
  QList<QMap<QString,QString> > trackBackList;
  if ( result[0].type() != QVariant::List ) {
    kError() << "Could not fetch list of trackback pings out of the"
                 << "result from the server.";
    emit q->error( MovableType::ParsingError,
                   i18n( "Could not fetch list of trackback pings out of the "
                         "result from the server." ) );
    return;
  }
  const QList<QVariant> trackBackReceived = result[0].toList();
  QList<QVariant>::ConstIterator it = trackBackReceived.begin();
  QList<QVariant>::ConstIterator end = trackBackReceived.end();
  for ( ; it != end; ++it ) {
    QMap<QString,QString> tping;
    kDebug() << "MIDDLE:" << ( *it ).typeName();
    const QMap<QString, QVariant> trackBackInfo = ( *it ).toMap();
    tping[ "title" ] = trackBackInfo[ "pingTitle"].toString();
    tping[ "url" ] = trackBackInfo[ "pingURL"].toString();
    tping[ "ip" ] = trackBackInfo[ "pingIP"].toString();
    trackBackList << tping;
  }
  kDebug() << "Emitting listedTrackBackPings()";
  emit q->listedTrackBackPings( post, trackBackList );
}

bool MovableTypePrivate::readArgsFromPost( QList<QVariant> *args, const BlogPost &post )
{
  //TODO 3 new keys are:
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // array mt_tb_ping_urls, the list of TrackBack ping URLs for this entry
  if ( !args ) {
    return false;
  }
  QMap<QString, QVariant> map;
  map["categories"] = post.categories();
  map["description"] = post.content();
  map["title"] = post.title();
  map["dateCreated"] = post.creationDateTime().toUtc().dateTime();
  map["mt_allow_comments"] = (int)post.isCommentAllowed();
  map["mt_allow_pings"] = (int)post.isTrackBackAllowed();
  map["mt_excerpt"] = post.summary();
  map["mt_keywords"] = post.tags().join(",");
  //map["mt_tb_ping_urls"] check for that, i think this should only be done on the server.
  *args << map;
  *args << QVariant( !post.isPrivate() );
  return true;
}

#include "movabletype.moc"
