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

#include "testblogger1.h"
#include "testblogger1.moc"

#include "kblog/blogger1.h"
#include "kblog/blogposting.h"

#include <qtest_kde.h>

#include <unistd.h>
#include <ktimezone.h>
#include <kdatetime.h>

#define TIMEOUT 20000
#define GLOBALTIMEOUT 30000
#define POSTINGID 41
#define DOWNLOADCOUNT 5

using namespace KBlog;

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

void TestBlogger1Warnings::error( const errorType &type, const QString &errStr )
{
  Q_UNUSED( type );
  QWARN( errStr.toUtf8().data() );
}

QTEST_KDEMAIN( TestBlogger1, NoGUI )

void TestBlogger1::testValidity()
{
  Blogger1 *b = new Blogger1( KUrl( "http://wrong.url.org/somegateway" ) );
  QVERIFY( b->url() == KUrl( "http://wrong.url.org/somegateway" ) );
  b->setUrl( KUrl( "http://soctest.wordpress.com/xmlrpc.php" ) );
  b->setUsername( "socapitest" );
  b->setPassword( "k0nt4ctbl0g" );
  b->setBlogId( "1" );
  b->setTimeZone( KTimeZone( "UTC" ) );
  QVERIFY( b->url() == KUrl( "http://soctest.wordpress.com/xmlrpc.php" ) );
  QVERIFY( b->blogId() == "1" );
  QVERIFY( b->username() == "socapitest" );
  QVERIFY( b->password() == "k0nt4ctbl0g" );
  QVERIFY( b->interfaceName() == "Blogger 1.0" );
  QVERIFY( b->timeZone().name() == QString( "UTC" ) );

  BlogPosting *p = new BlogPosting();
  KDateTime mDateTime( QDateTime::currentDateTime() );
  p->setTitle( "TestBlogger1" );
  p->setContent( "TestBlogger1: posted content." );
  p->setPublished( true );
  p->setPostingId( QString( "41" ) );
  p->setCreationDateTime( mDateTime );
  p->setModificationDateTime( mDateTime );
  QVERIFY( p->title() == "TestBlogger1" );
  QVERIFY( p->content() == "TestBlogger1: posted content." );
  QVERIFY( p->isPublished() == true );
  QVERIFY( p->postingId() == QString ( "41" ) );
  QVERIFY( p->creationDateTime() == mDateTime );
  QVERIFY( p->modificationDateTime() == mDateTime );

  TestBlogger1Warnings *warnings = new TestBlogger1Warnings();
  connect( b, SIGNAL( error( const errorType&, const QString& ) ),
           warnings, SLOT( error( const errorType&, const QString& ) ) );

  QTimer *fetchUserInfoTimer = new QTimer( this );
  fetchUserInfoTimer->setSingleShot( true );
  connect( fetchUserInfoTimer, SIGNAL( timeout() ),
           warnings, SLOT( fetchUserInfoTimeoutWarning() ) );

  QTimer *listBlogsTimer = new QTimer( this );
  listBlogsTimer->setSingleShot( true );
  connect( listBlogsTimer, SIGNAL( timeout() ),
           warnings, SLOT( listBlogsTimeoutWarning() ) );

  QTimer *listRecentPostingsTimer = new QTimer( this );
  listRecentPostingsTimer->setSingleShot( true );
  connect( listRecentPostingsTimer, SIGNAL( timeout() ),
           warnings, SLOT( listRecentPostingsTimeoutWarning() ) );

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

  connect( b, SIGNAL( fetchUserInfoRetrieved( const QString&, const QString&, const QString& ) ),
          fetchUserInfoTimer, SLOT( stop() ) );
  b->fetchUserInfo();
  fetchUserInfoTimer->start( TIMEOUT );

  connect( b, SIGNAL( blogInfoRetrieved( const QString&, const QString& ) ),
           listBlogsTimer, SLOT( stop() ) );
  b->listBlogs();
  listBlogsTimer->start( TIMEOUT );

  connect( b, SIGNAL( listRecentPostingsFinished() ),
           listRecentPostingsTimer, SLOT( stop() ) );
  b->listRecentPostings( DOWNLOADCOUNT );
  listRecentPostingsTimer->start( TIMEOUT );

  connect( b, SIGNAL( fetchedPosting( KBlog::BlogPosting& ) ),
           fetchPostingTimer, SLOT( stop() ) );
  b->fetchPosting( p );
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
