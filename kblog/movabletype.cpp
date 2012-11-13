/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2009 Christian Weilbach <christian_weilbach@web.de>
  Copyright (c) 2007-2008 Mike McQuaid <mike@mikemcquaid.com>

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
#include <kio/job.h>

#include <KDebug>
#include <KLocale>
#include <KDateTime>

#include <QtCore/QStringList>

using namespace KBlog;

MovableType::MovableType( const KUrl &server, QObject *parent )
  : MetaWeblog( server, *new MovableTypePrivate, parent )
{
  kDebug();
}

MovableType::MovableType( const KUrl &server, MovableTypePrivate &dd,
                        QObject *parent )
  : MetaWeblog( server, dd, parent )
{
  kDebug();
}

MovableType::~MovableType()
{
  kDebug();
}

QString MovableType::interfaceName() const
{
  return QLatin1String( "Movable Type" );
}

void MovableType::listRecentPosts( int number )
{
    Q_D( MovableType );
    kDebug();
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( number );
    d->mXmlRpcClient->call(
      "metaWeblog.getRecentPosts", args,
      this, SLOT(slotListRecentPosts(QList<QVariant>,QVariant)),
      this, SLOT(slotError(int,QString,QVariant)),
      QVariant( number ) );
}

void MovableType::listTrackBackPings( KBlog::BlogPost *post )
{
  Q_D( MovableType );
  kDebug();
  QList<QVariant> args;
  args << QVariant( post->postId() );
  unsigned int i = d->mCallCounter++;
  d->mCallMap[ i ] = post;
  d->mXmlRpcClient->call(
    "mt.getTrackbackPings", args,
    this, SLOT(slotListTrackbackPings(QList<QVariant>,QVariant)),
    this, SLOT(slotError(int,QString,QVariant)),
    QVariant( i ) );
}

void MovableType::fetchPost( BlogPost *post )
{
  Q_D( MovableType );
  kDebug();
  d->loadCategories();
  if ( d->mCategoriesList.isEmpty() &&
       post->categories( ).count() ) {
    d->mFetchPostCache << post;
    if ( d->mFetchPostCache.count() ) {
      // we are already trying to fetch another post, so we don't need to start
      // another listCategories() job
      return;
    }

    connect( this, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
             this, SLOT(slotTriggerFetchPost()) );
    listCategories();
  } else {
    MetaWeblog::fetchPost( post );
  }
}

void MovableType::createPost( BlogPost *post )
{
  // reimplemented because we do this:
  // http://comox.textdrive.com/pipermail/wp-testers/2005-July/000284.html
  kDebug();
  Q_D( MovableType );

  // we need mCategoriesList to be loaded first, since we cannot use the post->categories()
  // names later, but we need to map them to categoryId of the blog
  d->loadCategories();
  if ( d->mCategoriesList.isEmpty() &&
       !post->categories().isEmpty() ) {
    kDebug() << "No categories in the cache yet. Have to fetch them first.";
    d->mCreatePostCache << post;
    connect( this, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
             this, SLOT(slotTriggerCreatePost()) );
    listCategories();
  }
  else {
    bool publish = post->isPrivate();
    // If we do setPostCategories() later than we disable publishing first.
    if ( !post->categories().isEmpty() ) {
      post->setPrivate( true );
      if ( d->mSilentCreationList.contains( post ) ) {
        kDebug() << "Post already in mSilentCreationList, this *should* never happen!";
      } else {
        d->mSilentCreationList << post;
      }
    }
    MetaWeblog::createPost( post );
    // HACK: uuh this a bit ugly now... reenable the original publish argument,
    // since createPost should have parsed now
    post->setPrivate( publish );
  }
}

void MovableType::modifyPost( BlogPost *post )
{
  // reimplemented because we do this:
  // http://comox.textdrive.com/pipermail/wp-testers/2005-July/000284.html
  kDebug();
  Q_D( MovableType );

  // we need mCategoriesList to be loaded first, since we cannot use the post->categories()
  // names later, but we need to map them to categoryId of the blog
  d->loadCategories();
  if ( d->mCategoriesList.isEmpty() &&
       !post->categories().isEmpty() ) {
    kDebug() << "No categories in the cache yet. Have to fetch them first.";
    d->mModifyPostCache << post;
    connect( this, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
             this, SLOT(slotTriggerModifyPost()) );
    listCategories();
  }
  else {
    MetaWeblog::modifyPost( post );
  }
}

