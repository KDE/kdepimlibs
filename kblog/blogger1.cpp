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

#include "blogger1.h"
#include "blogger1_p.h"
#include "blogpost.h"

#include <kxmlrpcclient/client.h>

#include <KDebug>
#include <KDateTime>
#include <KLocale>

#include <QList>

#include <QStringList>

using namespace KBlog;

Blogger1::Blogger1( const KUrl &server, QObject *parent )
  : Blog( server, *new Blogger1Private, parent )
{
  kDebug() << "Blogger1()";
  setUrl( server );
}

Blogger1::Blogger1( const KUrl &server, Blogger1Private &dd, QObject *parent )
  : Blog( server, dd, parent )
{
  kDebug() << "Blogger1()";
  setUrl( server );
}

Blogger1::~Blogger1()
{
  kDebug() << "~Blogger1()";
}

QString Blogger1::interfaceName() const
{
  return QLatin1String( "Blogger 1.0" );
}

void Blogger1::setUrl( const KUrl &server )
{
  Q_D( Blogger1 );
  Blog::setUrl( server );
  delete d->mXmlRpcClient;
  d->mXmlRpcClient = new KXmlRpc::Client( server );
  d->mXmlRpcClient->setUserAgent( userAgent() );
}

void Blogger1::fetchUserInfo()
{
    Q_D( Blogger1 );
    kDebug() << "Fetch user's info...";
    QList<QVariant> args( d->blogger1Args() );
    d->mXmlRpcClient->call(
      "blogger.getUserInfo", args,
      this, SLOT(slotFetchUserInfo(const QList<QVariant>&,const QVariant&)),
      this, SLOT(slotError(int,const QString&,const QVariant&)) );
}

void Blogger1::listBlogs()
{
    Q_D( Blogger1 );
    kDebug() << "Fetch List of Blogs...";
    QList<QVariant> args( d->blogger1Args() );
    d->mXmlRpcClient->call(
      "blogger.getUsersBlogs", args,
      this, SLOT(slotListBlogs(const QList<QVariant>&,const QVariant&)),
      this, SLOT(slotError(int,const QString&,const QVariant&)) );
}

void Blogger1::listRecentPosts( int number )
{
    Q_D( Blogger1 );
    kDebug() << "Fetching List of Posts...";
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( number );
    d->mXmlRpcClient->call(
      d->getCallFromFunction( Blogger1Private::GetRecentPosts ), args,
      this, SLOT(slotListRecentPosts(const QList<QVariant>&,const QVariant&)),
      this, SLOT(slotError(int,const QString&,const QVariant&)),
      QVariant( number ) );
}

void Blogger1::fetchPost( KBlog::BlogPost *post )
{
  if ( !post ) {
    kError() << "Blogger1::modifyPost: post is null pointer";
    return;
  }

  Q_D( Blogger1 );
  kDebug() << "Fetching Post with url" << post->postId();
  QList<QVariant> args( d->defaultArgs( post->postId() ) );
  unsigned int i= d->mCallCounter++;
  d->mCallMap[ i ] = post;
  d->mXmlRpcClient->call(
    d->getCallFromFunction( Blogger1Private::FetchPost ), args,
    this, SLOT(slotFetchPost(const QList<QVariant>&,const QVariant&)),
    this, SLOT(slotError(int, const QString&,const QVariant&)),
    QVariant( i ) );
}

void Blogger1::modifyPost( KBlog::BlogPost *post )
{
  Q_D( Blogger1 );

  if ( !post ) {
    kError() << "Blogger1::modifyPost: post is null pointer";
    return;
  }

  kDebug() << "Uploading Post with postId" << post->postId();
  unsigned int i= d->mCallCounter++;
  d->mCallMap[ i ] = post;
  QList<QVariant> args( d->defaultArgs( post->postId() ) );
  d->readArgsFromPost( &args, *post );
  d->mXmlRpcClient->call(
    d->getCallFromFunction( Blogger1Private::ModifyPost ), args,
    this, SLOT(slotModifyPost(const QList<QVariant>&,const QVariant&)),
    this, SLOT(slotError(int,const QString&,const QVariant&)),
    QVariant( i ) );
}

void Blogger1::createPost( KBlog::BlogPost *post )
{
  Q_D( Blogger1 );

  if ( !post ) {
    kError() << "Blogger1::createPost: post is null pointer";
    return;
  }

  unsigned int i= d->mCallCounter++;
  d->mCallMap[ i ] = post;
  kDebug() << "Creating new Post with blogid" << blogId();
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  d->readArgsFromPost( &args, *post );
  d->mXmlRpcClient->call(
    d->getCallFromFunction( Blogger1Private::CreatePost ), args,
    this, SLOT(slotCreatePost(const QList<QVariant>&,const QVariant&)),
    this, SLOT(slotError(int, const QString&,const QVariant&)),
    QVariant( i ) );
}

