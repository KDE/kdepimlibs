/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
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

#include "blogger1.h"
#include "blogger1_p.h"
#include "blogpost.h"

#include <kxmlrpcclient/client.h>

#include <QDebug>
#include <KDateTime>
#include <KLocalizedString>

#include <QList>

#include <QStringList>

using namespace KBlog;

Blogger1::Blogger1( const QUrl &server, QObject *parent )
  : Blog( server, *new Blogger1Private, parent )
{
  qDebug();
  setUrl( server );
}

Blogger1::Blogger1( const QUrl &server, Blogger1Private &dd, QObject *parent )
  : Blog( server, dd, parent )
{
  qDebug();
  setUrl( server );
}

Blogger1::~Blogger1()
{
  qDebug();
}

QString Blogger1::interfaceName() const
{
  return QLatin1String( "Blogger 1.0" );
}

void Blogger1::setUrl( const QUrl &server )
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
    qDebug() << "Fetch user's info...";
    QList<QVariant> args( d->blogger1Args() );
    d->mXmlRpcClient->call(
      QStringLiteral("blogger.getUserInfo"), args,
      this, SLOT(slotFetchUserInfo(QList<QVariant>,QVariant)),
      this, SLOT(slotError(int,QString,QVariant)) );
}

void Blogger1::listBlogs()
{
    Q_D( Blogger1 );
    qDebug() << "Fetch List of Blogs...";
    QList<QVariant> args( d->blogger1Args() );
    d->mXmlRpcClient->call(
      QStringLiteral("blogger.getUsersBlogs"), args,
      this, SLOT(slotListBlogs(QList<QVariant>,QVariant)),
      this, SLOT(slotError(int,QString,QVariant)) );
}

void Blogger1::listRecentPosts( int number )
{
    Q_D( Blogger1 );
    qDebug() << "Fetching List of Posts...";
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( number );
    d->mXmlRpcClient->call(
      d->getCallFromFunction( Blogger1Private::GetRecentPosts ), args,
      this, SLOT(slotListRecentPosts(QList<QVariant>,QVariant)),
      this, SLOT(slotError(int,QString,QVariant)),
      QVariant( number ) );
}

void Blogger1::fetchPost( KBlog::BlogPost *post )
{
  if ( !post ) {
    qCritical() << "Blogger1::modifyPost: post is null pointer";
    return;
  }

  Q_D( Blogger1 );
  qDebug() << "Fetching Post with url" << post->postId();
  QList<QVariant> args( d->defaultArgs( post->postId() ) );
  unsigned int i= d->mCallCounter++;
  d->mCallMap[ i ] = post;
  d->mXmlRpcClient->call(
    d->getCallFromFunction( Blogger1Private::FetchPost ), args,
    this, SLOT(slotFetchPost(QList<QVariant>,QVariant)),
    this, SLOT(slotError(int,QString,QVariant)),
    QVariant( i ) );
}

void Blogger1::modifyPost( KBlog::BlogPost *post )
{
  Q_D( Blogger1 );

  if ( !post ) {
    qCritical() << "Blogger1::modifyPost: post is null pointer";
    return;
  }

  qDebug() << "Uploading Post with postId" << post->postId();
  unsigned int i= d->mCallCounter++;
  d->mCallMap[ i ] = post;
  QList<QVariant> args( d->defaultArgs( post->postId() ) );
  d->readArgsFromPost( &args, *post );
  d->mXmlRpcClient->call(
    d->getCallFromFunction( Blogger1Private::ModifyPost ), args,
    this, SLOT(slotModifyPost(QList<QVariant>,QVariant)),
    this, SLOT(slotError(int,QString,QVariant)),
    QVariant( i ) );
}