void MovableTypePrivate::slotTriggerCreatePost()
{
  kDebug();
  Q_Q( MovableType );

  q->disconnect( q, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                 q, SLOT(slotTriggerCreatePost()) );
  // now we can recall createPost with the posts from the cache
  QList<BlogPost*>::Iterator it = mCreatePostCache.begin();
  QList<BlogPost*>::Iterator end = mCreatePostCache.end();
  for ( ; it != end; it++ ) {
    q->createPost( *it );
  }
  mCreatePostCache.clear();
}

void MovableTypePrivate::slotTriggerModifyPost()
{
  kDebug();
  Q_Q( MovableType );

  q->disconnect( q, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                 q, SLOT(slotTriggerModifyPost()) );
  // now we can recall createPost with the posts from the cache
  QList<BlogPost*>::Iterator it = mModifyPostCache.begin();
  QList<BlogPost*>::Iterator end = mModifyPostCache.end();
  for ( ; it != end; it++ ) {
    q->modifyPost( *it );
  }
  mModifyPostCache.clear();
}

void MovableTypePrivate::slotTriggerFetchPost()
{
  kDebug();
  Q_Q( MovableType );

  q->disconnect( q, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                 q, SLOT(slotTriggerFetchPost()) );
  QList<BlogPost*>::Iterator it = mFetchPostCache.begin();
  QList<BlogPost*>::Iterator end = mFetchPostCache.end();
  for ( ; it != end; it++ ) {
    q->fetchPost( *it );
  }
  mFetchPostCache.clear();
}


MovableTypePrivate::MovableTypePrivate()
{
  kDebug();
}

MovableTypePrivate::~MovableTypePrivate()
{
  kDebug();
}

void MovableTypePrivate::slotCreatePost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( MovableType );
  // reimplement from Blogger1 to chainload the categories stuff before emit()
  kDebug();
  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  kDebug();
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  kDebug () << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::String &&
       result[0].type() != QVariant::Int ) {
    kError() << "Could not read the postId, not a string or an integer.";
    emit q->errorPost( Blogger1::ParsingError,
                          i18n( "Could not read the postId, not a string or an integer." ),
                          post );
    return;
  }
  QString serverID;
  if ( result[0].type() == QVariant::String ) {
    serverID = result[0].toString();
  }
  if ( result[0].type() == QVariant::Int ) {
    serverID = QString( "%1" ).arg( result[0].toInt() );
  }
  post->setPostId( serverID );
  if ( mSilentCreationList.contains(  post ) )
  {
    // set the categories and publish afterwards
    setPostCategories( post, !post->isPrivate() );
  } else {
    kDebug() << "emitting createdPost()"
                << "for title: \"" << post->title()
                << "\" server id: " << serverID;
    post->setStatus( KBlog::BlogPost::Created );
    emit q->createdPost( post );
  }
}

void MovableTypePrivate::slotFetchPost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( MovableType );
  kDebug();

  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  kDebug () << "TOP:" << result[0].typeName();
  if ( result[0].type() == QVariant::Map &&
       readPostFromMap( post, result[0].toMap() ) ) {
  } else {
    kError() << "Could not fetch post out of the result from the server.";
    post->setError( i18n( "Could not fetch post out of the result from the server." ) );
    post->setStatus( BlogPost::Error );
    emit q->errorPost( Blogger1::ParsingError,
                       i18n( "Could not fetch post out of the result from the server." ), post );
  }
  if ( post->categories().isEmpty() ) {
    QList<QVariant> args( defaultArgs( post->postId() ) );
    unsigned int i= mCallCounter++;
    mCallMap[ i ] = post;
    mXmlRpcClient->call(
      "mt.getPostCategories", args,
      q, SLOT(slotGetPostCategories(QList<QVariant>,QVariant)),
      q, SLOT(slotError(int,QString,QVariant)),
      QVariant( i ) );
  } else {
    kDebug() << "Emitting fetchedPost()";
    post->setStatus( KBlog::BlogPost::Fetched );
    emit q->fetchedPost( post );
  }
}