void Blogger1::removePost( KBlog::BlogPost *post )
{
  Q_D( Blogger1 );

  if ( !post ) {
    kError() << "Blogger1::removePost: post is null pointer";
    return;
  }

 unsigned int i = d->mCallCounter++;
 d->mCallMap[ i ] = post;
 kDebug() << "Blogger1::removePost: postId=" << post->postId();
 QList<QVariant> args( d->blogger1Args( post->postId() ) );
 args << QVariant( true ); // Publish must be set to remove post.
 d->mXmlRpcClient->call(
   "blogger.deletePost", args,
   this, SLOT(slotRemovePost(const QList<QVariant>&,const QVariant&)),
   this, SLOT(slotError(int,const QString&,const QVariant&)),
   QVariant( i ) );
}

Blogger1Private::Blogger1Private() :
mXmlRpcClient(0)
{
  mCallCounter = 1;
}

Blogger1Private::~Blogger1Private()
{
  kDebug() << "~Blogger1Private()";
  delete mXmlRpcClient;
}

QList<QVariant> Blogger1Private::defaultArgs( const QString &id )
{
  Q_Q ( Blogger1 );
  QList<QVariant> args;
  args << QVariant( QString( "0123456789ABCDEF" ) );
  if( !id.isEmpty() ) {
    args << QVariant( id );
  }
  else {
    //Blog ID is a required parameter, guess at a default value
    args << QVariant( "0" );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

// reimplemenet defaultArgs, since we may not use it virtually everywhere
QList<QVariant> Blogger1Private::blogger1Args( const QString &id )
{
  return defaultArgs( id );
}

void Blogger1Private::slotFetchUserInfo( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  Q_UNUSED( id );

  kDebug() << "Blog::slotFetchUserInfo";
  kDebug() << "TOP:" << result[0].typeName();
  QMap<QString,QString> userInfo;
  if ( result[0].type() != QVariant::Map ) {
    kError() << "Could not fetch user's info out of the result from the server,"
                 << "not a map.";
    emit q->error( Blogger1::ParsingError,
                        i18n( "Could not fetch user's info out of the result "
                              "from the server, not a map." ) );
    return;
  }
  const QMap<QString,QVariant> resultMap = result[0].toMap();
  userInfo["nickname"]=resultMap["nickname"].toString();
  userInfo["userid"]=resultMap["userid"].toString();
  userInfo["url"]=resultMap["url"].toString();
  userInfo["email"]=resultMap["email"].toString();
  userInfo["lastname"]=resultMap["lastname"].toString();
  userInfo["firstname"]=resultMap["firstname"].toString();

  emit q->fetchedUserInfo( userInfo );
}

void Blogger1Private::slotListBlogs( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  Q_UNUSED( id );

  kDebug() << "Blog::slotListBlogs";
  kDebug() << "TOP:" << result[0].typeName();
  QList<QMap<QString,QString> > blogsList;
  if ( result[0].type() != QVariant::List ) {
    kError() << "Could not fetch blogs out of the result from the server,"
                 << "not a list.";
    emit q->error( Blogger1::ParsingError,
                        i18n( "Could not fetch blogs out of the result "
                              "from the server, not a list." ) );
    return;
  }
  const QList<QVariant> posts = result[0].toList();
  QList<QVariant>::ConstIterator it = posts.begin();
  QList<QVariant>::ConstIterator end = posts.end();
  for ( ; it != end; ++it ) {
    kDebug() << "MIDDLE:" << ( *it ).typeName();
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    QMap<QString,QString> blogInfo;
    blogInfo[ "id" ] = postInfo["blogid"].toString();
    blogInfo[ "name" ] = postInfo["blogName"].toString();
    kDebug() << "Blog information retrieved: ID =" << blogInfo["id"]
        << ", Name =" << blogInfo["name"];
    blogsList << blogInfo;
  }
  emit q->listedBlogs( blogsList );
}

void Blogger1Private::slotListRecentPosts( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  int count = id.toInt(); // not sure if needed, actually the API should
// not give more posts

  kDebug() << "Blog::slotListRecentPosts";
  kDebug() << "TOP:" << result[0].typeName();

  QList <BlogPost> fetchedPostList;

  if ( result[0].type() != QVariant::List ) {
    kError() << "Could not fetch list of posts out of the"
                 << "result from the server, not a list.";
    emit q->error( Blogger1::ParsingError,
                   i18n( "Could not fetch list of posts out of the result "
                         "from the server, not a list." ) );
    return;
  }
  const QList<QVariant> postReceived = result[0].toList();
  QList<QVariant>::ConstIterator it = postReceived.begin();
  QList<QVariant>::ConstIterator end = postReceived.end();
  for ( ; it != end; ++it ) {
    BlogPost post;
    kDebug() << "MIDDLE:" << ( *it ).typeName();
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    if ( readPostFromMap( &post, postInfo ) ) {
      kDebug() << "Post with ID:"
                    << post.postId()
                    << "appended in fetchedPostList";
      post.setStatus( BlogPost::Fetched );
      fetchedPostList.append( post );
    } else {
      kError() << "readPostFromMap failed!";
      emit q->error( Blogger1::ParsingError, i18n( "Could not read post." ) );
    }
    if ( --count == 0 ) {
      break;
    }
  }
  kDebug() << "Emitting listRecentPostsFinished()";
  emit q->listedRecentPosts( fetchedPostList );
}

void Blogger1Private::slotFetchPost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  kDebug() << "Blog::slotFetchPost";

  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug () << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Map ) {
    kError() << "Could not fetch post out of the result from "
                  << "the server.";
    post->setError( i18n( "Could not fetch post out of the "
                              "result from the server." ) );
    post->setStatus( BlogPost::Error );
    emit q->errorPost( Blogger1::ParsingError,
                       i18n( "Could not fetch post out of the result "
                           "from the server." ), post );
    return;
  }
}

