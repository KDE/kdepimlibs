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

#include "metaweblog.h"
#include "metaweblog_p.h"
#include "blogposting.h"
#include "blogmedia.h"

#include <kxmlrpcclient/client.h>
#include <KDebug>
#include <KLocale>
#include <KDateTime>

using namespace KBlog;

MetaWeblog::MetaWeblog( const KUrl &server, QObject *parent )
  : Blogger1( server, *new MetaWeblogPrivate, parent )
{
  setUrl( server );
}

MetaWeblog::MetaWeblog( const KUrl &server, MetaWeblogPrivate &dd,
                        QObject *parent )
  : Blogger1( server, dd, parent )
{
  setUrl( server );
}

MetaWeblog::~MetaWeblog()
{
}

QString MetaWeblog::interfaceName() const
{
  return QLatin1String( "MetaWeblog" );
}

void MetaWeblog::listRecentPostings( int number )
{
    Q_D(MetaWeblog);
    kDebug(5323) << "Fetching List of Posts...";
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( number );
    d->mXmlRpcClient->call(
      "metaWeblog.getRecentPosts", args,
      this, SLOT( slotListRecentPostings( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT( slotError( int, const QString&, const QVariant& ) ), QVariant( number ) );
}

void MetaWeblog::listCategories()
{
    Q_D(MetaWeblog);
    kDebug(5323) << "Fetching List of Categories...";
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    d->mXmlRpcClient->call(
      "metaWeblog.getCategories", args,
      this, SLOT( slotListCategories( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT ( slotError( int, const QString&, const QVariant& ) ) );
}

void MetaWeblog::fetchPosting( KBlog::BlogPosting *posting )
{
  Q_D(MetaWeblog);
  if ( !posting ) {
    kDebug(5323) << "MetaWeblog::fetchPosting: posting is a null pointer";
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

void MetaWeblog::modifyPosting( KBlog::BlogPosting *posting )
{
  Q_D(MetaWeblog);
  if ( !posting ) {
    kDebug(5323) << "MetaWeblog::modifyPosting: posting is a null pointer";
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
  args << map;
  args << QVariant( posting->isPublished() );
  d->mXmlRpcClient->call(
    "metaWeblog.editPost", args,
     this, SLOT( slotModifyPosting( const QList<QVariant>&, const QVariant& ) ),
     this, SLOT ( slotError( int, const QString&, const QVariant& ) ), QVariant( i ) );
}

void MetaWeblog::createPosting( KBlog::BlogPosting *posting )
{
  Q_D(MetaWeblog);
  if ( !posting ) {
    kDebug(5323) << "MetaWeblog::createPosting: posting is a null pointer";
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
  args << map;
  args << QVariant( posting->isPublished() );
  d->mXmlRpcClient->call (
    "metaWeblog.newPost", args,
    this, SLOT( slotCreatePosting( const QList<QVariant>&, const QVariant& ) ),
    this, SLOT ( slotError( int, const QString&, const QVariant& ) ), QVariant( i ) );
}

void MetaWeblog::createMedia( KBlog::BlogMedia *media )
{
  Q_D(MetaWeblog);
  if ( !media ) {
    kDebug(5323) << "MetaWeblog::createMedia: media is a null pointer";
    emit error ( Other, i18n( "Media is a null pointer." ) );
    return;
  }
  unsigned int i = d->callMediaCounter++;
  d->callMediaMap[ i ] = media;
  kDebug(5323) << "MetaWeblog::createMedia: name="<< media->name();
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  QMap<QString, QVariant> map;
  QList<QVariant> list;
  map["name"] = media->name();
  map["type"] = media->mimetype();
  map["bits"] = media->data();
  args << map;
  d->mXmlRpcClient->call(
    "metaWeblog.newMediaObject", args,
    this, SLOT( slotCreateMedia( const QList<QVariant>&, const QVariant& ) ),
    this, SLOT ( slotError( int, const QString&, const QVariant& ) ), QVariant( i ) );

}

MetaWeblogPrivate::MetaWeblogPrivate()
{
  callMediaCounter=1;
}

MetaWeblogPrivate::~MetaWeblogPrivate()
{
}

QList<QVariant> MetaWeblogPrivate::defaultArgs( const QString &id )
{
  Q_Q(MetaWeblog);
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

void MetaWeblogPrivate::slotListCategories( const QList<QVariant> &result,
                                                              const QVariant &id )
{
  Q_Q(MetaWeblog);
  Q_UNUSED( id );

  QMap<QString,QMap<QString,QString> > categoriesMap;

  kDebug(5323) << "MetaWeblogPrivate::slotListCategories";
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Map &&
       result[0].type() != QVariant::List ) {
    // include fix for not metaweblog standard compatible apis with
    // array of structs instead of struct of structs, e.g. wordpress
    kDebug(5323) << "Could not list categories out of the result from the server.";
    emit q->error( MetaWeblog::ParsingError,
                        i18n( "Could not list categories out of the result "
                              "from the server." ) );
  } else {
    if ( result[0].type() == QVariant::Map ) {
      const QMap<QString, QVariant> categories = result[0].toMap();
      const QList<QString> categoryNames = categories.keys();

      QList<QString>::ConstIterator it = categoryNames.begin();
      QList<QString>::ConstIterator end = categoryNames.end();
      for ( ; it != end; ++it ) {
        kDebug(5323) << "MIDDLE:" << ( *it );
        const QMap<QString, QVariant> c = categories[*it].toMap();
        categoriesMap[ *it ]["description"] = c[ "description" ].toString();
        categoriesMap[ *it ]["htmlUrl"]=c[ "htmlUrl" ].toString();
        categoriesMap[ *it ]["rssUrl"]=c[ "rssUrl" ].toString();
        }
        emit q->listedCategories( categoriesMap );
        kDebug(5323) << "Emitting listedCategories";
      }
    }
    if ( result[0].type() == QVariant::List ) {
      // include fix for not metaweblog standard compatible apis with
      // array of structs instead of struct of structs, e.g. wordpress
      const QList<QVariant> categories = result[0].toList();
      QList<QVariant>::ConstIterator it = categories.begin();
      QList<QVariant>::ConstIterator end = categories.end();
      for ( ; it != end; ++it ) {
        kDebug(5323) << "MIDDLE:" << ( *it ).typeName();
        const QMap<QString, QVariant> c = ( *it ).toMap();
        const QString name= c["categoryName"].toString();
        categoriesMap[ name ]["description"] = c[ "description" ].toString();
        categoriesMap[ name ]["htmlUrl"]=c[ "htmlUrl" ].toString();
        categoriesMap[ name ]["rssUrl"]=c[ "rssUrl" ].toString();
      }
      kDebug(5323) << "Emitting listedCategories()";
      emit q->listedCategories( categoriesMap );
    }
  }

void MetaWeblogPrivate::slotListRecentPostings( const QList<QVariant> &result,
                                                            const QVariant &id )
{
  Q_Q(MetaWeblog);

  int count = id.toInt();

  QList <BlogPosting> fetchedPostingList;

  kDebug(5323) << "MetaWeblog::slotListRecentPostings";
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::List ) {
    kDebug(5323) << "Could not fetch list of postings out of the"
                 << "result from the server.";
    emit q->error( MetaWeblog::ParsingError,
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
        emit q->error( MetaWeblog::ParsingError, i18n( "Could not read posting." ) );
      }
      if( --count == 0 ) break;
    }
  } //FIXME should we emit here? (see below, too)
  kDebug(5323) << "Emitting listRecentPostingsFinished()";
  emit q->listedRecentPostings( fetchedPostingList );
}

void MetaWeblogPrivate::slotFetchPosting( const QList<QVariant> &result,
                                                            const QVariant &id )
{
  Q_Q(MetaWeblog);

  KBlog::BlogPosting* posting = callMap[ id.toInt() ];
  callMap.remove( id.toInt() );

  kDebug(5323) << "MetaWeblog::slotFetchPosting";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Map ) {
    kDebug(5323) << "Could not fetch posting out of the result from the server.";
    emit q->error( MetaWeblog::ParsingError,
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
      emit q->error( MetaWeblog::ParsingError,
                         i18n( "Could not read posting." ), posting );
    }
  }
}

void MetaWeblogPrivate::slotCreatePosting( const QList<QVariant> &result,
                                                             const QVariant &id )
{
  Q_Q(MetaWeblog);

  KBlog::BlogPosting* posting = callMap[ id.toInt() ];
  callMap.remove( id.toInt() );

  kDebug(5323) << "MetaWeblog::slotCreatePosting";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::String ) {
    kDebug(5323) << "Could not read the postingId, not a string.";
    emit q->error( MetaWeblog::ParsingError,
                       i18n( "Could not read the postingId, not a string." ), posting );
  } else {
     posting->setPostingId( result[0].toString() );
     posting->setStatus( BlogPosting::Created );
     emit q->createdPosting( posting );
    kDebug(5323) << "emitting createdPosting(" << result[0].toString() << ")";
  }
}

void MetaWeblogPrivate::slotModifyPosting( const QList<QVariant> &result,
                                                             const QVariant &id )
{
  Q_Q(MetaWeblog);

  KBlog::BlogPosting* posting = callMap[ id.toInt() ];
  callMap.remove( id.toInt() );

  kDebug(5323) << "MetaWeblog::slotModifyPosting";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Bool ) {
    kDebug(5323) << "Could not read the result, not a boolean.";
    emit q->error( MetaWeblog::ParsingError,
                       i18n( "Could not read the result, not a boolean." ), posting );
  } else {
    posting->setStatus( BlogPosting::Modified );
    emit q->modifiedPosting( posting );
    kDebug(5323) << "emitting modifiedPosting()";
  }
}

void MetaWeblogPrivate::slotCreateMedia( const QList<QVariant> &result,
                                                           const QVariant &id )
{
  Q_Q(MetaWeblog);

  KBlog::BlogMedia* media = callMediaMap[ id.toInt() ];
  callMediaMap.remove( id.toInt() );

  kDebug(5323) << "MetaWeblogPrivate::slotCreateMedia, no error!";
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != 8 ) {
    kDebug(5323) << "Could not read the result, not a map.";
    emit q->error( MetaWeblog::ParsingError,
                       i18n( "Could not read the result, not a map." ) );
  } else {
    const QMap<QString, QVariant> resultStruct = result[0].toMap();
    const QString url = resultStruct["url"].toString();
    kDebug(5323) << "MetaWeblog::slotCreateMedia url=" << url;

    if ( !url.isEmpty() ) {
      media->setUrl( KUrl( url ) );
      media->setStatus( BlogMedia::Created );
      emit q->createdMedia( media );
      kDebug(5323) << "Emitting createdMedia( url=" << url  << ");";
    }
  }
}

void MetaWeblogPrivate::slotError( int number,
                                                     const QString &errorString,
                                                     const QVariant &id )
{
  Q_Q(MetaWeblog);
  Q_UNUSED( number );
  Q_UNUSED( id );

  emit q->error( MetaWeblog::XmlRpc, errorString );
}

bool MetaWeblogPrivate::readPostingFromMap( BlogPosting *post,
                                                        const QMap<QString, QVariant> &postInfo )
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

  post->setTitle( title );
  post->setContent( description );
  if ( !categories.isEmpty() ){
    kDebug(5323) << "Categories:" << categories;
    post->setCategories( categories );
  }
  return true;
}

#include "metaweblog.moc"
