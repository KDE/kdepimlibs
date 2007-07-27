/*
  This file is part of the kblog library.

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

#include "blog_p.h"
#include "blogposting.h"

#include <kxmlrpcclient/client.h>

#include <KDebug>
#include <KDateTime>
#include <KLocale>

#include <QList>

using namespace KBlog;

BlogPrivate::BlogPrivate()
{
  mXmlRpcClient = 0;
  callCounter = 1;
}

BlogPrivate::~BlogPrivate()
{
}

QList<QVariant> BlogPrivate::defaultArgs( const QString &id )
{
  QList<QVariant> args;
  if ( !id.isNull() ) {
    args << QVariant( id );
  }
  args << QVariant( q_ptr->username() )
       << QVariant( q_ptr->password() );
  return args;
}

void BlogPrivate::slotListBlogs(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( id );

  kDebug(5323) << "Blog::slotListBlogs" << endl;
  kDebug(5323) << "TOP: " << result[0].typeName() << endl;
  QMap<QString,QString> blogsInfo;
  if ( result[0].type() != QVariant::List ) {
    kDebug(5323) << "Could not fetch blogs out of the result from the server, "
                 << "not a list." << endl;
    emit q_ptr->error( KBlog::Blog::ParsingError,
                        i18n( "Could not fetch blogs out of the result "
                              "from the server, not a list." ) );
  } else {
    const QList<QVariant> posts = result[0].toList();
    QList<QVariant>::ConstIterator it = posts.begin();
    QList<QVariant>::ConstIterator end = posts.end();
    for ( ; it != end; ++it ) {
      kDebug(5323) << "MIDDLE: " << ( *it ).typeName() << endl;
      const QMap<QString, QVariant> postInfo = ( *it ).toMap();

      QString blogId = postInfo["blogId"].toString();
      QString blogName = postInfo["blogName"].toString();
      if ( blogId.isEmpty() && !blogName.isEmpty() ) {
        kDebug(5323) << "blogs infos retrieved id=" << blogsInfo["id"]
                     << ", name=" << blogsInfo["name"] << endl;
        blogsInfo[blogId]=blogName;
      }
    }
    emit q_ptr->listedBlogs( blogsInfo );
  }
}

void BlogPrivate::slotListRecentPostings(
    const QList<QVariant> &result, const QVariant &id )
{
   int count = id.toInt();

   kDebug(5323) << "Blog::slotListRecentPostings" << endl;
   kDebug(5323) << "TOP: " << result[0].typeName() << endl;

   QList <BlogPosting*> fetchedPostingList;

   if ( result[0].type() != QVariant::List ) {
     kDebug(5323) << "Could not fetch list of postings out of the "
                  << "result from the server, not a list." << endl;
     emit q_ptr->error( KBlog::Blog::ParsingError,
                         i18n( "Could not fetch list of postings out of the "
                               "result from the server, not a list." ) );
   } else {
     const QList<QVariant> postReceived = result[0].toList();
     QList<QVariant>::ConstIterator it = postReceived.begin();
     QList<QVariant>::ConstIterator end = postReceived.end();
     for ( ; it != end; ++it ) {
       BlogPosting* posting = new BlogPosting;
       kDebug(5323) << "MIDDLE: " << ( *it ).typeName() << endl;
       const QMap<QString, QVariant> postInfo = ( *it ).toMap();
       if ( readPostingFromMap( posting, postInfo ) ) {
         kDebug(5323) << "Posting with ID:"
                     << posting->postingId()
                     << " appended in fetchedPostingList" << endl;
         fetchedPostingList << posting;
       } else {
         kDebug(5323) << "d->readPostingFromMap failed! " << endl;
         emit q_ptr->error( KBlog::Blog::ParsingError,
                             i18n( "Could not read posting." ) );
       }
       if( --count == 0 )
         break;
     }
   }
   kDebug(5323) << "Emitting listRecentPostingsFinished()" << endl;
   emit q_ptr->listedRecentPostings(fetchedPostingList);
}

void BlogPrivate::slotFetchPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  kDebug(5323) << "Blog::slotFetchPosting" << endl;

//   if( !callMap[ id.toInt() ] ){
//     kDebug(5323) << "Could not map the the id back to the posting. " << endl;
//     break;
//   }

  KBlog::BlogPosting* posting = callMap[ id.toInt() ];

  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug (5323) << "TOP: " << result[0].typeName() << endl;
  if ( result[0].type() != QVariant::Map ) {
    kDebug (5323) << "Could not fetch posting out of the result from "
                  << "the server." << endl;
    emit q_ptr->error( KBlog::Blog::ParsingError,
                        i18n( "Could not fetch posting out of the result from "
                              "the server." ) );
    posting->setError( i18n( "Could not fetch posting out of the "
                              "result from the server." ) );
//    emit posting->statusChanged( KBlog::BlogPosting::Error );
  } else {
    const QMap<QString, QVariant> postInfo = result[0].toMap();
    if ( readPostingFromMap( posting, postInfo ) ) {
      kDebug(5323) << "Emitting fetchedPosting( posting.postingId()="
                   << posting->postingId() << "); " << endl;
    } else {
      kDebug(5323) << "d->readPostingFromMap failed! " << endl;
      emit q_ptr->error( KBlog::Blog::ParsingError,
                          i18n( "Could not read posting." ) );
    }
  }
  callMap.remove( id.toInt() );
}

void BlogPrivate::slotCreatePosting(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( id );

  kDebug(5323) << "Blog::slotCreatePosting" << endl;
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug (5323) << "TOP: " << result[0].typeName() << endl;
  if ( result[0].type() != QVariant::Int ) {
    kDebug(5323) << "Could not read the postingId, not an integer." << endl;
    emit q_ptr->error( KBlog::Blog::ParsingError,
                        i18n( "Could not read the postingId, not an integer." ) );
  } else {
//     emit q_ptr->createdPosting( QString().setNum( result[0].toInt() ) );
    kDebug(5323) << "emitting createdPosting( " << result[0].toInt()
                 << " )" << endl;
  }
}

void BlogPrivate::slotModifyPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( id );

  kDebug(5323) << "Blog::slotModifyPosting" << endl;
  //array of structs containing ISO.8601
  // dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug(5323) << "TOP: " << result[0].typeName() << endl;
  if ( result[0].type() != QVariant::Bool ) {
    kDebug (5323) << "Could not read the result, not a boolean." << endl;
    emit q_ptr->error( KBlog::Blog::ParsingError,
                        i18n( "Could not read the result, not a boolean." ) );
  } else {
//     emit q_ptr->modifiedPosting( result[0].toBool() );
    kDebug(5323) << "emitting modifiedPosting( " << result[0].toBool()
                 << " )" << endl;
  }
}

void BlogPrivate::slotError( int number,
                                               const QString &errorString,
                                               const QVariant &id )
{
  Q_UNUSED( number );
  Q_UNUSED( id );

  emit q_ptr->error( KBlog::Blog::XmlRpc, errorString );
}

bool BlogPrivate::readPostingFromMap(
    BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  // FIXME: integrate error handling
  if ( !post ) {
    return false;
  }
  QStringList mapkeys = postInfo.keys();
  kDebug(5323) << endl << "Keys: " << mapkeys.join( ", " ) << endl << endl;

  KDateTime dt( postInfo["dateCreated"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt );
  }
  dt = KDateTime ( postInfo["lastModified"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt );
  }
  //TODO remove if sure that not needed
  //post->setUserId( postInfo["userid"].toString() );
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

#include "blog_p.moc"
