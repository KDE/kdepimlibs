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

#include <blogger.h>

#include <kxmlrpcclient/client.h>
#include <kdebug.h>
#include <klocale.h>

#include <QtCore/QList>

using namespace KBlog;

class APIBlogger::Private
{
  public:
    KXmlRpc::Client* mXmlRpcClient;
};

APIBlogger::APIBlogger( const KUrl &server, QObject *parent, const char *name ) : APIBlog( server, parent, name ), 
d( new Private)
{
  d->mXmlRpcClient = new KXmlRpc::Client( server );
}

APIBlogger::~APIBlogger()
{
  delete d->mXmlRpcClient;
  delete d;
}

QString APIBlogger::getFunctionName( blogFunctions type )
{
  switch ( type ) {
    case blogGetUserInfo:    return "blog.getUserInfo";
    case blogGetUsersBlogs:  return "blog.getUsersBlogs";
    case blogGetCategories:  return "blog.getCategories"; // not implemented in fact
    case blogGetRecentPosts: return "blog.getRecentPosts";
    case blogNewPost:        return "blog.newPost";
    case blogNewMedia:       return "blog.newMedia"; // not implemented in fact
    case blogEditPost:       return "blog.editPost";
    case blogDeletePost:     return "blog.deletePost";
    case blogGetPost:        return "blog.getPost";
    case blogGetTemplate:    return "blog.getTemplate";
    case blogSetTemplate:    return "blog.setTemplate";
    default: return QString::null;
  }
}




