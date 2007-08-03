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

#include "data.h"

#include "kblog/gdata.h"
#include "kblog/blogposting.h"

#include <qtest_kde.h>

#include <unistd.h>
#include <ktimezone.h>
#include <kdatetime.h>

#define TIMEOUT 20000
#define GLOBALTIMEOUT 140000
#define DOWNLOADCOUNT 5

using namespace KBlog;

class TestGData : public QObject
{
  Q_OBJECT

  public Q_SLOTS:
    // use this functions as a chain to go through network traffic.
    void fetchProfileId( const QString & );
    void listBlogs( const QMap<QString,QMap<QString,QString> >& );
    void listRecentPostings( const QList<KBlog::BlogPosting>& postings );
    void createPosting( KBlog::BlogPosting* posting );
    void modifyPosting( KBlog::BlogPosting* posting );
    void fetchPosting( KBlog::BlogPosting* posting );
    void removePosting( KBlog::BlogPosting* posting );
    // end chain
    void error( KBlog::Blog::ErrorType type, const QString &errStr, KBlog::BlogPosting* );
  private Q_SLOTS:
    void testValidity();
    void testNetwork();
  private:
    void dumpPosting( const KBlog::BlogPosting* );
    KBlog::GData *b;
    KBlog::BlogPosting *p;
    QEventLoop *eventLoop;
    QTimer *fetchProfileIdTimer;
    QTimer *listBlogsTimer;
    QTimer *listRecentPostingsTimer;
    QTimer *fetchPostingTimer;
    QTimer *modifyPostingTimer;
    QTimer *createPostingTimer;
    QTimer *removePostingTimer;
};

class TestGDataWarnings : public QObject
{
  Q_OBJECT
  private Q_SLOTS:
    void fetchProfileIdTimeoutWarning();
    void listBlogsTimeoutWarning();
    void listRecentPostingsTimeoutWarning();
    void fetchPostingTimeoutWarning();
    void modifyPostingTimeoutWarning();
    void createPostingTimeoutWarning();
    void removePostingTimeoutWarning();

};

#include "testGData.moc"

void TestGData::dumpPosting( const BlogPosting* posting )
{
  qDebug() << "########### posting ############";
  qDebug() << "# postingId: " << posting->postingId();
  qDebug() << "# title: " << posting->title();
  qDebug() << "# content: " << posting->content();
  qDebug() << "# publish: " << posting->isPublished();
  qDebug() << "# categories: " << posting->categories().join( " " );
  qDebug() << "# error: " << posting->error();
  qDebug() << "# journalId: " << posting->journalId();
  switch ( posting->status() ){
    case BlogPosting::New:
      qDebug() << "# status: New"; break;
    case BlogPosting::Fetched:
      qDebug() << "# status: Fetched"; break;
    case BlogPosting::Created:
      qDebug() << "# status: Created"; break;
    case BlogPosting::Modified:
      qDebug() << "# status: Modified"; break;
    case BlogPosting::Removed:
      qDebug() << "# status: Removed"; break;
    case BlogPosting::Error:
      qDebug() << "# status: Error"; break;
  };
  qDebug() << "# creationDateTime(UTC): " << 
      posting->creationDateTime().toUtc().toString();
  qDebug() << "# modificationDateTime(UTC): " << 
      posting->modificationDateTime().toUtc().toString();
  qDebug() << "###########################";
}

// the chain starts here

void TestGData::fetchProfileId( const QString &pid )
{
  fetchProfileIdTimer->stop();
  qDebug() << "########### fetchProfileId ###########";
  qDebug() << "# profileId: " << pid;
  qDebug() << "##############################\n";

  connect( b, SIGNAL( listedBlogs( const QMap<QString,QMap<QString,QString> >& ) ),
           this, SLOT( listBlogs( const QMap<QString,QMap<QString,QString> >& ) ) );
  b->listBlogs();
  listBlogsTimer->start( TIMEOUT );
}