void Blogger1::createPost( KBlog::BlogPost *post )
{
  Q_D( Blogger1 );

  if ( !post ) {
    qCritical() << "Blogger1::createPost: post is null pointer";
    return;
  }

  unsigned int i= d->mCallCounter++;
  d->mCallMap[ i ] = post;
  qDebug() << "Creating new Post with blogid" << blogId();
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  d->readArgsFromPost( &args, *post );
  d->mXmlRpcClient->call(
    d->getCallFromFunction( Blogger1Private::CreatePost ), args,
    this, SLOT(slotCreatePost(QList<QVariant>,QVariant)),
    this, SLOT(slotError(int,QString,QVariant)),
    QVariant( i ) );
}

void Blogger1::removePost( KBlog::BlogPost *post )
{
  Q_D( Blogger1 );

  if ( !post ) {
    qCritical() << "Blogger1::removePost: post is null pointer";
    return;
  }

 unsigned int i = d->mCallCounter++;
 d->mCallMap[ i ] = post;
 qDebug() << "Blogger1::removePost: postId=" << post->postId();
 QList<QVariant> args( d->blogger1Args( post->postId() ) );
 args << QVariant( true ); // Publish must be set to remove post.
 d->mXmlRpcClient->call(
   QStringLiteral("blogger.deletePost"), args,
   this, SLOT(slotRemovePost(QList<QVariant>,QVariant)),
   this, SLOT(slotError(int,QString,QVariant)),
   QVariant( i ) );
}

Blogger1Private::Blogger1Private() :
mXmlRpcClient(0)
{
  qDebug();
  mCallCounter = 1;
}

Blogger1Private::~Blogger1Private()
{
  qDebug();
  delete mXmlRpcClient;
}

