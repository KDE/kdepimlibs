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

#include "testmetaweblog.h"
#include "testmetaweblog.moc"

#include <metaweblog.h>

#define TIMEOUT 20000
#define GLOBALTIMEOUT 30000
#define POSTINGID 68
#define DOWNLOADCOUNT 5

using namespace KBlog;

void TestMetaWeblogWarnings::listCategoriesTimeoutWarning()
{
  QWARN( "listCategories() timeout. This can be caused by an error, too." );
}

void TestMetaWeblogWarnings::createMediaTimeoutWarning()
{
  QWARN( "createMedia()  timeout. This can be caused by an error, too." );
}

void TestMetaWeblogWarnings::listPostingsTimeoutWarning()
{
  QWARN( "listPostings() timeout. This can be caused by an error, too." );
}

void TestMetaWeblogWarnings::fetchPostingTimeoutWarning()
{
  QWARN( "fetchPosting() timeout. This can be caused by an error, too." );
}

void TestMetaWeblogWarnings::modifyPostingTimeoutWarning()
{
  QWARN( "modifyPosting() timeout. This can be caused by an error, too." );
}

void TestMetaWeblogWarnings::createPostingTimeoutWarning()
{
  QWARN( "createPosting() timeout. This can be caused by an error, too." );
}

void TestMetaWeblogWarnings::error( const errorType &type,
                                    const QString &errStr )
{
  Q_UNUSED( type );
  QWARN( errStr.toUtf8().data() );
}

QTEST_KDEMAIN( TestMetaWeblog, NoGUI )

void TestMetaWeblog::testValidity()
{
  APIMetaWeblog *b = new APIMetaWeblog(
    KUrl( "http://wrong.url.org/somegateway" ) );
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
  QVERIFY( b->interfaceName() == "MetaWeblog API" );
  QVERIFY( b->timezone().name() == QString( "UTC" ) );
  QVERIFY( b->downloadCount() == DOWNLOADCOUNT );

  BlogPosting *p = new BlogPosting();
  KDateTime mDateTime( QDateTime::currentDateTime() );
  p->setTitle( "TestMetaWeblog" );
  p->setContent( "TestMetaWeblog: posted content." );
  p->setPublish( true );
  p->setPostingId( QString( "41" ) );
  p->setCreationDateTime( mDateTime );
  p->setModificationDateTime( mDateTime );
  QVERIFY( p->title() == "TestMetaWeblog" );
  QVERIFY( p->content() == "TestMetaWeblog: posted content." );
  QVERIFY( p->publish() == true );
  QVERIFY( p->postingId() == QString ( "41" ) );
  QVERIFY( p->creationDateTime() == mDateTime );
  QVERIFY( p->modificationDateTime() == mDateTime );

  BlogMedia *m = new BlogMedia();
  m->setName( "testmetaweblog.txt" );
  m->setMimetype( "text/plain" );
  m->setData( QString( "YTM0NZomIzI2OTsmIzM0NTueYQ==" ).toAscii() );
  QVERIFY( m->mimetype() == "text/plain" );
  QVERIFY( m->data() == QString( "YTM0NZomIzI2OTsmIzM0NTueYQ==" ).toAscii() );
  QVERIFY( m->name() == QString( "testmetaweblog.txt" ) );

  TestMetaWeblogWarnings *warnings = new TestMetaWeblogWarnings();
  connect( b, SIGNAL( error( const errorType&, const QString& ) ),
           warnings, SLOT( error( const errorType&, const QString& ) ) );

  QTimer *listCategoriesTimer = new QTimer( this );
  listCategoriesTimer->setSingleShot( true );
  connect( listCategoriesTimer, SIGNAL( timeout() ),
           warnings, SLOT( listCategoriesTimeoutWarning() ) );

  QTimer *createMediaTimer = new QTimer( this );
  createMediaTimer->setSingleShot( true );
  connect( createMediaTimer, SIGNAL( timeout() ),
           warnings, SLOT( createMediaTimeoutWarning() ) );

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

  connect( b, SIGNAL( listCategoriesFinished() ),
           listCategoriesTimer, SLOT( stop() ) );
  b->listCategories();
  listCategoriesTimer->start( TIMEOUT );

  connect( b, SIGNAL( createdMedia( const QString& ) ),
           createMediaTimer, SLOT( stop() ) );
  b->createMedia( m );
  createMediaTimer->start( TIMEOUT );

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

  QTimer::singleShot( GLOBALTIMEOUT, eventLoop, SLOT( quit() ) );
  eventLoop->exec();

  delete b;
  delete p;
}