void TestGData::listBlogs( const QMap<QString,QMap<QString,QString> >& listedBlogs )
{
  listBlogsTimer->stop();
  QList<QString> keys = listedBlogs.keys();
  qDebug() << "########### listBlogs ###########";
  QList<QString>::ConstIterator it = keys.begin();
  QList<QString>::ConstIterator end = keys.end();
  for ( ; it != end; ++it ) {
    qDebug() << "# " << ( *it );
  }
  qDebug() << "###########################\n";

  connect( b, SIGNAL( listedRecentPostings(const QList<KBlog::BlogPosting>&) ),
           this, SLOT( listRecentPostings(const QList<KBlog::BlogPosting>&) ) );
  b->listRecentPostings( DOWNLOADCOUNT );
  listRecentPostingsTimer->start( TIMEOUT );
}

void TestGData::listRecentPostings( 
           const QList<KBlog::BlogPosting>& postings )
{
  listRecentPostingsTimer->stop();
  qDebug() << "########### listRecentPostings ###########";
  QList<KBlog::BlogPosting>::ConstIterator it = postings.begin();
  QList<KBlog::BlogPosting>::ConstIterator end = postings.end();
  for ( ; it != end; ++it ) {
    dumpPosting( &( *it ) );
  }
  qDebug() << "#################################\n";

  connect( b, SIGNAL( createdPosting( KBlog::BlogPosting* ) ),
           this, SLOT( createPosting( KBlog::BlogPosting* ) ) );
  b->createPosting( p ); // start chain
  createPostingTimer->start( TIMEOUT );
}

void TestGData::createPosting( KBlog::BlogPosting *posting )
{
  createPostingTimer->stop();
  qDebug() << "########### createPosting ############";
  dumpPosting( posting );
  qDebug() << "################################\n";
  QVERIFY( posting->status() == BlogPosting::Created );

  connect( b, SIGNAL( modifiedPosting( KBlog::BlogPosting* ) ),
           this, SLOT( modifyPosting( KBlog::BlogPosting* ) ) );
  p->setContent( mModifiedContent );
  b->modifyPosting( p );
  modifyPostingTimer->start( TIMEOUT );
}

void TestGData::modifyPosting( KBlog::BlogPosting *posting )
{
  modifyPostingTimer->stop();
  qDebug() << "########### modifyPosting ############";
  dumpPosting( posting );
  qDebug() << "################################\n";
  QVERIFY( posting->status() == BlogPosting::Modified );

  connect( b, SIGNAL( fetchedPosting( KBlog::BlogPosting* ) ),
           this, SLOT( fetchPosting( KBlog::BlogPosting* ) ) );
  p->setContent( "TestGData: created content." );
  b->fetchPosting( p );
  fetchPostingTimer->start( TIMEOUT );
}

void TestGData::fetchPosting( KBlog::BlogPosting *posting )
{
  fetchPostingTimer->stop();
  qDebug() << "########### fetchPosting ############";
  dumpPosting( posting );
  qDebug() << "###############################\n";
  QVERIFY( posting->status() == BlogPosting::Fetched );
//   QVERIFY( posting->content() == mModifiedContent ); // changed by GData

  connect( b, SIGNAL( removedPosting( KBlog::BlogPosting* ) ),
           this, SLOT( removePosting( KBlog::BlogPosting* ) ) );
  b->removePosting( p );
  removePostingTimer->start( TIMEOUT );
}

void TestGData::removePosting( KBlog::BlogPosting *posting )
{
  removePostingTimer->stop();
  qDebug() << "########### removePosting ###########";
  dumpPosting( posting );
  qDebug() << "################################\n";
  QVERIFY( posting->status() == BlogPosting::Removed );
  eventLoop->quit();
}