QList<QVariant> Blogger1Private::defaultArgs( const QString &id )
{
  qDebug();
  Q_Q ( Blogger1 );
  QList<QVariant> args;
  args << QVariant( QLatin1String( "0123456789ABCDEF" ) );
  if ( !id.isEmpty() ) {
    args << QVariant( id );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

// reimplemenet defaultArgs, since we may not use it virtually everywhere
QList<QVariant> Blogger1Private::blogger1Args( const QString &id )
{
  qDebug();
  Q_Q ( Blogger1 );
  QList<QVariant> args;
  args << QVariant( QLatin1String( "0123456789ABCDEF" ) );
  if ( !id.isEmpty() ) {
    args << QVariant( id );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

void Blogger1Private::slotFetchUserInfo( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  Q_UNUSED( id );

  qDebug();
  qDebug() << "TOP:" << result[0].typeName();
  QMap<QString,QString> userInfo;
  if ( result[0].type() != QVariant::Map ) {
    qCritical() << "Could not fetch user's info out of the result from the server,"
                 << "not a map.";
    emit q->error( Blogger1::ParsingError,
                        i18n( "Could not fetch user's info out of the result "
                              "from the server, not a map." ) );
    return;
  }
  const QMap<QString,QVariant> resultMap = result[0].toMap();
  userInfo[QStringLiteral("nickname")]=resultMap[QStringLiteral("nickname")].toString();
  userInfo[QStringLiteral("userid")]=resultMap[QStringLiteral("userid")].toString();
  userInfo[QStringLiteral("url")]=resultMap[QStringLiteral("url")].toString();
  userInfo[QStringLiteral("email")]=resultMap[QStringLiteral("email")].toString();
  userInfo[QStringLiteral("lastname")]=resultMap[QStringLiteral("lastname")].toString();
  userInfo[QStringLiteral("firstname")]=resultMap[QStringLiteral("firstname")].toString();

  emit q->fetchedUserInfo( userInfo );
}

void Blogger1Private::slotListBlogs( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  Q_UNUSED( id );

  qDebug();
  qDebug() << "TOP:" << result[0].typeName();
  QList<QMap<QString,QString> > blogsList;
  if ( result[0].type() != QVariant::List ) {
    qCritical() << "Could not fetch blogs out of the result from the server,"
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
    qDebug() << "MIDDLE:" << ( *it ).typeName();
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    QMap<QString,QString> blogInfo;
    blogInfo[ QStringLiteral("id") ] = postInfo[QStringLiteral("blogid")].toString();
    blogInfo[ QStringLiteral("url") ] = postInfo[QStringLiteral("url")].toString();
    blogInfo[ QStringLiteral("apiUrl") ] = postInfo[QStringLiteral("xmlrpc")].toString();
    blogInfo[ QStringLiteral("title") ] = postInfo[QStringLiteral("blogName")].toString();
    qDebug() << "Blog information retrieved: ID =" << blogInfo[QStringLiteral("id")]
        << ", Name =" << blogInfo[QStringLiteral("title")];
    blogsList << blogInfo;
  }
  emit q->listedBlogs( blogsList );
}

void Blogger1Private::slotListRecentPosts( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  int count = id.toInt(); // not sure if needed, actually the API should
// not give more posts

  qDebug();
  qDebug() << "TOP:" << result[0].typeName();

  QList <BlogPost> fetchedPostList;

  if ( result[0].type() != QVariant::List ) {
    qCritical() << "Could not fetch list of posts out of the"
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
    qDebug() << "MIDDLE:" << ( *it ).typeName();
    const QMap<QString, QVariant> postInfo = ( *it ).toMap();
    if ( readPostFromMap( &post, postInfo ) ) {
      qDebug() << "Post with ID:"
                    << post.postId()
                    << "appended in fetchedPostList";
      post.setStatus( BlogPost::Fetched );
      fetchedPostList.append( post );
    } else {
      qCritical() << "readPostFromMap failed!";
      emit q->error( Blogger1::ParsingError, i18n( "Could not read post." ) );
    }
    if ( --count == 0 ) {
      break;
    }
  }
  qDebug() << "Emitting listRecentPostsFinished()";
  emit q->listedRecentPosts( fetchedPostList );
}

void Blogger1Private::slotFetchPost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  qDebug();

  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  qDebug () << "TOP:" << result[0].typeName();
  if ( result[0].type() == QVariant::Map &&
       readPostFromMap( post, result[0].toMap() ) ) {
    qDebug() << "Emitting fetchedPost()";
    post->setStatus( KBlog::BlogPost::Fetched );
    emit q->fetchedPost( post );
  } else {
    qCritical() << "Could not fetch post out of the result from the server.";
    post->setError( i18n( "Could not fetch post out of the result from the server." ) );
    post->setStatus( BlogPost::Error );
    emit q->errorPost( Blogger1::ParsingError,
                       i18n( "Could not fetch post out of the result from the server." ), post );
  }
}

void Blogger1Private::slotCreatePost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  qDebug();
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  qDebug () << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::String &&
       result[0].type() != QVariant::Int ) {
    qCritical() << "Could not read the postId, not a string or an integer.";
    emit q->errorPost( Blogger1::ParsingError,
                          i18n( "Could not read the postId, not a string or an integer." ),
                          post );
    return;
  }
  QString serverID;
  if ( result[0].type() == QVariant::String ) {
    serverID = result[0].toString();
  } else if ( result[0].type() == QVariant::Int ) {
    serverID = QString::fromLatin1( "%1" ).arg( result[0].toInt() );
  }
  post->setPostId( serverID );
  post->setStatus( KBlog::BlogPost::Created );
  qDebug() << "emitting createdPost()"
                << "for title: \"" << post->title()
                << "\" server id: " << serverID;
  emit q->createdPost( post );
}

void Blogger1Private::slotModifyPost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  qDebug();
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  qDebug() << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Bool &&
       result[0].type() != QVariant::Int ) {
    qCritical() << "Could not read the result, not a boolean.";
    emit q->errorPost( Blogger1::ParsingError,
                          i18n( "Could not read the result, not a boolean." ),
                          post );
    return;
  }
  post->setStatus( KBlog::BlogPost::Modified );
  qDebug() << "emitting modifiedPost() for title: \""
      << post->title() << "\"";
  emit q->modifiedPost( post );
}

