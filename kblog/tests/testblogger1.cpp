/*
  This file is part of the kblog library.

  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>

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

#include "testblogger1.h"
#include "data.h"
#include "testblogger1.moc"

#include "kblog/blogger1.h"
#include "kblog/blogposting.h"

#include <qtest_kde.h>

#include <unistd.h>
#include <ktimezone.h>
#include <kdatetime.h>
#include <kdebug.h>

#define TIMEOUT 20000
#define GLOBALTIMEOUT 120000
#define DOWNLOADCOUNT 5

using namespace KBlog;

void TestBlogger1::dumpPosting( const BlogPosting* posting )
{
  kDebug(5323) << "########### posting ############";
  kDebug(5323) << "# postingId: " << posting->postingId();
  kDebug(5323) << "# title: " << posting->title();
  kDebug(5323) << "# content: " << posting->content();
  kDebug(5323) << "# publish: " << posting->isPublished();
  kDebug(5323) << "# categories: " << posting->categories().join( " " );
  kDebug(5323) << "# error: " << posting->error();
  kDebug(5323) << "# journalId: " << posting->journalId();
  switch ( posting->status() ){
    case BlogPosting::New:
      kDebug(5323) << "# status: New"; break;
    case BlogPosting::Fetched:
      kDebug(5323) << "# status: Fetched"; break;
    case BlogPosting::Created:
      kDebug(5323) << "# status: Created"; break;
    case BlogPosting::Modified:
      kDebug(5323) << "# status: Modified"; break;
    case BlogPosting::Removed:
      kDebug(5323) << "# status: Removed"; break;
    case BlogPosting::Error:
      kDebug(5323) << "# status: Error"; break;
  };
  kDebug(5323) << "# creationDateTime(UTC): " << 
      posting->creationDateTime().toUtc().toString();
  kDebug(5323) << "# modificationDateTime(UTC): " << 
      posting->modificationDateTime().toUtc().toString();
  kDebug(5323) << "###########################\r\n";
}

void TestBlogger1::fetchUserInfo( const QMap<QString,QString>& userInfo )
{
  fetchUserInfoTimer->stop();
  kDebug(5323) << "########### fetchUserInfo ###########";
  kDebug(5323) << "# nickname: " << userInfo["nickname"];
  kDebug(5323) << "# userid: "  << userInfo["userid"];
  kDebug(5323) << "# url: " <<  userInfo["url"];
  kDebug(5323) << "# email: " <<  userInfo["email"];
  kDebug(5323) << "# lastname: " << userInfo["lastname"];
  kDebug(5323) << "# firstname: " <<  userInfo["firstname"];
  kDebug(5323) << "##############################\r\n";

  connect( b, SIGNAL( listedBlogs( const QMap<QString,QString>& ) ),
           this, SLOT( listBlogs( const QMap<QString,QString>& ) ) );
  b->listBlogs();
  listBlogsTimer->start( TIMEOUT );
}

void TestBlogger1::listBlogs( const QMap<QString,QString>& listedBlogs )
{
  listBlogsTimer->stop();
  QList<QString> keys = listedBlogs.keys();
  kDebug(5323) << "########### listBlogs ###########";
  QList<QString>::ConstIterator it = keys.begin();
  QList<QString>::ConstIterator end = keys.end();
  for ( ; it != end; ++it ) {
    kDebug(5323) << "# " << ( *it ) << ": " << listedBlogs[ ( *it ) ];
  }
  kDebug(5323) << "###########################\r\n";

  connect( b, SIGNAL( listedRecentPostings(const QList<KBlog::BlogPosting>&) ),
           this, SLOT( listRecentPostings(const QList<KBlog::BlogPosting>&) ) );
  b->listRecentPostings( DOWNLOADCOUNT );
  listRecentPostingsTimer->start( TIMEOUT );
}

void TestBlogger1::listRecentPostings( 
           const QList<KBlog::BlogPosting>& postings )
{
  listRecentPostingsTimer->stop();
  kDebug(5323) << "########### listRecentPostings ###########";
  QList<KBlog::BlogPosting>::ConstIterator it = postings.begin();
  QList<KBlog::BlogPosting>::ConstIterator end = postings.end();
  for ( ; it != end; ++it ) {
    dumpPosting( &( *it ) );
  }
  kDebug(5323) << "#################################\r\n";

  connect( b, SIGNAL( createdPosting( KBlog::BlogPosting* ) ),
           this, SLOT( createPosting( KBlog::BlogPosting* ) ) );
  b->createPosting( p ); // start chain
  createPostingTimer->start( TIMEOUT );
}

void TestBlogger1::createPosting( KBlog::BlogPosting *posting )
{
  createPostingTimer->stop();
  kDebug(5323) << "########### createPosting ############";
  dumpPosting( posting );
  kDebug(5323) << "################################\r\n";
  QVERIFY( posting->status() == BlogPosting::Created );

  connect( b, SIGNAL( modifiedPosting( KBlog::BlogPosting* ) ),
           this, SLOT( modifyPosting( KBlog::BlogPosting* ) ) );
  p->setContent( mModifiedContent );
  b->modifyPosting( p );
  modifyPostingTimer->start( TIMEOUT );
}

void TestBlogger1::fetchPosting( KBlog::BlogPosting *posting )
{
  fetchPostingTimer->stop();
  kDebug(5323) << "########### fetchPosting ############";
  dumpPosting( posting );
  kDebug(5323) << "###############################\r\n";
  QVERIFY( posting->status() == BlogPosting::Fetched );
  QVERIFY( posting->postingId() == mPostingId );
  QVERIFY( posting->content() == mModifiedContent );

  connect( b, SIGNAL( removedPosting( KBlog::BlogPosting* ) ),
           this, SLOT( removePosting( KBlog::BlogPosting* ) ) );
  b->removePosting( p );
  removePostingTimer->start( TIMEOUT );
}

void TestBlogger1::modifyPosting( KBlog::BlogPosting *posting )
{
  modifyPostingTimer->stop();
  kDebug(5323) << "########### modifyPosting ############";
  dumpPosting( posting );
  kDebug(5323) << "################################\r\n";
  QVERIFY( posting->status() == BlogPosting::Modified );

  connect( b, SIGNAL( fetchedPosting( KBlog::BlogPosting* ) ),
           this, SLOT( fetchPosting( KBlog::BlogPosting* ) ) );
  p->setContent( "TestBlogger1: created content." );
  b->fetchPosting( p );
  fetchPostingTimer->start( TIMEOUT );
}


void TestBlogger1::removePosting( KBlog::BlogPosting *posting )
{
  removePostingTimer->stop();
  kDebug(5323) << "########### removePosting ###########";
  dumpPosting( posting );
  kDebug(5323) << "################################\r\n";
  QVERIFY( posting->status() == BlogPosting::Removed );
}

void TestBlogger1Warnings::fetchUserInfoTimeoutWarning()
{
  QWARN( "fetchUserInfo() timeout. This can be caused by an error, too." );
}

void TestBlogger1Warnings::listBlogsTimeoutWarning()
{
  QWARN( "listBlogs()  timeout. This can be caused by an error, too." );
}


void TestBlogger1Warnings::listRecentPostingsTimeoutWarning()
{
  QWARN( "listRecentPostings() timeout. This can be caused by an error, too." );
}

void TestBlogger1Warnings::fetchPostingTimeoutWarning()
{
  QWARN( "fetchPosting() timeout. This can be caused by an error, too." );
}

void TestBlogger1Warnings::modifyPostingTimeoutWarning()
{
  QWARN( "modifyPosting() timeout. This can be caused by an error, too." );
}

void TestBlogger1Warnings::createPostingTimeoutWarning()
{
  QWARN( "createPosting() timeout. This can be caused by an error, too." );
}

void TestBlogger1Warnings::removePostingTimeoutWarning()
{
  QWARN( "removePosting() timeout. This can be caused by an error, too." );
}

void TestBlogger1Warnings::error( KBlog::Blog::ErrorType type, const QString &errStr,
        KBlog::BlogPosting* posting )
{
  kDebug(5323) << "############ error #############";
  switch ( type ){
    case Blog::Atom: kDebug(5323) << "type: Atom"; break;
    case Blog::XmlRpc: kDebug(5323) << "type: xmlRpc"; break;
    case Blog::ParsingError: kDebug(5323) << "type: ParsingError"; break;
    case Blog::AuthenticationError: kDebug(5323) << "type: AuthenticationError"; break;
    case Blog::NotSupported: kDebug(5323) << "type: NotSupported"; break;
    case Blog::Other: kDebug(5323) << "type: Other"; break;
  };
  kDebug(5323) << "error: " << errStr;
//   if( posting!=0 ) dumpPosting( posting );
  kDebug(5323) << "#############################\r\n";
}

QTEST_KDEMAIN( TestBlogger1, NoGUI )

void TestBlogger1::testValidity()
{
  b = new Blogger1( KUrl( "http://wrong.url.org/somegateway" ) );
  QVERIFY( b->url() == KUrl( "http://wrong.url.org/somegateway" ) );
  b->setUrl( mUrl );
  b->setUsername( mUsername );
  b->setPassword( mPassword );
  b->setBlogId( mBlogId );
  b->setTimeZone( mTimeZone );
  QVERIFY( b->url() == mUrl );
  QVERIFY( b->blogId() == mBlogId );
  QVERIFY( b->username() == mUsername );
  QVERIFY( b->password() == mPassword );
  QVERIFY( b->interfaceName() == "Blogger 1.0" );
  QVERIFY( b->timeZone().name() == mTimeZone.name() );

  p = new BlogPosting();
  p->setTitle( mTitle );
  p->setContent( mContent );
  p->setPublished( mPublished );
  p->setPostingId( mPostingId );
  p->setCreationDateTime( mCreationDateTime );
  p->setModificationDateTime( mModificationDateTime );
  QVERIFY( p->title() == mTitle );
  QVERIFY( p->content() == mContent );
  QVERIFY( p->isPublished() == mPublished );
  QVERIFY( p->postingId() == mPostingId );
  QVERIFY( p->creationDateTime() == mCreationDateTime );
  QVERIFY( p->modificationDateTime() == mModificationDateTime );

  TestBlogger1Warnings *warnings = new TestBlogger1Warnings();
  connect( b, SIGNAL( error( KBlog::Blog::ErrorType, const QString&, KBlog::BlogPosting* ) ),
           warnings, SLOT( error( KBlog::Blog::ErrorType, const QString&, KBlog::BlogPosting* ) ) );

  fetchUserInfoTimer = new QTimer( this );
  fetchUserInfoTimer->setSingleShot( true );
  connect( fetchUserInfoTimer, SIGNAL( timeout() ),
           warnings, SLOT( fetchUserInfoTimeoutWarning() ) );

  listBlogsTimer = new QTimer( this );
  listBlogsTimer->setSingleShot( true );
  connect( listBlogsTimer, SIGNAL( timeout() ),
           warnings, SLOT( listBlogsTimeoutWarning() ) );

  listRecentPostingsTimer = new QTimer( this );
  listRecentPostingsTimer->setSingleShot( true );
  connect( listRecentPostingsTimer, SIGNAL( timeout() ),
           warnings, SLOT( listRecentPostingsTimeoutWarning() ) );

  fetchPostingTimer = new QTimer( this );
  fetchPostingTimer->setSingleShot( true );
  connect( fetchPostingTimer, SIGNAL( timeout() ),
           warnings, SLOT( fetchPostingTimeoutWarning() ) );

  modifyPostingTimer = new QTimer( this );
  modifyPostingTimer->setSingleShot( true );
  connect( modifyPostingTimer, SIGNAL( timeout() ),
           warnings, SLOT( modifyPostingTimeoutWarning() ) );

  createPostingTimer = new QTimer( this );
  createPostingTimer->setSingleShot( true );
  connect( createPostingTimer, SIGNAL( timeout() ),
           warnings, SLOT( createPostingTimeoutWarning() ) );

  removePostingTimer = new QTimer( this );
  removePostingTimer->setSingleShot( true );
  connect( removePostingTimer, SIGNAL( timeout() ),
           warnings, SLOT( removePostingTimeoutWarning() ) );

  QEventLoop *eventLoop = new QEventLoop( this );

  connect( b, SIGNAL( fetchedUserInfo( const QMap<QString,QString>& ) ),
          this, SLOT( fetchUserInfo( const QMap<QString,QString>&) ) );
  b->fetchUserInfo();
  fetchUserInfoTimer->start( TIMEOUT );

// wait for all jobs to finish

  QTimer::singleShot( GLOBALTIMEOUT, eventLoop, SLOT(quit()));
  eventLoop->exec();

  delete b;
  delete p;
}