void MovableTypePrivate::slotModifyPost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( MovableType );
  // reimplement from Blogger1
  kDebug();
  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  kDebug() << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Bool &&
       result[0].type() != QVariant::Int ) {
    kError() << "Could not read the result, not a boolean.";
    emit q->errorPost( Blogger1::ParsingError,
                          i18n( "Could not read the result, not a boolean." ),
                          post );
    return;
  }
  if ( mSilentCreationList.contains( post ) ) {
    post->setStatus( KBlog::BlogPost::Created );
    mSilentCreationList.removeOne( post );
    emit q->createdPost( post );
  } else {
    if ( !post->categories().isEmpty() ) {
      setPostCategories( post, false );
    }
  }
}

void MovableTypePrivate::setPostCategories( BlogPost *post, bool publishAfterCategories )
{
  kDebug();
  Q_Q( MovableType );

  unsigned int i = mCallCounter++;
  mCallMap[ i ] = post;
  mPublishAfterCategories[ i ] = publishAfterCategories;
  QList<QVariant> catList;
  QList<QVariant> args( defaultArgs( post->postId() ) );

  // map the categoryId of the server to the name
  QStringList categories = post->categories();
  for ( int j = 0; j < categories.count(); j++ ) {
     for ( int k = 0; k < mCategoriesList.count(); k++ ) {
       if ( mCategoriesList[k]["name"] == categories[j] ) {
         kDebug() << "Matched category with name: " << categories[ j ] << " and id: " << mCategoriesList[ k ][ "categoryId" ];
         QMap<QString,QVariant> category;
         //the first in the QStringList of post->categories()
         // is the primary category
         category["categoryId"] = mCategoriesList[k]["categoryId"].toInt();
         catList << QVariant( category );
         break;
       }
       if ( k == mCategoriesList.count() ) {
         kDebug() << "Couldn't find categoryId for: " << categories[j];
       }
     }
  }
  args << QVariant( catList );

  mXmlRpcClient->call(
    "mt.setPostCategories", args,
    q, SLOT(slotSetPostCategories(QList<QVariant>,QVariant)),
    q, SLOT(slotError(int,QString,QVariant)),
    QVariant( i ) );
}

void MovableTypePrivate::slotGetPostCategories(const QList<QVariant>& result,const QVariant& id)
{
  kDebug();
  Q_Q( MovableType );

  int i = id.toInt();
  BlogPost* post = mCallMap[ i ];
  mCallMap.remove( i );

  if ( result[ 0 ].type() != QVariant::List ) {
    kError() << "Could not read the result, not a list. Category fetching failed! We will still emit fetched post now.";
    emit q->errorPost( Blogger1::ParsingError,
        i18n( "Could not read the result - is not a list. Category fetching failed." ), post );

    post->setStatus( KBlog::BlogPost::Fetched );
    emit q->fetchedPost( post );
  } else {
    QList<QVariant> categoryList = result[ 0 ].toList();
    QList<QString> newCatList;
    QList<QVariant>::ConstIterator it = categoryList.constBegin();
    QList<QVariant>::ConstIterator end = categoryList.constEnd();
    for ( ; it != end; it++ ) {
      newCatList << ( *it ).toMap()[ "categoryName" ].toString();
    }
    kDebug() << "categories list: " << newCatList;
    post->setCategories( newCatList );
    post->setStatus( KBlog::BlogPost::Fetched );
    emit q->fetchedPost( post );
  }
}

