/*
    This file is part of the kblog library.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2006 Christian Weilbach <christian@whiletaker.homeip.net>

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

#include <metaweblog.h>
#include <kxmlrpcclient/client.h>

#include <kdebug.h>
#include <klocale.h>
#include <kdatetime.h>

#include <QtCore/QList>

using namespace KBlog;

class APIMetaWeblog::Private
{
  public:
    KXmlRpc::Client* mXmlRpcClient;
};

APIMetaWeblog::APIMetaWeblog( const KUrl &server, QObject *parent, const char *name ) : APIBlog( server, parent, name ), 
d( new Private)
{
  d->mXmlRpcClient = new KXmlRpc::Client( server );
}

APIMetaWeblog::~APIMetaWeblog()
{
  delete d->mXmlRpcClient;
  delete d;
}

QString APIMetaWeblog::getFunctionName( blogFunctions type )
{
  switch ( type ) {
    case blogGetUserInfo:    return "metaWeblog.getUserInfo";
    case blogGetUsersBlogs:  return "metaWeblog.getUsersBlogs";
    case blogGetCategories:  return "metaWeblog.getCategories";
    case blogGetRecentPosts: return "metaWeblog.getRecentPosts";
    case blogNewPost:        return "metaWeblog.newPost";
    case blogNewMedia:	return "metaWeblog.newMediaObject";
    case blogEditPost:       return "metaWeblog.editPost";
    case blogDeletePost:     return "metaWeblog.deletePost";
    case blogGetPost:        return "metaWeblog.getPost";
    case blogGetTemplate:    return "metaWeblog.getTemplate";
    case blogSetTemplate:    return "metaWeblog.setTemplate";
    default: return QString::null;
  }
}




void APIMetaWeblog::userInfo()
{
  kDebug() << "userInfo(): getUserInfo is not available in MetaWeblog API." << endl;
  emit error( i18n( "User info is not available in MetaWeblog API." ) );
}

void APIMetaWeblog::listBlogs()
{
  kDebug() << "userInfo(): getUsersBlogs is not available in MetaWeblog API." << endl;
  emit error( i18n( "Fetching user's blogs is not available in MetaWeblog API." ) );
}

void APIMetaWeblog::listPostings()
{
  kDebug() << "Fetching List of Posts..." << endl;
  QList<QVariant> args( defaultArgs( blogId() ) );
  args << QVariant( downloadCount() );
  d->mXmlRpcClient->call( getFunctionName( blogGetRecentPosts ), args, 
                          this, SLOT( slotListPostings( QList<QVariant> &result, QVariant &id ) ), 
			  this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::listCategories(){
  kDebug() << "Fetching List of Categories..." << endl;
  QList<QVariant> args( defaultArgs( blogId() ) );
  d->mXmlRpcClient->call( getFunctionName( blogGetCategories ), args, 
                          this, SLOT( slotListCategories( QList<QVariant> &result, QVariant &id ) ), 
			  this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );

}

void APIMetaWeblog::fetchPosting( const QString &postId )
{
  kDebug() << "Fetching Posting with postId " << postId << endl;
  QList<QVariant> args( defaultArgs( postId ) );
  d->mXmlRpcClient->call( getFunctionName( blogGetPost ), args, 
                          this, SLOT( slotFetchPosting( QList<QVariant> &result, QVariant &id ) ), 
			  this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::modifyPosting( KBlog::BlogPosting *posting )
{
  if ( !posting ) {
    kDebug() << "APIMetaWeblog::modifyPosting: posting null pointer" << endl;
  }
  kDebug() << "Uploading Posting with postId " << posting->postId() << endl;

  QList<QVariant> args( defaultArgs( posting->postId() ) );
  QMap<QString, QVariant> map;
  QList<QVariant> list;
  list.append( QString( posting->category() ) );
  map["categories"]=list;
  map["description"]=posting->content();
  map["title"]=posting->title();
  KDateTime date;
  map["dateCreated"]=date.currentUtcDateTime().dateTime();
  args << map;
  args << QVariant( posting->publish() );
  d->mXmlRpcClient->call( getFunctionName( blogEditPost ), args, 
                          this, SLOT( slotCreatePosting( QList<QVariant> &result, QVariant &id ) ), 
			  this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::createPosting( KBlog::BlogPosting *posting )
{
  if ( !posting ) {
    kDebug() << "APIMetaWeblog::createPosting: posting null pointer" << endl;
  }
  kDebug() << "Creating new Posting with blogId " << posting->blogId() << endl;
  QList<QVariant> args( defaultArgs( posting->blogId() ) );
  QMap<QString, QVariant> map;
  QList<QVariant> list;
  list.append( QString( posting->category() ) );
  map["categories"]=list;
  map["description"]=posting->content();
  map["title"]=posting->title();
  map["dateCreated"]=posting->creationDateTime().dateTime(); // TODO use original date of result?
  args << map;
  args << QVariant( posting->publish() );
  d->mXmlRpcClient->call( getFunctionName( blogNewPost ), args, 
                          this, SLOT( slotCreatePosting( QList<QVariant> &result, QVariant &id ) ), 
			  this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
  posting->setUploaded( true ); // FIXME check for errors?
}

void APIMetaWeblog::createMedia( KBlog::BlogMedia *media ){
  kDebug() << "APIMetaWeblog::createMedia: name="<< media->title() << endl;
  QList<QVariant> args( defaultArgs( media->blogId() ) );
  QMap<QString, QVariant> map;
  QList<QVariant> list;
  map["name"]=media->title();
  map["type"]=media->mimetype();
  map["bits"]=media->data();
  args << map;
  d->mXmlRpcClient->call( getFunctionName( blogNewMedia ), args, 
                          this, SLOT( slotCreateMedia( QList<QVariant> &result, QVariant &id ) ), 
			  this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );

}

void APIMetaWeblog::removePosting( const QString &postId )
{
  kDebug() << "APIMetaWeblog::removePosting: postid=" << postId << endl;
  QList<QVariant> args( defaultArgs( postId ) );
  args << QVariant( /*publish=*/true );
  d->mXmlRpcClient->call( getFunctionName( blogDeletePost ), args, 
                          this, SLOT( slotCreateMedia( QList<QVariant> &result, QVariant &id ) ), 
			  this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::slotUserInfo( const QList<QVariant> &result, const QVariant &id )
{
  kDebug()<<  "APIMetaWeblog::slotUserInfo not implemented for MetaWeblog API" << endl;
}

void APIMetaWeblog::slotListBlogs( const QList<QVariant> &result, const QVariant &id )
{
  kDebug() << "APIMetaWeblog::slotListBlogs  not implemented for MetaWeblog API" << endl;
}

void APIMetaWeblog::slotListCategories( const QList<QVariant> &result, const QVariant &id )
{
  kDebug() << "APIMetaWeblog::slotListCategories" << endl;
  kDebug () << "TOP: " << result[0].typeName() << endl;

  const QMap<QString, QVariant> categories = result[0].toMap();
  const QList<QString> categoryNames = categories.keys();

  QList<QString>::ConstIterator it = categoryNames.begin();
  QList<QString>::ConstIterator end = categoryNames.end();
  for ( ; it != end; ++it ) {
    kDebug () << "MIDDLE: " << ( *it ) << endl;
    const QString name( *it );
    const QMap<QString, QVariant> category = categories[ *it ].toMap();
    const QString description( category["description"].toString() );
    if (  !name.isEmpty() ) {
      emit categoryInfoRetrieved( name, description );
       kDebug()<< "Emitting categorieInfoRetrieved( name=" << name << " description=" << description << " ); " << endl;
    }
  }
  kDebug() << "Emitting fetchingCategoriesFinished()" << endl;
  emit fetchingCategoriesFinished();
}

void APIMetaWeblog::slotListPostings( const QList<QVariant> &result, const QVariant &id )
{
  kDebug()<<"APIMetaWeblog::slotListPostings"<<endl;
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;

  const QList<QVariant> postReceived = result[ 0 ].toList();
  QList<QVariant>::ConstIterator it = postReceived.begin();
  QList<QVariant>::ConstIterator end = postReceived.end();
  for ( ; it != end; ++it ) {
    BlogPosting posting;
    kDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    if ( readPostingFromMap( &posting, postInfo ) ) {
       kDebug() << "Emitting itemOnServer( posting with postId()=" << posting.postId() << "); " << endl;
       emit itemOnServer( posting ); 
    } else {
       kDebug() << "readPostingFromMap failed! " << endl;
       emit error( i18n( "Couldn't read posting." ) );
    }
  }
  kDebug() << "Emitting fetchingPostsFinished() " << endl;
  emit fetchingPostsFinished();
}

void APIMetaWeblog::slotFetchPosting( const QList<QVariant> &result, const QVariant &id )
{
    //array of structs containing ISO.8601 dateCreated, String userid, String postid, String content;
    // TODO: Time zone for the dateCreated!
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;
  BlogPosting posting;
  const QMap<QString, QVariant> postInfo = result[ 0 ].toMap();
  if ( readPostingFromMap( &posting, postInfo ) ) {
     kDebug() << "Emitting itemOnServer( posting with postId()=" << posting.postId() << "); " << endl;
     emit itemOnServer( posting ); //KUrl( posting.postId() ) );
  } else {
     kDebug() << "readPostingFromMap failed! " << endl;
     emit error( "Couldn't read posting." );
  }
}

void APIMetaWeblog::slotCreatePosting( const QList<QVariant> &result, const QVariant &id )
{
  //array of structs containing ISO.8601 dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;
  QString postId = result[ 0 ].toString();
  kDebug() << "MIDDLE: postId=" << postId << endl;
  emit uploadPostId( postId.toInt() );
  kDebug() << "Emitting uploadPostId( " << postId.toInt() << " )" << endl;
}

void APIMetaWeblog::slotCreateMedia( const QList<QVariant> &result, const QVariant &id )
{
  kDebug() << "APIMetaWeblog::slotCreateMedia, no error!" << endl;
  kDebug () << "TOP: " << result[0].typeName() << endl;

  const QMap<QString, QVariant> resultStruct = result[0].toMap();
  const QString url = resultStruct["url"].toString();
  kDebug() << "APIMetaWeblog::slotCreateMedia url="<< url << endl;

  if (  !url.isEmpty() ) {
    emit mediaInfoRetrieved( url );
    kDebug()<< "Emitting mediaInfoRetrieved( url=" << url  << " ); " << endl;
  }
}

void APIMetaWeblog::faultSlot( int number, const QString& errorString, const QVariant& id )
{
  emit error( errorString );
}

bool APIMetaWeblog::readPostingFromMap( BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  // FIXME: integrate error handling
  if ( !post ) return false;
  QStringList mapkeys = postInfo.keys();
  kDebug() << endl << "Keys: " << mapkeys.join(", ") << endl << endl;
  
  KDateTime dt = KDateTime( postInfo[ "dateCreated" ].toDateTime() );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt );
  }
  dt = KDateTime( postInfo[ "lastModified" ].toDateTime() );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt );
  }

  post->setUserId( postInfo[ "userid" ].toString() );
  post->setPostId( postInfo[ "postid" ].toString() );

  QString title( postInfo[ "title" ].toString() );
  QString description( postInfo[ "description" ].toString() );
  QList<QVariant> categories( postInfo[ "categories" ].toList() );

  post->setTitle( title );
  post->setContent( description );
  if ( !categories.isEmpty() ){
    QString category = ( *categories.begin() ).toString();
    kDebug() << "Category: " <<  category  << endl;
    post->setCategory( category );
  }
  return true;
}

