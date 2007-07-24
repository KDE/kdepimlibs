/*
  This file is part of the kblog library.

  Copyright (c) 2007 Christian Weilbach <christian_weilbach@web.de>

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

#include "testgdata.h"
#include "testgdata.moc"

#include "kblog/gdata.h"
#include "kblog/blogposting.h"

#include <qtest_kde.h>
#include <ktimezone.h>
#include <kdatetime.h>

#include <unistd.h>

#define TIMEOUT 20000
#define GLOBALTIMEOUT 30000
#define POSTINGID 41
#define DOWNLOADCOUNT 5

using namespace KBlog;

void TestGDataWarnings::userInfoTimeoutWarning()
{
  QWARN( "userInfo() timeout. This can be caused by an error, too." );
}

void TestGDataWarnings::listBlogsTimeoutWarning()
{
  QWARN( "listBlogs()  timeout. This can be caused by an error, too." );
}

void TestGDataWarnings::listPostingsTimeoutWarning()
{
  QWARN( "listPostings() timeout. This can be caused by an error, too." );
}

void TestGDataWarnings::fetchPostingTimeoutWarning()
{
  QWARN( "fetchPosting() timeout. This can be caused by an error, too." );
}

void TestGDataWarnings::modifyPostingTimeoutWarning()
{
  QWARN( "modifyPosting() timeout. This can be caused by an error, too." );
}

void TestGDataWarnings::createPostingTimeoutWarning()
{
  QWARN( "createPosting() timeout. This can be caused by an error, too." );
}

void TestGDataWarnings::error( const errorType &type, const QString &errStr )
{
  Q_UNUSED( type );
  QWARN( errStr.toUtf8().data() );
}

QTEST_KDEMAIN( TestGData, NoGUI )

void TestGData::testValidity()
{
  // we do not test the setUrl() function additionally here
  APIGData *b = new APIGData( KUrl( "http://blogger2test.blogspot.com" ) );
  b->setUsername( "christian_weilbach@web.de" );
  b->setProfileId( "11235141638164909615" );
  b->setPassword( "Wo ist Hans?" );
  b->setBlogId( "4662848212819772532" );
  b->setTimeZone( KTimeZone( "UTC" ) );
  QVERIFY( b->url() == KUrl( "http://blogger2test.blogspot.com" ) );
  QVERIFY( b->blogId() == "4662848212819772532" );
  QVERIFY( b->username() == "christian_weilbach@web.de" );
  QVERIFY( b->profileId() == "11235141638164909615" );
  QVERIFY( b->password() == "Wo ist Hans?" );
  QVERIFY( b->interfaceName() == "GData API" );
  QVERIFY( b->timeZone().name() == QString( "UTC" ) );

  BlogPosting *p = new BlogPosting();
  KDateTime mDateTime( QDateTime::currentDateTime() );
  p->setTitle( "TestGData" );
  p->setContent( "TestGData: posted content." );
  p->setPublished( true );
  p->setPostingId( QString( "41" ) );
  p->setCreationDateTime( mDateTime );
  p->setModificationDateTime( mDateTime );
  QVERIFY( p->title() == "TestGData" );
  QVERIFY( p->content() == "TestGData: posted content." );
  QVERIFY( p->isPublished() == true );
  QVERIFY( p->postingId() == QString ( "41" ) );
  QVERIFY( p->creationDateTime() == mDateTime );
  QVERIFY( p->modificationDateTime() == mDateTime );

  TestGDataWarnings *warnings = new TestGDataWarnings();
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

  connect( b, SIGNAL( listedRecentPostings( const QList<KBlog::BlogPosting> &postings ) ),
           listPostingsTimer, SLOT( stop() ) );
  b->listRecentPostings( 10 );
  listPostingsTimer->start( TIMEOUT );

//   connect( b, SIGNAL( fetchedPosting( KBlog::BlogPosting& ) ),
//            fetchPostingTimer, SLOT( stop() ) );
//   b->fetchPosting( QString( "3250382476119530786" ) );
//   fetchPostingTimer->start( TIMEOUT );
// 
//   connect( b, SIGNAL( modifiedPosting( bool ) ),
//            modifyPostingTimer, SLOT( stop() ) );
//   b->modifyPosting( p );
//   modifyPostingTimer->start( TIMEOUT );

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
