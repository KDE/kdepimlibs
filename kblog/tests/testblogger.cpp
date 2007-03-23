/*
  This file is part of the kblog library.

  Copyright (c) 2006 Christian Weilbach <christian_weilbach@web.de>

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

#include <unistd.h>

#include <qtest_kde.h>

#include "testblogger.h"
#include "testblogger.moc"

#include <blogger.h>

#define TIMEOUT 20000
#define GLOBALTIMEOUT 30000
#define POSTINGID 41
#define DOWNLOADCOUNT 5

using namespace KBlog;

void TestBloggerWarnings::userInfoTimeoutWarning()
{
  QWARN( "userInfo() timeout. This can be caused by an error, too." );
}

void TestBloggerWarnings::listBlogsTimeoutWarning()
{
  QWARN( "listBlogs()  timeout. This can be caused by an error, too." );
}

void TestBloggerWarnings::listPostingsTimeoutWarning()
{
  QWARN( "listPostings() timeout. This can be caused by an error, too." );
}

void TestBloggerWarnings::fetchPostingTimeoutWarning()
{
  QWARN( "fetchPosting() timeout. This can be caused by an error, too." );
}

void TestBloggerWarnings::modifyPostingTimeoutWarning()
{
  QWARN( "modifyPosting() timeout. This can be caused by an error, too." );
}

void TestBloggerWarnings::createPostingTimeoutWarning()
{
  QWARN( "createPosting() timeout. This can be caused by an error, too." );
}

void TestBloggerWarnings::error( const errorType &type, const QString &errStr )
{
  QWARN( errStr.toUtf8().data() );
}

QTEST_KDEMAIN( TestBlogger, NoGUI )

  void TestBlogger::testValidity()
{
  APIBlogger *b = new APIBlogger( KUrl( "http://wrong.url.org/somegateway" ) );
  QVERIFY( b->url() == KUrl( "http://wrong.url.org/somegateway" ) );
  b->setUrl( KUrl( "http://10.13.37.101/wordpress/xmlrpc.php" ) );
  b->setUsername( "admin" );
  b->setPassword( "e9f51d" );
  b->setBlogId( "1" );
  b->setTimezone( KTimeZone() );
  b->setDownloadCount( DOWNLOADCOUNT );
  QVERIFY( b->url() == KUrl( "http://10.13.37.101/wordpress/xmlrpc.php" ) );
  QVERIFY( b->blogId() == "1" );
  QVERIFY( b->username() == "admin" );
  QVERIFY( b->password() == "e9f51d" );
  QVERIFY( b->interfaceName() == "Blogger API 1.0" );
  QVERIFY( b->timezone().name() == QString( "UTC" ) );
  QVERIFY( b->downloadCount() == DOWNLOADCOUNT );

  BlogPosting *p = new BlogPosting();
  KDateTime mDateTime( QDateTime::currentDateTime() );
  p->setTitle( "TestBlogger" );
  p->setContent( "TestBlogger: posted content." );
  p->setPublish( true );
  p->setPostingId( QString( "41" ) );
  p->setCreationDateTime( mDateTime );
  p->setModificationDateTime( mDateTime );
  QVERIFY( p->title() == "TestBlogger" );
  QVERIFY( p->content() == "TestBlogger: posted content." );
  QVERIFY( p->publish() == true );
  QVERIFY( p->postingId() == QString ( "41" ) );
  QVERIFY( p->creationDateTime() == mDateTime );
  QVERIFY( p->modificationDateTime() == mDateTime );

  TestBloggerWarnings *warnings = new TestBloggerWarnings();
  connect( b, SIGNAL( error( const errorType&, const QString& ) ),
           warnings, SLOT( error( const errorType&, const QString& ) ) );

  QTimer *userInfoTimer = new QTimer( this );
  userInfoTimer->setSingleShot( true );
  connect( userInfoTimer, SIGNAL( timeout() ),
           warnings, SLOT( userInfoTimeoutWarning() ) );

  QTimer *listBlogsTimer = new QTimer( this );
  listBlogsTimer->setSingleShot( true );
  connect( listBlogsTimer, SIGNAL( timeout() ),
           warnings, SLOT( listBlogsTimeoutWarning() ) );

  QTimer *listPostingsTimer = new QTimer( this );
  listPostingsTimer->setSingleShot( true );
  connect( listPostingsTimer, SIGNAL( timeout() ),
           warnings, SLOT( listPostingsTimeoutWarning() ) );

  QTimer *fetchPostingTimer = new QTimer( this );
  fetchPostingTimer->setSingleShot( true );
  connect( fetchPostingTimer, SIGNAL( timeout() ),
           warnings, SLOT( fetchPostingTimeoutWarning() ) );

  QTimer *modifyPostingTimer = new QTimer( this );
  modifyPostingTimer->setSingleShot( true );
  connect( modifyPostingTimer, SIGNAL( timeout() ),
           warnings, SLOT( modifyPostingTimeoutWarning() ) );

  QTimer *createPostingTimer = new QTimer( this );
  createPostingTimer->setSingleShot( true );
  connect( createPostingTimer, SIGNAL( timeout() ),
           warnings, SLOT( createPostingTimeoutWarning() ) );

  QEventLoop *eventLoop = new QEventLoop( this );

  connect( b, SIGNAL( userInfoRetrieved( const QString&, const QString&, const QString& ) ),
          userInfoTimer, SLOT( stop() ) );
  b->userInfo();
  userInfoTimer->start( TIMEOUT );

  connect( b, SIGNAL( blogInfoRetrieved( const QString&, const QString& ) ),
           listBlogsTimer, SLOT( stop() ) );
  b->listBlogs();
  listBlogsTimer->start( TIMEOUT );

  connect( b, SIGNAL( listPostingsFinished() ),
           listPostingsTimer, SLOT( stop() ) );
  b->listPostings();
  listPostingsTimer->start( TIMEOUT );

  connect( b, SIGNAL( fetchedPosting( KBlog::BlogPosting& ) ),
           fetchPostingTimer, SLOT( stop() ) );
  b->fetchPosting( QString( "41" ) );
  fetchPostingTimer->start( TIMEOUT );

  connect( b, SIGNAL( modifiedPosting( bool ) ),
           modifyPostingTimer, SLOT( stop() ) );
  b->modifyPosting( p );
  modifyPostingTimer->start( TIMEOUT );

  connect( b, SIGNAL( createdPosting( QString ) ),
           createPostingTimer, SLOT( stop() ) );
  b->createPosting( p );
  createPostingTimer->start( TIMEOUT );

// wait for all jobs to finish

  QTimer::singleShot( GLOBALTIMEOUT, eventLoop, SLOT(quit()));
  eventLoop->exec();

  delete b;
  delete p;
}