void TestGData::error( KBlog::Blog::ErrorType type, const QString &errStr,
        KBlog::BlogPosting* posting )
{
  qDebug() << "############ error #############";
  switch ( type ){
    case Blog::Atom: qDebug() << "type: Atom"; break;
    case Blog::XmlRpc: qDebug() << "type: xmlRpc"; break;
    case Blog::ParsingError: qDebug() << "type: ParsingError"; break;
    case Blog::AuthenticationError: qDebug() << "type: AuthenticationError"; break;
    case Blog::NotSupported: qDebug() << "type: NotSupported"; break;
    case Blog::Other: qDebug() << "type: Other"; break;
  };
  qDebug() << "error: " << errStr;
  if( posting!=0 ) dumpPosting( posting );
  qDebug() << "#############################\n";
}

// Warnings for Timouts:

void TestGDataWarnings::fetchProfileIdTimeoutWarning()
{
  QWARN( "fetchProfileId() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::listBlogsTimeoutWarning()
{
  QWARN( "listBlogs()  timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::listRecentPostingsTimeoutWarning()
{
  QWARN( "listRecentPostings() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::fetchPostingTimeoutWarning()
{
  QWARN( "fetchPosting() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::modifyPostingTimeoutWarning()
{
  QWARN( "modifyPosting() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::createPostingTimeoutWarning()
{
  QWARN( "createPosting() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::removePostingTimeoutWarning()
{
  QWARN( "removePosting() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGData::testValidity()
{
  eventLoop = new QEventLoop( this );

  // we do not test the setUrl() function additionally here
  b = new GData( KUrl( "http://blogger2test.blogspot.com" ) );
  b->setUsername( "christian_weilbach@web.de" );
  b->setFullName( "Santa Claus" );
  b->setProfileId( "11235141638164909615" );
  b->setPassword( "Wo ist Hans?" );
  b->setBlogId( "4662848212819772532" );
  b->setTimeZone( KTimeZone( "UTC" ) );
  QVERIFY( b->url() == KUrl( "http://blogger2test.blogspot.com" ) );
  QVERIFY( b->blogId() == "4662848212819772532" );
  QVERIFY( b->fullName() == "Santa Claus" );
  QVERIFY( b->username() == "christian_weilbach@web.de" );
  QVERIFY( b->profileId() == "11235141638164909615" );
  QVERIFY( b->password() == "Wo ist Hans?" );
  QVERIFY( b->interfaceName() == "Google Blogger Data" );
  QVERIFY( b->timeZone().name() == QString( "UTC" ) );
}

void TestGData::testNetwork()
{
  KDateTime mCDateTime( mCreationDateTime );
  KDateTime mMDateTime( mModificationDateTime );
  p = new BlogPosting(); // no need to delete later ;-)
  p->setTitle( mTitle );
  p->setContent( mContent );
  p->setPublished( mPublished );
  p->setPostingId( mPostingId );
  p->setCreationDateTime( mCDateTime );
  p->setModificationDateTime( mMDateTime );


  connect( b, SIGNAL( error( KBlog::Blog::ErrorType, const QString&, KBlog::BlogPosting* ) ),
           this, SLOT( error( KBlog::Blog::ErrorType, const QString&, KBlog::BlogPosting* ) ) );

  TestGDataWarnings *warnings = new TestGDataWarnings();

  fetchProfileIdTimer = new QTimer( this );
  fetchProfileIdTimer->setSingleShot( true );
  connect( fetchProfileIdTimer, SIGNAL( timeout() ),
           warnings, SLOT( fetchProfileIdTimeoutWarning() ) );

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

  // start the chain
  connect( b, SIGNAL( fetchedProfileId( const QString & ) ),
          this, SLOT( fetchProfileId( const QString & ) ) );
  b->fetchProfileId();
  fetchProfileIdTimer->start( TIMEOUT );

// wait for all jobs to finish

  QTimer::singleShot( GLOBALTIMEOUT, eventLoop, SLOT(quit()));
  eventLoop->exec();
}

QTEST_KDEMAIN_CORE(TestGData)
