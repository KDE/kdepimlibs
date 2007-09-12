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
#include "kblog/blogpost.h"
#include "kblog/blogcomment.h"

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
    void listRecentPostings( const QList<KBlog::BlogPost>& postings );
    void createPosting( KBlog::BlogPost* posting );
    void createComment( const KBlog::BlogPost* posting, const KBlog::BlogComment *comment );
    void removeComment( const KBlog::BlogPost* posting, const KBlog::BlogComment *comment );
    void modifyPosting( KBlog::BlogPost* posting );
    void fetchPosting( KBlog::BlogPost* posting );
    void removePosting( KBlog::BlogPost* posting );
    // end chain
    void error( KBlog::Blog::ErrorType type, const QString &errStr, KBlog::BlogPost* );
  private Q_SLOTS:
    void testValidity();
    void testNetwork();
  private:
    void dumpPosting( const KBlog::BlogPost* );
    void dumpComment( const KBlog::BlogComment* );
    KBlog::GData *b;
    KBlog::BlogPost *p;
    KBlog::BlogComment *c;
    QEventLoop *eventLoop;
    QTimer *fetchProfileIdTimer;
    QTimer *listBlogsTimer;
    QTimer *listRecentPostingsTimer;
    QTimer *fetchPostingTimer;
    QTimer *modifyPostingTimer;
    QTimer *createPostingTimer;
    QTimer *createCommentTimer;
    QTimer *removeCommentTimer;
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
    void createCommentTimeoutWarning();
    void removeCommentTimeoutWarning();
};

#include "testGData.moc"

void TestGData::dumpPosting( const BlogPost* posting )
{
  qDebug() << "########### posting ############";
  qDebug() << "# postingId: " << posting->postingId();
  qDebug() << "# title: " << posting->title();
  qDebug() << "# content: " << posting->content();
  qDebug() << "# private: " << posting->isPrivate();
  qDebug() << "# categories: " << posting->categories().join( " " );
  qDebug() << "# error: " << posting->error();
  qDebug() << "# journalId: " << posting->journalId();
  switch ( posting->status() ){
    case BlogPost::New:
      qDebug() << "# status: New"; break;
    case BlogPost::Fetched:
      qDebug() << "# status: Fetched"; break;
    case BlogPost::Created:
      qDebug() << "# status: Created"; break;
    case BlogPost::Modified:
      qDebug() << "# status: Modified"; break;
    case BlogPost::Removed:
      qDebug() << "# status: Removed"; break;
    case BlogPost::Error:
      qDebug() << "# status: Error"; break;
  };
  qDebug() << "# creationDateTime(UTC): " <<
      posting->creationDateTime().toUtc().toString();
  qDebug() << "# modificationDateTime(UTC): " <<
      posting->modificationDateTime().toUtc().toString();
  qDebug() << "###########################";
}

void TestGData::dumpComment( const BlogComment* comment )
{
  qDebug() << "########### comment ############";
  qDebug() << "# commentId: " << comment->commentId();
  qDebug() << "# title: " << comment->title();
  qDebug() << "# content: " << comment->content();
  qDebug() << "# name: " << comment->name();
  qDebug() << "# email: " << comment->email();
  qDebug() << "# url: " << comment->url().url();
  qDebug() << "# error: " << comment->error();
  switch ( comment->status() ){
    case BlogComment::New:
      qDebug() << "# status: New"; break;
    case BlogComment::Fetched:
      qDebug() << "# status: Fetched"; break;
    case BlogComment::Created:
      qDebug() << "# status: Created"; break;
    case BlogComment::Removed:
      qDebug() << "# status: Removed"; break;
    case BlogComment::Error:
      qDebug() << "# status: Error"; break;
  };
  qDebug() << "# creationDateTime(UTC): " <<
      comment->creationDateTime().toUtc().toString();
  qDebug() << "# modificationDateTime(UTC): " <<
      comment->modificationDateTime().toUtc().toString();
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

  connect( b, SIGNAL( listedRecentPostings(const QList<KBlog::BlogPost>&) ),
           this, SLOT( listRecentPostings(const QList<KBlog::BlogPost>&) ) );
  b->listRecentPostings( DOWNLOADCOUNT );
  listRecentPostingsTimer->start( TIMEOUT );
}

void TestGData::listRecentPostings(
           const QList<KBlog::BlogPost>& postings )
{
  listRecentPostingsTimer->stop();
  qDebug() << "########### listRecentPostings ###########";
  QList<KBlog::BlogPost>::ConstIterator it = postings.begin();
  QList<KBlog::BlogPost>::ConstIterator end = postings.end();
  for ( ; it != end; ++it ) {
    dumpPosting( &( *it ) );
  }
  qDebug() << "#################################\n";

  connect( b, SIGNAL( createdPosting( KBlog::BlogPost* ) ),
           this, SLOT( createPosting( KBlog::BlogPost* ) ) );
  b->createPosting( p ); // start chain
  createPostingTimer->start( TIMEOUT );
}