void MovableTypePrivate::slotSetPostCategories(const QList<QVariant>& result,const QVariant& id)
{
  kDebug();
  Q_Q( MovableType );

  int i = id.toInt();
  BlogPost* post = mCallMap[ i ];
  bool publish = mPublishAfterCategories[ i ];
  mCallMap.remove( i );
  mPublishAfterCategories.remove( i );

  if ( result[0].type() != QVariant::Bool ) {
    kError() << "Could not read the result, not a boolean. Category setting failed! We will still publish if now if necessary. ";
    emit q->errorPost( Blogger1::ParsingError,
                          i18n( "Could not read the result - is not a boolean value. Category setting failed.  Will still publish now if necessary." ),
                          post );
  }
  // Finally publish now, if the post was meant to be published in the beginning.
  // The first boolean is necessary to only publish if the post is created, not
  // modified.
  if ( publish && !post->isPrivate() ) {
    q->modifyPost( post );
  }

  // this is the end of the chain then
  if ( !publish ) {
    if ( mSilentCreationList.contains( post ) ) {
      kDebug() << "emitting createdPost() for title: \""
              << post->title() << "\"";
      post->setStatus( KBlog::BlogPost::Created );
      mSilentCreationList.removeOne( post );
      emit q->createdPost( post );
    } else {
      kDebug() << "emitting modifiedPost() for title: \""
              << post->title() << "\"";
      post->setStatus( KBlog::BlogPost::Modified );
      emit q->modifiedPost( post );
    }
  }
}

QList<QVariant> MovableTypePrivate::defaultArgs( const QString &id )
{
  Q_Q( MovableType );
  QList<QVariant> args;
  if ( !id.isEmpty() ) {
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
    post->setCreationDateTime( dt.toLocalZone() );
  }

  dt =
    KDateTime( postInfo["lastModified"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt.toLocalZone() );
  }

  post->setPostId( postInfo["postid"].toString().isEmpty() ? postInfo["postId"].toString() :
                   postInfo["postid"].toString() );

  QString title( postInfo["title"].toString() );
  QString description( postInfo["description"].toString() );
  QStringList categoryIdList = postInfo["categories"].toStringList();
  QStringList categories;
  // since the metaweblog definition is ambigious, we try different
  // category mappings
  for ( int i = 0; i < categoryIdList.count(); i++ ) {
    for ( int k = 0; k < mCategoriesList.count(); k++ ) {
      if ( mCategoriesList[ k ][ "name" ] == categoryIdList[ i ] ) {
        categories << mCategoriesList[ k ][ "name" ];
      } else if ( mCategoriesList[ k ][ "categoryId" ] == categoryIdList[ i ]) {
        categories << mCategoriesList[ k ][ "name" ];
      }
    }
  }

  //TODO 2 new keys are:
  // String mt_convert_breaks, the value for the convert_breaks field
  post->setSlug( postInfo["wp_slug"].toString() );
  post->setAdditionalContent( postInfo["mt_text_more"].toString() );
  post->setTitle( title );
  post->setContent( description );
  post->setCommentAllowed( (bool)postInfo["mt_allow_comments"].toInt() );
  post->setTrackBackAllowed( (bool)postInfo["mt_allow_pings"].toInt() );
  post->setSummary( postInfo["mt_excerpt"].toString() );
  post->setTags( postInfo["mt_keywords"].toStringList() );
  post->setLink( postInfo["link"].toString() );
  post->setPermaLink( postInfo["permaLink"].toString() );
  QString postStatus = postInfo["post_status"].toString();
  if ( postStatus != "publish" &&
       !postStatus.isEmpty() ) {
    /**
     * Maybe this field wasn't set by server! so, on that situation, we will assume it as non-Private,
     * The postStatus.isEmpty() check is for that!
     * I found this field on Wordpress output! it's value can be: publish, private, draft (as i see)
    */
    post->setPrivate( true );
  }
  if ( !categories.isEmpty() ) {
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
  //TODO 2 new keys are:
  // String mt_convert_breaks, the value for the convert_breaks field
  // array mt_tb_ping_urls, the list of TrackBack ping URLs for this entry
  if ( !args ) {
    return false;
  }
  QMap<QString, QVariant> map;
  map["categories"] = post.categories();
  map["description"] = post.content();
  if ( !post.additionalContent().isEmpty() ) {
    map["mt_text_more"] = post.additionalContent();
  }
  map["title"] = post.title();
  map["dateCreated"] = post.creationDateTime().dateTime().toUTC();
  map["mt_allow_comments"] = (int)post.isCommentAllowed();
  map["mt_allow_pings"] = (int)post.isTrackBackAllowed();
  map["mt_excerpt"] = post.summary();
  map["mt_keywords"] = post.tags().join( "," );
  //map["mt_tb_ping_urls"] check for that, i think this should only be done on the server.
  *args << map;
  *args << QVariant( !post.isPrivate() );
  return true;
}

#include "moc_movabletype.cpp"
