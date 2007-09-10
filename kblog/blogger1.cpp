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

#include "blogger1.h"
#include "blogger1_p.h"
#include "blogposting.h"

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
  kDebug(5323) << "Blogger1()";
  setUrl( server );
}

Blogger1::Blogger1( const KUrl &server, Blogger1Private &dd, QObject *parent )
  : Blog( server, dd, parent )
{
  kDebug(5323) << "Blogger1()";
  setUrl( server );
}

Blogger1::~Blogger1()
{
  kDebug(5323) << "~Blogger1()";
}

QString Blogger1::interfaceName() const
{
  return QLatin1String( "Blogger 1.0" );
}

void Blogger1::setUrl( const KUrl &server )
{
  Q_D(Blogger1);
  Blog::setUrl( server );
  delete d->mXmlRpcClient;
  d->mXmlRpcClient = new KXmlRpc::Client( server );
  d->mXmlRpcClient->setUserAgent( userAgent() );
}

void Blogger1::fetchUserInfo()
{
    Q_D(Blogger1);
    kDebug(5323) << "Fetch user's info...";
    QList<QVariant> args( d->defaultArgs() );
    d->mXmlRpcClient->call(
      "blogger.getUserInfo", args,
      this, SLOT( slotFetchUserInfo( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT( slotError( int, const QString&, const QVariant& ) ) );
}

void Blogger1::listBlogs()
{
    Q_D(Blogger1);
    kDebug(5323) << "Fetch List of Blogs...";
    QList<QVariant> args( d->defaultArgs() );
    d->mXmlRpcClient->call(
      "blogger.getUsersBlogs", args,
      this, SLOT( slotListBlogs( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT( slotError( int, const QString&, const QVariant& ) ) );
}

void Blogger1::listRecentPostings( int number )
{
    Q_D(Blogger1);
    kDebug(5323) << "Fetching List of Posts...";
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( number );
    d->mXmlRpcClient->call(
      "blogger.getRecentPosts", args,
      this, SLOT( slotListRecentPostings( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT( slotError( int, const QString&, const QVariant& ) ),
               QVariant( number ) );
}

void Blogger1::fetchPosting( KBlog::BlogPosting *posting )
{
  if ( !posting ) {
    kError(5323) << "Blogger1::modifyPosting: posting is null pointer";
    return;
  }
     Q_D(Blogger1);
     kDebug(5323) << "Fetching Posting with url" << posting->postingId();
     QList<QVariant> args( d->defaultArgs( posting->postingId() ) );
     unsigned int i= d->mCallCounter++; // multithreading problem? must be executed at once
     d->mCallMap[ i ] = posting;
     d->mXmlRpcClient->call(
       "blogger.getPost", args,
       this, SLOT( slotFetchPosting( const QList<QVariant>&, const QVariant& ) ),
       this, SLOT( slotError( int, const QString&, const QVariant& ) ),
                QVariant( i ) );
}

void Blogger1::modifyPosting( KBlog::BlogPosting *posting )
{
  Q_D(Blogger1);

  if ( !posting ) {
    kError(5323) << "Blogger1::modifyPosting: posting is null pointer";
    return;
  }
    kDebug(5323) << "Uploading Posting with postingId" << posting->postingId();
    unsigned int i= d->mCallCounter++;
    d->mCallMap[ i ] = posting;
    QList<QVariant> args( d->defaultArgs( posting->postingId() ) );
    QStringList categories = posting->categories();
    QString content = "<title>" + posting->title() + "</title>";
    QStringList::const_iterator it;
    for ( it = categories.constBegin(); it != categories.constEnd(); ++it ) {
      content += "<category>" + *it + "</category>";
    }
    content += posting->content();
    args << QVariant( content );
    args << QVariant( !posting->isPrivate() );
    d->mXmlRpcClient->call(
      "blogger.editPost", args,
      this, SLOT( slotModifyPosting( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT( slotError( int, const QString&, const QVariant& ) ), QVariant( i ) );
}

void Blogger1::createPosting( KBlog::BlogPosting *posting )
{
  Q_D(Blogger1);
  if ( !posting ) {
    kError(5323) << "Blogger1::createPosting: posting is null pointer";
    return;
  }
    unsigned int i= d->mCallCounter++;
    d->mCallMap[ i ] = posting;
    kDebug(5323) << "Creating new Posting with blogid" << blogId();
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    QStringList categories = posting->categories();
    QString content = "<title>" + posting->title() + "</title>";
    QStringList::const_iterator it;
    for ( it = categories.constBegin(); it != categories.constEnd(); ++it ) {
      content += "<category>" + *it + "</category>";
    }
    content += posting->content();
    args << QVariant( content );
    args << QVariant( !posting->isPrivate() );
    d->mXmlRpcClient->call(
      "blogger.newPost", args,
      this, SLOT( slotCreatePosting( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT( slotError( int, const QString&, const QVariant& ) ), QVariant( i ) );
}

void Blogger1::removePosting( KBlog::BlogPosting *posting )
{
 Q_D(Blogger1);
  if ( !posting ) {
    kError(5323) << "Blogger1::removePosting: posting is null pointer";
    return;
  }
  unsigned int i = d->mCallCounter++;
  d->mCallMap[ i ] = posting;
 kDebug(5323) << "Blogger1::removePosting: postingId=" << posting->postingId();
 QList<QVariant> args( d->defaultArgs( posting->postingId() ) );
 args << QVariant( true ); // Publish must be set to remove posting.
 d->mXmlRpcClient->call(
   "blogger.deletePost", args,
   this, SLOT( slotRemovePosting( const QList<QVariant>&, const QVariant& ) ),
   this, SLOT( slotError( int, const QString&, const QVariant& ) ), QVariant( i ) );
}

Blogger1Private::Blogger1Private() :
mXmlRpcClient(0)
{
  mCallCounter = 1;
}

Blogger1Private::~Blogger1Private()
{
  kDebug(5323) << "~Blogger1Private()";
  delete mXmlRpcClient;
}

QList<QVariant> Blogger1Private::defaultArgs( const QString &id )
{
  Q_Q(Blogger1);
  QList<QVariant> args;
  args << QVariant( QString( "0123456789ABCDEF" ) );
  if( !id.isEmpty() )
    args << QVariant( id );
  args << QVariant( q->username() )
          << QVariant( q->password() );
  return args;
}

void Blogger1Private::slotFetchUserInfo(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q(Blogger1);
  Q_UNUSED( id );

  kDebug(5323) << "Blog::slotFetchUserInfo";
  kDebug(5323) << "TOP:" << result[0].typeName();
  QMap<QString,QString> userInfo;
  if ( result[0].type() != QVariant::Map ) {
    kError(5323) << "Could not fetch user's info out of the result from the server,"
                 << "not a map.";
    emit q->error( Blogger1::ParsingError,
                        i18n( "Could not fetch user's info out of the result "
                              "from the server, not a map." ) );
  } else {
    const QMap<QString,QVariant> resultMap = result[0].toMap();
    userInfo["nickname"]=resultMap["nickname"].toString();
    userInfo["userid"]=resultMap["userid"].toString();
    userInfo["url"]=resultMap["url"].toString();
    userInfo["email"]=resultMap["email"].toString();
    userInfo["lastname"]=resultMap["lastname"].toString();
    userInfo["firstname"]=resultMap["firstname"].toString();

    emit q->fetchedUserInfo( userInfo );
  }
}

void Blogger1Private::slotListBlogs(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q(Blogger1);
  Q_UNUSED( id );

  kDebug(5323) << "Blog::slotListBlogs";
  kDebug(5323) << "TOP:" << result[0].typeName();
  QList<QMap<QString,QString> > blogsList;
  if ( result[0].type() != QVariant::List ) {
    kError(5323) << "Could not fetch blogs out of the result from the server,"
                 << "not a list.";
    emit q->error( Blogger1::ParsingError,
                        i18n( "Could not fetch blogs out of the result "
                              "from the server, not a list." ) );
  } else {
    const QList<QVariant> posts = result[0].toList();
    QList<QVariant>::ConstIterator it = posts.begin();
    QList<QVariant>::ConstIterator end = posts.end();
    for ( ; it != end; ++it ) {
      kDebug(5323) << "MIDDLE:" << ( *it ).typeName();
      const QMap<QString, QVariant> postInfo = ( *it ).toMap();
      QMap<QString,QString> blogInfo;
      blogInfo[ "id" ] = postInfo["blogid"].toString();
      blogInfo[ "name" ] = postInfo["blogName"].toString();
      kDebug(5323) << "Blog information retrieved: ID =" << blogInfo["id"]
          << ", Name =" << blogInfo["name"];
      blogsList << blogInfo;
    }
    emit q->listedBlogs( blogsList );
  }
}

void Blogger1Private::slotListRecentPostings(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q(Blogger1);
  int count = id.toInt(); // not sure if needed, actually the API should
// not give more posts

  kDebug(5323) << "Blog::slotListRecentPostings";
  kDebug(5323) << "TOP:" << result[0].typeName();

  QList <BlogPosting> fetchedPostingList;

  if ( result[0].type() != QVariant::List ) {
    kError(5323) << "Could not fetch list of postings out of the"
                 << "result from the server, not a list.";
    emit q->error( Blogger1::ParsingError,
                         i18n( "Could not fetch list of postings out of the "
                               "result from the server, not a list." ) );
  } else {
    const QList<QVariant> postReceived = result[0].toList();
    QList<QVariant>::ConstIterator it = postReceived.begin();
    QList<QVariant>::ConstIterator end = postReceived.end();
    for ( ; it != end; ++it ) {
      BlogPosting posting;
      kDebug(5323) << "MIDDLE:" << ( *it ).typeName();
      const QMap<QString, QVariant> postInfo = ( *it ).toMap();
      if ( readPostingFromMap( &posting, postInfo ) ) {
        kDebug(5323) << "Posting with ID:"
                    << posting.postingId()
                    << "appended in fetchedPostingList";
        fetchedPostingList.append( posting );
      } else {
        kError(5323) << "readPostingFromMap failed!";
        emit q->error( Blogger1::ParsingError,
                             i18n( "Could not read posting." ) );
       }
       if( --count == 0 )
         break;
     }
   }
   kDebug(5323) << "Emitting listRecentPostingsFinished()";
   emit q->listedRecentPostings(fetchedPostingList);
}

void Blogger1Private::slotFetchPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q(Blogger1);
  kDebug(5323) << "Blog::slotFetchPosting";

  KBlog::BlogPosting* posting = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug (5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Map ) {
    kError(5323) << "Could not fetch posting out of the result from "
                  << "the server.";
    emit q->errorPosting( Blogger1::ParsingError,
                          i18n( "Could not fetch posting out of the result"
                                "from the server." ), posting );
    posting->setError( i18n( "Could not fetch posting out of the "
                              "result from the server." ) );
    posting->setStatus( BlogPosting::Error );
  } else {
    const QMap<QString, QVariant> postInfo = result[0].toMap();
    if ( readPostingFromMap( posting, postInfo ) ) {
      kDebug(5323) << "Emitting fetchedPosting( posting.postingId()="
                   << posting->postingId() << ");";
      posting->setStatus( BlogPosting::Fetched );
      emit q->fetchedPosting( posting );
    } else {
      kError(5323) << "readPostingFromMap failed!";
      emit q->errorPosting( Blogger1::ParsingError,
                            i18n( "Could not read posting." ), posting );
      posting->setError( i18n( "Could not read posting." ) );
      posting->setStatus( BlogPosting::Error );
    }
  }

}

void Blogger1Private::slotCreatePosting(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q(Blogger1);
  KBlog::BlogPosting* posting = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  kDebug(5323) << "Blog::slotCreatePosting";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug (5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::String ) {
    kError(5323) << "Could not read the postingId, not a string.";
    emit q->errorPosting( Blogger1::ParsingError,
                          i18n( "Could not read the postingId, not a string." ),
                          posting );
  } else {
    posting->setPostingId( result[0].toString() );
    posting->setStatus( KBlog::BlogPosting::Created );
    emit q->createdPosting( posting );
    kDebug(5323) << "emitting createdPosting()" <<
             "for" << result[0].toInt();
  }

}

void Blogger1Private::slotModifyPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q(Blogger1);
  KBlog::BlogPosting* posting = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  kDebug(5323) << "Blog::slotModifyPosting";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Bool ) {
    kError(5323) << "Could not read the result, not a boolean.";
    emit q->errorPosting( Blogger1::ParsingError,
                          i18n( "Could not read the result, not a boolean." ),
                          posting );
  } else {
    posting->setStatus( KBlog::BlogPosting::Modified );
    emit q->modifiedPosting( posting );
    kDebug(5323) << "emitting modifiedPosting()";
  }
}

void Blogger1Private::slotRemovePosting(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q(Blogger1);
  KBlog::BlogPosting* posting = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );

  kDebug(5323) << "Blog::slotRemovePosting";
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug(5323) << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Bool ) {
    kError(5323) << "Could not read the result, not a boolean.";
    emit q->errorPosting( Blogger1::ParsingError,
                          i18n( "Could not read the result, not a boolean." ),
                          posting );
  } else {
    posting->setStatus( KBlog::BlogPosting::Removed );
    emit q->removedPosting( posting );
    kDebug(5323) << "emitting removedPosting()";
  }
}

void Blogger1Private::slotError( int number,
                                               const QString &errorString,
                                               const QVariant &id )
{
  Q_Q(Blogger1);
  Q_UNUSED( number );
  BlogPosting *posting = mCallMap[ id.toInt() ];

  emit q->errorPosting( Blogger1::XmlRpc, errorString, posting );
}

bool Blogger1Private::readPostingFromMap(
    BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  // FIXME: integrate error handling
  if ( !post ) {
    return false;
  }
  QStringList mapkeys = postInfo.keys();
  kDebug(5323) << endl << "Keys:" << mapkeys.join( ", " );
  kDebug(5323) << endl;

  KDateTime dt( postInfo["dateCreated"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt );
  }
  dt = KDateTime ( postInfo["lastModified"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt );
  }
  post->setPostingId( postInfo["postid"].toString() );

  QString title( postInfo["title"].toString() );
  QString description( postInfo["description"].toString() );
  QString contents( postInfo["content"].toString() );
  QStringList category;

  // Check for hacked title/category support (e.g. in Wordpress)
  QRegExp titleMatch = QRegExp("<title>([^<]*)</title>");
  QRegExp categoryMatch = QRegExp("<category>([^<]*)</category>");
  contents.remove( titleMatch );
  if ( titleMatch.numCaptures() > 0) {
    // Get the title value from the regular expression match
    title = titleMatch.cap( 1 );
  }
  contents.remove( categoryMatch );
  if ( categoryMatch.numCaptures() > 0) {
    // Get the category value from the regular expression match
    category = categoryMatch.capturedTexts();
  }

  post->setTitle( title );
  post->setContent( contents );
  post->setCategories( category );
  return true;
}

#include "blogger1.moc"