void TestGData::createPosting( KBlog::BlogPost *posting )
{
  createPostingTimer->stop();
  qDebug() << "########### createPosting ############";
  dumpPosting( posting );
  qDebug() << "################################\n";
  QVERIFY( posting->status() == BlogPost::Created );

  connect( b, SIGNAL( createdComment( const KBlog::BlogPost*, const KBlog::BlogComment* ) ),
           this, SLOT( createComment( const KBlog::BlogPost*, const KBlog::BlogComment* ) ) );
  b->createComment( p, c );
  createCommentTimer->start( TIMEOUT );
}


void TestGData::createComment( const KBlog::BlogPost* posting, const KBlog::BlogComment *comment )
{
  createPostingTimer->stop();
  qDebug() << "########### createComment ############";
  dumpPosting( posting );
  dumpComment( comment );
  qDebug() << "################################\n";
  QVERIFY( comment->status() == BlogComment::Created );

  connect( b, SIGNAL( removedComment( const KBlog::BlogPost*, const KBlog::BlogComment * ) ),
           this, SLOT( removeComment( const KBlog::BlogPost*, const KBlog::BlogComment* ) ) );
  b->removeComment( p, c );
  removeCommentTimer->start( TIMEOUT );
}


void TestGData::removeComment( const KBlog::BlogPost* posting, const KBlog::BlogComment *comment )
{
  createPostingTimer->stop();
  qDebug() << "########### removeComment ############";
  dumpPosting( posting );
  dumpComment( comment );
  qDebug() << "################################\n";
  QVERIFY( comment->status() == BlogComment::Created );

  connect( b, SIGNAL( modifiedPosting( KBlog::BlogPost* ) ),
           this, SLOT( modifyPosting( KBlog::BlogPost* ) ) );
  p->setContent( mModifiedContent );
  b->modifyPosting( p );
  modifyPostingTimer->start( TIMEOUT );
}

void TestGData::modifyPosting( KBlog::BlogPost *posting )
{
  modifyPostingTimer->stop();
  qDebug() << "########### modifyPosting ############";
  dumpPosting( posting );
  qDebug() << "################################\n";
  QVERIFY( posting->status() == BlogPost::Modified );

  connect( b, SIGNAL( fetchedPosting( KBlog::BlogPost* ) ),
           this, SLOT( fetchPosting( KBlog::BlogPost* ) ) );
  p->setContent( "TestGData: created content." );
  b->fetchPosting( p );
  fetchPostingTimer->start( TIMEOUT );
}

void TestGData::fetchPosting( KBlog::BlogPost *posting )
{
  fetchPostingTimer->stop();
  qDebug() << "########### fetchPosting ############";
  dumpPosting( posting );
  qDebug() << "###############################\n";
  QVERIFY( posting->status() == BlogPost::Fetched );
//   QVERIFY( posting->content() == mModifiedContent ); // changed by GData

  connect( b, SIGNAL( removedPosting( KBlog::BlogPost* ) ),
           this, SLOT( removePosting( KBlog::BlogPost* ) ) );
  b->removePosting( p );
  removePostingTimer->start( TIMEOUT );
}

void TestGData::removePosting( KBlog::BlogPost *posting )
{
  removePostingTimer->stop();
  qDebug() << "########### removePosting ###########";
  dumpPosting( posting );
  qDebug() << "################################\n";
  QVERIFY( posting->status() == BlogPost::Removed );
  eventLoop->quit();
}

void TestGData::error( KBlog::Blog::ErrorType type, const QString &errStr,
        KBlog::BlogPost* posting )
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

void TestGDataWarnings::createCommentTimeoutWarning()
{
  QWARN( "createComment() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::removeCommentTimeoutWarning()
{
  QWARN( "removeComment() timeout. This can be caused by an error, too. Any following calls will fail." );
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
  p = new BlogPost(); // no need to delete later ;-)
  p->setTitle( mTitle );
  p->setContent( mContent );
  p->setPrivate( mPrivate );
  p->setPostingId( mPostingId );
  p->setCreationDateTime( mCDateTime );
  p->setModificationDateTime( mMDateTime );

  c = new BlogComment(); // no need to delete later ;-)
  c->setTitle( mCommentTitle );
  c->setContent( mCommentContent );
  c->setName( mUsername );
  c->setEmail( mCommentEmail );
  c->setCreationDateTime( mCDateTime );
  c->setModificationDateTime( mMDateTime );

  connect( b, SIGNAL( error( KBlog::Blog::ErrorType, const QString&, KBlog::BlogPost* ) ),
           this, SLOT( error( KBlog::Blog::ErrorType, const QString&, KBlog::BlogPost* ) ) );

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

  createCommentTimer = new QTimer( this );
  createCommentTimer->setSingleShot( true );
  connect( createCommentTimer, SIGNAL( timeout() ),
           warnings, SLOT( createCommentTimeoutWarning() ) );

  removeCommentTimer = new QTimer( this );
  removeCommentTimer->setSingleShot( true );
  connect( removeCommentTimer, SIGNAL( timeout() ),
           warnings, SLOT( removeCommentTimeoutWarning() ) );


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
  delete b;
  delete p;
  delete c;
}

QTEST_KDEMAIN_CORE(TestGData)
