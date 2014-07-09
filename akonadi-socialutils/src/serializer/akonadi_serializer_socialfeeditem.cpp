/*
  Social feed serializer
  Copyright (C) 2012  Martin Klapetek <martin.klapetek@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or (at your
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

#include "akonadi_serializer_socialfeeditem.h"
#include "../socialfeeditem.h"

//#include "akonadi/config-akonadi.h"
#include <item.h>

#include <QtCore/qplugin.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace Akonadi;

bool SocialFeedItemSerializerPlugin::deserialize( Item &item,
                                                  const QByteArray &label,
                                                  QIODevice &data,
                                                  int version )
{
  Q_UNUSED( version );

  if ( label != Item::FullPayload ) {
    return false;
  }

  SocialFeedItem feedItem;

  QJsonDocument json = QJsonDocument::fromJson( data.readAll() );
  QJsonObject map = json.object();

  feedItem.setNetworkString( map.value( QStringLiteral( "networkString" ) ).toString() );
  feedItem.setPostId( map.value( QStringLiteral( "postId" ) ).toString() );
  feedItem.setPostText( map.value( QStringLiteral( "postText" ) ).toString() );
  feedItem.setPostLinkTitle( map.value( QStringLiteral( "postLinkTitle" ) ).toString() );
  feedItem.setPostLink( QUrl::fromUserInput( map.value( QStringLiteral( "postLink" ) ).toString() ) );
  feedItem.setPostImageUrl( QUrl::fromUserInput( map.value( QStringLiteral( "postImageUrl" ) ).toString() ) );
  feedItem.setPostInfo( map.value( QStringLiteral( "postInfo" ) ).toString() );
  feedItem.setUserName( map.value( QStringLiteral( "userName" ) ).toString() );
  feedItem.setUserDisplayName( map.value( QStringLiteral( "userDisplayName" ) ).toString() );
  feedItem.setUserId( map.value( QStringLiteral( "userId" ) ).toString() );
  feedItem.setAvatarUrl( QUrl::fromUserInput( map.value( QStringLiteral( "avatarUrl" ) ).toString() ) );
  feedItem.setPostTime( map.value( QStringLiteral( "postTimeString" ) ).toString(),
                        map.value( QStringLiteral( "postTimeFormat" ) ).toString() );
  feedItem.setShared( map.value( QStringLiteral( "shared" ) ).toBool() );
  feedItem.setSharedFrom( map.value( QStringLiteral( "sharedFrom" ) ).toString() );
  feedItem.setSharedFromId( map.value( QStringLiteral( "sharedFromId" ) ).toString() );
  feedItem.setLiked( map.value( QStringLiteral( "liked" ) ).toBool() );
  feedItem.setItemSourceMap( map.value( QStringLiteral( "itemSourceMap" ) ).toVariant().toMap() );

  if ( map.keys().contains( QStringLiteral( "postReplies" ) ) ) {
    QList<SocialFeedItem> replies;
    QJsonArray repliesArray = map.value( QStringLiteral( "postReplies" ) ).toArray();
    Q_FOREACH ( const QJsonValue &replyData, repliesArray ) {
      QJsonObject reply = replyData.toObject();
      SocialFeedItem postReply;
      postReply.setUserId( reply.value( QStringLiteral( "userId" ) ).toString() );
      postReply.setUserName( reply.value( QStringLiteral( "userName" ) ).toString() );
      postReply.setAvatarUrl( QUrl::fromUserInput( reply.value( QStringLiteral( "userAvatarUrl" ) ).toString() ) );
      postReply.setPostText( reply.value( QStringLiteral( "replyText" ) ).toString() );
//       postReply.setPostTime( reply.value( QStringLiteral( "replyTime" ) ).toString();
      postReply.setPostId( reply.value( QStringLiteral( "replyId" ) ).toString() );
//       postReply.postId        = reply.value( QStringLiteral( "postId" ) ).toString();

      replies.append( postReply );
    }

    feedItem.setPostReplies( replies );
  }

  item.setMimeType( QStringLiteral( "text/x-vnd.akonadi.socialfeeditem" ) );
  item.setPayload<SocialFeedItem>( feedItem );

  return true;
}

void SocialFeedItemSerializerPlugin::serialize( const Item &item,
                                                const QByteArray &label,
                                                QIODevice &data,
                                                int &version )
{
  Q_UNUSED( label );
  Q_UNUSED( version );

  if ( !item.hasPayload<SocialFeedItem>() ) {
    return;
  }

  SocialFeedItem feedItem = item.payload<SocialFeedItem>();

  QJsonObject map;

  map.insert( QStringLiteral( "networkString" ), feedItem.networkString() );
  map.insert( QStringLiteral( "postId" ), feedItem.postId() );
  map.insert( QStringLiteral( "postText" ), feedItem.postText() );
  map.insert( QStringLiteral( "postLinkTitle" ), feedItem.postLinkTitle() );
  map.insert( QStringLiteral( "postLink" ), feedItem.postLink().toString() );
  map.insert( QStringLiteral( "postImageUrl" ), feedItem.postImageUrl().toString() );
  map.insert( QStringLiteral( "postInfo" ), feedItem.postInfo() );
  map.insert( QStringLiteral( "userName" ), feedItem.userName() );
  map.insert( QStringLiteral( "userDisplayName" ), feedItem.userDisplayName() );
  map.insert( QStringLiteral( "userId" ), feedItem.userId() );
  map.insert( QStringLiteral( "avatarUrl" ), feedItem.avatarUrl().toString() );
  map.insert( QStringLiteral( "postTimeString" ), feedItem.postTimeString() );
  map.insert( QStringLiteral( "postTimeFormat" ), feedItem.postTimeFormat() );
  map.insert( QStringLiteral( "shared" ), feedItem.isShared() );
  map.insert( QStringLiteral( "sharedFrom" ), feedItem.sharedFrom() );
  map.insert( QStringLiteral( "sharedFromId" ), feedItem.sharedFromId() );
  map.insert( QStringLiteral( "liked" ), feedItem.isLiked() );
  map.insert( QStringLiteral( "itemSourceMap" ), QJsonObject::fromVariantMap( feedItem.itemSourceMap() ) );

  if (!feedItem.postReplies().isEmpty() ) {
    QJsonArray replies;
    Q_FOREACH ( const SocialFeedItem &reply, feedItem.postReplies() ) {
      QJsonObject replyData;
      replyData.insert( QStringLiteral( "userId" ), reply.userId() );
      replyData.insert( QStringLiteral( "userName" ), reply.userName() );
      replyData.insert( QStringLiteral( "userAvatarUrl" ), reply.avatarUrl().toString() );
      replyData.insert( QStringLiteral( "replyText" ), reply.postText() );
//       replyData.insert( QStringLiteral( "replyTime" ), reply.postTimeString() );
      replyData.insert( QStringLiteral( "replyId" ), reply.postId() );
//       replyData.insert( QStringLiteral( "postId" ), reply.postId );
      replies.append( replyData );
    }

    map.insert( QStringLiteral( "postReplies" ), replies );
  }
  // TODO: rather use compact json format?
  data.write( QJsonDocument( map ).toJson( QJsonDocument::Indented ) );

}

QSet<QByteArray> SocialFeedItemSerializerPlugin::parts( const Item &item ) const
{
  // only need to reimplement this when implementing partial serialization
  // i.e. when using the "label" parameter of the other two methods
  return ItemSerializerPlugin::parts( item );
}