void Blogger1Private::slotRemovePost( const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( Blogger1 );
  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  qDebug() << "slotRemovePost";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  qDebug() << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Bool &&
       result[0].type() != QVariant::Int ) {
    qCritical() << "Could not read the result, not a boolean.";
    emit q->errorPost( Blogger1::ParsingError,
                          i18n( "Could not read the result, not a boolean." ),
                          post );
    return;
  }
  post->setStatus( KBlog::BlogPost::Removed );
  qDebug() << "emitting removedPost()";
  emit q->removedPost( post );
}

void Blogger1Private::slotError( int number,
                                 const QString &errorString,
                                 const QVariant &id )
{
  Q_Q( Blogger1 );
  Q_UNUSED( number );
  qDebug() << "An error occurred: " << errorString;
  BlogPost *post = mCallMap[ id.toInt() ];

  if ( post )
    emit q->errorPost( Blogger1::XmlRpc, errorString, post );
  else
    emit q->error( Blogger1::XmlRpc, errorString );
}

bool Blogger1Private::readPostFromMap(
    BlogPost *post, const QMap<QString, QVariant> &postInfo )
{
  // FIXME: integrate error handling
  if ( !post ) {
    return false;
  }
  QStringList mapkeys = postInfo.keys();
  qDebug() << endl << "Keys:" << mapkeys.join( QStringLiteral(", ") );
  qDebug() << endl;

  KDateTime dt( postInfo[QStringLiteral("dateCreated")].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt.toLocalZone() );
  }
  dt = KDateTime ( postInfo[QStringLiteral("lastModified")].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt.toLocalZone() );
  }
  post->setPostId( postInfo[QStringLiteral("postid")].toString().isEmpty() ? postInfo[QStringLiteral("postId")].toString() :
                   postInfo[QStringLiteral("postid")].toString() );

  QString title( postInfo[QStringLiteral("title")].toString() );
  //QString description( postInfo["description"].toString() );
  QString contents;
  if ( postInfo[QStringLiteral("content")].type() == QVariant::ByteArray ) {
    QByteArray tmpContent = postInfo[QStringLiteral("content")].toByteArray();
    contents = QString::fromUtf8( tmpContent.data(), tmpContent.size() );
  } else {
    contents = postInfo[QStringLiteral("content")].toString();
  }
  QStringList category;

  // Check for hacked title/category support (e.g. in Wordpress)
  QRegExp titleMatch = QRegExp( QStringLiteral("<title>([^<]*)</title>") );
  QRegExp categoryMatch = QRegExp( QStringLiteral("<category>([^<]*)</category>") );
  if ( contents.indexOf( titleMatch ) != -1 ) {
    // Get the title value from the regular expression match
    title = titleMatch.cap( 1 );
  }
  if ( contents.indexOf( categoryMatch ) != -1 ) {
      // Get the category value from the regular expression match
      category = categoryMatch.capturedTexts();
  }
  contents.remove( titleMatch );
  contents.remove( categoryMatch );

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
  const QStringList categories = post.categories();
  QString content = QStringLiteral("<title>") + post.title() + QStringLiteral("</title>");
  QStringList::const_iterator it;
  QStringList::const_iterator end(categories.constEnd());
  for ( it = categories.constBegin(); it != end; ++it ) {
    content += QStringLiteral("<category>") + *it + QStringLiteral("</category>");
  }
  content += post.content();
  *args << QVariant( content );
  *args << QVariant( !post.isPrivate() );
  return true;
}

QString Blogger1Private::getCallFromFunction( FunctionToCall type )
{
  switch ( type ) {
    case GetRecentPosts: return QStringLiteral("blogger.getRecentPosts");
    case CreatePost:        return QStringLiteral("blogger.newPost");
    case ModifyPost:       return QStringLiteral("blogger.editPost");
    case FetchPost:        return QStringLiteral("blogger.getPost");
    default: return QString();
  }
}

#include "moc_blogger1.cpp"