void APIBlogger::userInfo()
{
  kDebug() << "read user info..." << endl;
  QList<QVariant> args( defaultArgs() );
  d->mXmlRpcClient->call( getFunctionName( blogGetUserInfo ), args, 
                          this, SLOT( slotUserInfo(const QList<QVariant>&, const QVariant&) ), 
			  this, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::listBlogs()
{
  kDebug() << "Fetch List of Blogs..." << endl;
  QList<QVariant> args( defaultArgs() );
  d->mXmlRpcClient->call( getFunctionName( blogEditPost ), args, 
                          this, SLOT( slotListBlogs( QList<QVariant> &result, QVariant &id ) ), 
			  this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::listPostings()
{
  kDebug() << "Fetching List of Posts..." << endl;
  QList<QVariant> args( defaultArgs( blogId() ) );
  args << QVariant( downloadCount() );
  d->mXmlRpcClient->call( getFunctionName( blogGetRecentPosts ), args, 
                          this, SLOT( slotListPostings( QList<QVariant> &result, QVariant &id ) ), 
			  this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::listCategories(){
  kDebug() << "Categories are not supported in Blogger API 1.0" << endl;
}

void APIBlogger::fetchPosting( const QString &postId )
{
  kDebug() << "Fetching Posting with url " << postId << endl;
  QList<QVariant> args( defaultArgs( postId ) );
  d->mXmlRpcClient->call( getFunctionName( blogGetPost ), args, 
                    this, SLOT( slotFetchPosting( QList<QVariant> &result, QVariant &id ) ), 
		    this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::modifyPosting( KBlog::BlogPosting *posting )
{
  if ( !posting ) {
    kDebug() << "APIBlogger::modifyPosting: posting is null pointer" << endl;
  }
    kDebug() << "Uploading Posting with postId " << posting->postId() << endl;
    QList<QVariant> args( defaultArgs( posting->postId() ) );
    args << QVariant( posting->content() );
    args << QVariant( posting->publish() );
    d->mXmlRpcClient->call( getFunctionName( blogEditPost ), args, 
                            this, SLOT( slotCreatePosting( QList<QVariant> &result, QVariant &id ) ), 
			    this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::createPosting( KBlog::BlogPosting *posting )
{
  if ( !posting ) {
    kDebug() << "APIBlogger::createPosting: posting is null pointer" << endl;
  }
    kDebug() << "Creating new Posting with blogid " << posting->blogId() << endl;
    QList<QVariant> args( defaultArgs( posting->blogId() ) );
    args << QVariant( posting->content() );
    args << QVariant( posting->publish() );
    d->mXmlRpcClient->call( getFunctionName( blogNewPost ), args, 
                            this, SLOT( slotCreatePosting( QList<QVariant> &result, QVariant &id ) ), 
			    this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
    posting->setUploaded( true ); // FIXME: check for errors?
}

void APIBlogger::createMedia( KBlog::BlogMedia *media ){
  kDebug() << "APIBlogger::createMedia: not available in Blogger API." << endl;
}

void APIBlogger::removePosting( const QString &postId )
{
  kDebug() << "APIBlogger::removePosting: postId=" << postId << endl;
  QList<QVariant> args( defaultArgs( postId ) );
  args << QVariant( /*publish=*/true );
  d->mXmlRpcClient->call( getFunctionName( blogDeletePost ), args, 
                          this, SLOT( slotCreateMedia( QList<QVariant> &result, QVariant &id ) ), 
			  this, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::slotUserInfo( const QList<QVariant> &result, const QVariant &id )
{
  // TODO: Implement user authentication
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;
  const QList<QVariant> posts = result;
  QList<QVariant>::ConstIterator it = posts.begin();
  QList<QVariant>::ConstIterator end = posts.end();
  for ( ; it != end; ++it ) {
    kDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    const QString nickname = postInfo[ "nickname" ].toString();
    const QString userid = postInfo[ "userid" ].toString();
    const QString email = postInfo[ "email" ].toString();
    kDebug() << "Post " << nickname << " " << userid << " " << email << endl;
    // FIXME: What about a BlogUserInfo class???
    emit userInfoRetrieved( nickname, userid, email );
  }
}

void APIBlogger::slotListBlogs( const QList<QVariant> &result, const QVariant &id )
{
  kDebug() << "APIBlogger::slotListBlogs" << endl;
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;

  const QList<QVariant> posts = result[ 0 ].toList();
  QList<QVariant>::ConstIterator it = posts.begin();
  QList<QVariant>::ConstIterator end = posts.end();
  for ( ; it != end; ++it ) {
    kDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();

    const QString id( postInfo[ "blogid" ].toString() );
    const QString name( postInfo[ "blogName" ].toString() );
    const QString url( postInfo[ "url" ].toString() );

    if ( !id.isEmpty() && !name.isEmpty() ) {
      emit folderInfoRetrieved( id, name );
      kDebug()<< "Emitting folderInfoRetrieved( id=" << id << ", name=" << name << "); " << endl;
    }
  }
}

void APIBlogger::slotListCategoriesJob( const QList<QVariant> &result, const QVariant &id ){
  kDebug() << "Categories are not supported in Blogger API 1.0" << endl;
  emit error( i18n("Categories are not supported in Blogger API 1.0") );
}

void APIBlogger::slotListPostings( const QList<QVariant> &result, const QVariant &id )
{
  slotFetchPosting( result, id );
}

void APIBlogger::slotFetchPosting( const QList<QVariant> &result, const QVariant &id )
{
  kDebug(5800)<<"APIBlogger::slotFetchPosting"<<endl;
  //array of structs containing ISO.8601 dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;

  const QList<QVariant> postReceived = result[ 0 ].toList();
  QList<QVariant>::ConstIterator it = postReceived.begin();
  QList<QVariant>::ConstIterator end = postReceived.end();
  for ( ; it != end; ++it ) {
    BlogPosting posting;
    kDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    if ( readPostingFromMap( &posting, postInfo ) ) {
      kDebug() << "Emitting itemOnServer( posting.postId()=" <<posting.postId() << "); " << endl;
      emit itemOnServer( posting ); // KUrl( posting.postId() ) );
    } else {
      kDebug() << "readPostingFromMap failed! " << endl;
      emit error( i18n("Couldn't read posting.") );
    }
  }
  kDebug() << "Emitting fetchingPostsFinished()" << endl;
  emit fetchingPostsFinished();
}

void APIBlogger::slotCreatePosting( const QList<QVariant> &result, const QVariant &id )
{
  kDebug(5800)<<"APIBlogger::slotCreatePosting"<<endl;
  //array of structs containing ISO.8601 dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;

  const QList<QVariant> postReceived = result[ 0 ].toList();
  QList<QVariant>::ConstIterator it = postReceived.begin();
  QList<QVariant>::ConstIterator end = postReceived.end();
  kDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
  emit uploadPostId( ( *it ).toInt() );
  kDebug() << "emitting uploadPostId( " << ( *it ).toInt() << " )" << endl;
}

void APIBlogger::slotCreateMedia( const QList<QVariant> &result, const QVariant &id ){
  kDebug()<< "Sending Media is not available in Blogger API." << endl;
  emit error ( i18n(  "Sending Media is not available in Blogger API." ) ); 
}

void APIBlogger::faultSlot( int number, const QString& errorString, const QVariant& id )
{
  emit error( errorString );
}

bool APIBlogger::readPostingFromMap( BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  // FIXME: integrate error handling
  if ( !post ) return false;
  QStringList mapkeys = postInfo.keys();
  kDebug() << endl << "Keys: " << mapkeys.join(", ") << endl << endl;
  
  KDateTime dt( postInfo[ "dateCreated" ].toDateTime() );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt );
  }
  dt = KDateTime ( postInfo[ "lastModified" ].toDateTime() );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt );
  }
  post->setUserId( postInfo[ "userid" ].toString() );
  post->setPostId( postInfo[ "postid" ].toString() );

  QString title( postInfo[ "title" ].toString() );
  QString description( postInfo[ "description" ].toString() );
  QString contents( postInfo[ "content" ].toString() );
  QString category;

  post->setTitle( title );
  post->setContent( contents );
  if ( !category.isEmpty() )
    post->setCategory( category );
  return true;
}