void Blogger1Private::slotCreatePost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  kDebug() << "Blog::slotCreatePost";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug () << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::String && result[0].type() != QVariant::Int ) {
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
  post->setStatus( KBlog::BlogPost::Created );
  kDebug() << "emitting createdPost()"
                << "for title: \"" << post->title()
                << "\" server id: " << serverID;
  emit q->createdPost( post );
}

void Blogger1Private::slotModifyPost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  kDebug() << "Blog::slotModifyPost";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug() << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Bool ) {
    kError() << "Could not read the result, not a boolean.";
    emit q->errorPost( Blogger1::ParsingError,
                          i18n( "Could not read the result, not a boolean." ),
                          post );
    return;
  }
  post->setStatus( KBlog::BlogPost::Modified );
  kDebug() << "emitting modifiedPost() for title: \""
      << post->title() << "\"";
  emit q->modifiedPost( post );
}

void Blogger1Private::slotRemovePost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  kDebug() << "Blog::slotRemovePost";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug() << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Bool ) {
    kError() << "Could not read the result, not a boolean.";
    emit q->errorPost( Blogger1::ParsingError,
                          i18n( "Could not read the result, not a boolean." ),
                          post );
    return;
  }
  post->setStatus( KBlog::BlogPost::Removed );
  kDebug() << "emitting removedPost()";
  emit q->removedPost( post );
}

void Blogger1Private::slotError( int number,
                                 const QString &errorString,
                                 const QVariant &id )
{
  Q_Q( Blogger1 );
  Q_UNUSED( number );
  BlogPost *post = mCallMap[ id.toInt() ];

  emit q->errorPost( Blogger1::XmlRpc, errorString, post );
}

bool Blogger1Private::readPostFromMap(
    BlogPost *post, const QMap<QString, QVariant> &postInfo )
{
  // FIXME: integrate error handling
  if ( !post ) {
    return false;
  }
  QStringList mapkeys = postInfo.keys();
  kDebug() << endl << "Keys:" << mapkeys.join( ", " );
  kDebug() << endl;

  KDateTime dt( postInfo["dateCreated"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt );
  }
  dt = KDateTime ( postInfo["lastModified"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt );
  }
  post->setPostId( postInfo["postid"].toString() );

  QString title( postInfo["title"].toString() );
  QString description( postInfo["description"].toString() );
  QString contents( postInfo["content"].toString() );
  QStringList category;

  // Check for hacked title/category support (e.g. in Wordpress)
  QRegExp titleMatch = QRegExp( "<title>([^<]*)</title>" );
  QRegExp categoryMatch = QRegExp( "<category>([^<]*)</category>" );
  contents.remove( titleMatch );
  if ( titleMatch.numCaptures() > 0 ) {
    // Get the title value from the regular expression match
    title = titleMatch.cap( 1 );
  }
  contents.remove( categoryMatch );
  if ( categoryMatch.numCaptures() > 0 ) {
    // Get the category value from the regular expression match
    category = categoryMatch.capturedTexts();
  }

  post->setTitle( title );
  post->setContent( contents );
  post->setCategories( category );
  return true;
}

bool Blogger1Private::readArgsFromPost( QList<QVariant> *args, const BlogPost &post )
{
  if ( !args ) {
    return false;
  }
  QStringList categories = post.categories();
  QString content = "<title>" + post.title() + "</title>";
  QStringList::const_iterator it;
  for ( it = categories.constBegin(); it != categories.constEnd(); ++it ) {
    content += "<category>" + *it + "</category>";
  }
  content += post.content();
  *args << QVariant( content );
  *args << QVariant( !post.isPrivate() );
  return true;
}

QString Blogger1Private::getCallFromFunction( FunctionToCall type )
{
  switch ( type ) {
    case GetRecentPosts: return "blogger.getRecentPosts";
    case CreatePost:        return "blogger.newPost";
    case ModifyPost:       return "blogger.editPost";
    case FetchPost:        return "blogger.getPost";
    default: return QString();
  }
}

#include "blogger1.moc"
