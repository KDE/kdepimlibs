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
    void listBlogs( const QList<QMap<QString,QString> >& );
    void listRecentPosts( const QList<KBlog::BlogPost>& posts );
    void createPost( KBlog::BlogPost *post );
    void createComment( const KBlog::BlogPost *post, const KBlog::BlogComment *comment );
    void removeComment( const KBlog::BlogPost *post, const KBlog::BlogComment *comment );
    void modifyPost( KBlog::BlogPost *post );
    void fetchPost( KBlog::BlogPost *post );
    void removePost( KBlog::BlogPost *post );
    // end chain
    void error( KBlog::Blog::ErrorType type, const QString &errStr, KBlog::BlogPost * );
  private Q_SLOTS:
    void testValidity();
    void testNetwork();
  private:
    void dumpPost( const KBlog::BlogPost * );
    void dumpComment( const KBlog::BlogComment * );
    KBlog::GData *b;
    KBlog::BlogPost *p;
    KBlog::BlogComment *c;
    QEventLoop *eventLoop;
    QTimer *fetchProfileIdTimer;
    QTimer *listBlogsTimer;
    QTimer *listRecentPostsTimer;
    QTimer *fetchPostTimer;
    QTimer *modifyPostTimer;
    QTimer *createPostTimer;
    QTimer *createCommentTimer;
    QTimer *removeCommentTimer;
    QTimer *removePostTimer;
};

class TestGDataWarnings : public QObject
{
  Q_OBJECT
  private Q_SLOTS:
    void fetchProfileIdTimeoutWarning();
    void listBlogsTimeoutWarning();
    void listRecentPostsTimeoutWarning();
    void fetchPostTimeoutWarning();
    void modifyPostTimeoutWarning();
    void createPostTimeoutWarning();
    void removePostTimeoutWarning();
    void createCommentTimeoutWarning();
    void removeCommentTimeoutWarning();
};

#include "testGData.moc"

void TestGData::dumpPost( const BlogPost *post )
{
  qDebug() << "########### post ############";
  qDebug() << "# postId: " << post->postId();
  qDebug() << "# title: " << post->title();
  qDebug() << "# content: " << post->content();
  qDebug() << "# private: " << post->isPrivate();
  qDebug() << "# categories: " << post->categories().join( " " );
  qDebug() << "# error: " << post->error();
  qDebug() << "# journalId: " << post->journalId();
  switch ( post->status() ) {
  case BlogPost::New:
    qDebug() << "# status: New";
    break;
  case BlogPost::Fetched:
    qDebug() << "# status: Fetched";
    break;
  case BlogPost::Created:
    qDebug() << "# status: Created";
    break;
  case BlogPost::Modified:
    qDebug() << "# status: Modified";
    break;
  case BlogPost::Removed:
    qDebug() << "# status: Removed";
    break;
  case BlogPost::Error:
    qDebug() << "# status: Error";
    break;
  };
  qDebug() << "# creationDateTime(UTC): "
           << post->creationDateTime().toUtc().toString();
  qDebug() << "# modificationDateTime(UTC): "
           << post->modificationDateTime().toUtc().toString();
  qDebug() << "###########################";
}

void TestGData::dumpComment( const BlogComment *comment )
{
  qDebug() << "########### comment ############";
  qDebug() << "# commentId: " << comment->commentId();
  qDebug() << "# title: " << comment->title();
  qDebug() << "# content: " << comment->content();
  qDebug() << "# name: " << comment->name();
  qDebug() << "# email: " << comment->email();
  qDebug() << "# url: " << comment->url().url();
  qDebug() << "# error: " << comment->error();
  switch ( comment->status() ) {
  case BlogComment::New:
    qDebug() << "# status: New";
    break;
  case BlogComment::Fetched:
    qDebug() << "# status: Fetched";
    break;
  case BlogComment::Created:
    qDebug() << "# status: Created";
    break;
  case BlogComment::Removed:
    qDebug() << "# status: Removed";
    break;
  case BlogComment::Error:
    qDebug() << "# status: Error";
    break;
  };
  qDebug() << "# creationDateTime(UTC): "
           << comment->creationDateTime().toUtc().toString();
  qDebug() << "# modificationDateTime(UTC): "
           << comment->modificationDateTime().toUtc().toString();
  qDebug() << "###########################";
}

// the chain starts here

void TestGData::fetchProfileId( const QString &pid )
{
  fetchProfileIdTimer->stop();
  qDebug() << "########### fetchProfileId ###########";
  qDebug() << "# profileId: " << pid;
  qDebug() << "##############################\n";

  connect( b, SIGNAL(listedBlogs(QList<QMap<QString,QString> >)),
           this, SLOT(listBlogs(QList<QMap<QString,QString> >)) );
  b->listBlogs();
  listBlogsTimer->start( TIMEOUT );
}

void TestGData::listBlogs( const QList<QMap<QString,QString> >& listedBlogs )
{
  listBlogsTimer->stop();
  qDebug() << "########### listBlogs ###########";
  QList<QMap<QString,QString> >::ConstIterator it = listedBlogs.begin();
  QList<QMap<QString,QString> >::ConstIterator end = listedBlogs.end();
  for ( ; it != end; ++it ) {
    qDebug() << "# " << ( *it ).values().first();
  }
  qDebug() << "###########################\n";

  connect( b, SIGNAL(listedRecentPosts(QList<KBlog::BlogPost>)),
           this, SLOT(listRecentPosts(QList<KBlog::BlogPost>)) );
  b->listRecentPosts( DOWNLOADCOUNT );
  listRecentPostsTimer->start( TIMEOUT );
}

void TestGData::listRecentPosts(
           const QList<KBlog::BlogPost>& posts )
{
  listRecentPostsTimer->stop();
  qDebug() << "########### listRecentPosts ###########";
  QList<KBlog::BlogPost>::ConstIterator it = posts.begin();
  QList<KBlog::BlogPost>::ConstIterator end = posts.end();
  for ( ; it != end; ++it ) {
    dumpPost( &( *it ) );
  }
  qDebug() << "#################################\n";

  connect( b, SIGNAL(createdPost(KBlog::BlogPost*)),
           this, SLOT(createPost(KBlog::BlogPost*)) );
  b->createPost( p ); // start chain
  createPostTimer->start( TIMEOUT );
}

void TestGData::createPost( KBlog::BlogPost *post )
{
  createPostTimer->stop();
  qDebug() << "########### createPost ############";
  dumpPost( post );
  qDebug() << "################################\n";
  QVERIFY( post->status() == BlogPost::Created );

  connect( b, SIGNAL(createdComment(const KBlog::BlogPost*,const KBlog::BlogComment*)),
           this, SLOT(createComment(const KBlog::BlogPost*,const KBlog::BlogComment*)) );
  b->createComment( p, c );
  createCommentTimer->start( TIMEOUT );
}

void TestGData::createComment( const KBlog::BlogPost *post, const KBlog::BlogComment *comment )
{
  createPostTimer->stop();
  qDebug() << "########### createComment ############";
  dumpPost( post );
  dumpComment( comment );
  qDebug() << "################################\n";
  QVERIFY( comment->status() == BlogComment::Created );

  connect( b, SIGNAL(removedComment(const KBlog::BlogPost*,const KBlog::BlogComment*)),
           this, SLOT(removeComment(const KBlog::BlogPost*,const KBlog::BlogComment*)) );
  b->removeComment( p, c );
  removeCommentTimer->start( TIMEOUT );
}

void TestGData::removeComment( const KBlog::BlogPost *post, const KBlog::BlogComment *comment )
{
  createPostTimer->stop();
  qDebug() << "########### removeComment ############";
  dumpPost( post );
  dumpComment( comment );
  qDebug() << "################################\n";
  QVERIFY( comment->status() == BlogComment::Created );

  connect( b, SIGNAL(modifiedPost(KBlog::BlogPost*)),
           this, SLOT(modifyPost(KBlog::BlogPost*)) );
  p->setContent( mModifiedContent );
  b->modifyPost( p );
  modifyPostTimer->start( TIMEOUT );
}

void TestGData::modifyPost( KBlog::BlogPost *post )
{
  modifyPostTimer->stop();
  qDebug() << "########### modifyPost ############";
  dumpPost( post );
  qDebug() << "################################\n";
  QVERIFY( post->status() == BlogPost::Modified );

  connect( b, SIGNAL(fetchedPost(KBlog::BlogPost*)),
           this, SLOT(fetchPost(KBlog::BlogPost*)) );
  p->setContent( "TestGData: created content." );
  b->fetchPost( p );
  fetchPostTimer->start( TIMEOUT );
}

void TestGData::fetchPost( KBlog::BlogPost *post )
{
  fetchPostTimer->stop();
  qDebug() << "########### fetchPost ############";
  dumpPost( post );
  qDebug() << "###############################\n";
  QVERIFY( post->status() == BlogPost::Fetched );
//   QVERIFY( post->content() == mModifiedContent ); // changed by GData

  connect( b, SIGNAL(removedPost(KBlog::BlogPost*)),
           this, SLOT(removePost(KBlog::BlogPost*)) );
  b->removePost( p );
  removePostTimer->start( TIMEOUT );
}

void TestGData::removePost( KBlog::BlogPost *post )
{
  removePostTimer->stop();
  qDebug() << "########### removePost ###########";
  dumpPost( post );
  qDebug() << "################################\n";
  QVERIFY( post->status() == BlogPost::Removed );
  eventLoop->quit();
}

void TestGData::error( KBlog::Blog::ErrorType type, const QString &errStr,
                       KBlog::BlogPost *post )
{
  qDebug() << "############ error #############";
  switch ( type ) {
  case Blog::Atom:
    qDebug() << "type: Atom";
    break;
  case Blog::XmlRpc:
    qDebug() << "type: xmlRpc";
    break;
  case Blog::ParsingError:
    qDebug() << "type: ParsingError";
    break;
  case Blog::AuthenticationError:
    qDebug() << "type: AuthenticationError";
    break;
  case Blog::NotSupported:
    qDebug() << "type: NotSupported";
    break;
  case Blog::Other:
    qDebug() << "type: Other";
    break;
  };
  qDebug() << "error: " << errStr;
  if ( post != 0 ) {
    dumpPost( post );
  }
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

void TestGDataWarnings::listRecentPostsTimeoutWarning()
{
  QWARN( "listRecentPosts() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::fetchPostTimeoutWarning()
{
  QWARN( "fetchPost() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::modifyPostTimeoutWarning()
{
  QWARN( "modifyPost() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::createPostTimeoutWarning()
{
  QWARN( "createPost() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::createCommentTimeoutWarning()
{
  QWARN( "createComment() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::removeCommentTimeoutWarning()
{
  QWARN( "removeComment() timeout. This can be caused by an error, too. Any following calls will fail." );
}

void TestGDataWarnings::removePostTimeoutWarning()
{
  QWARN( "removePost() timeout. This can be caused by an error, too. Any following calls will fail." );
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
  p->setPostId( mPostId );
  p->setCreationDateTime( mCDateTime );
  p->setModificationDateTime( mMDateTime );

  c = new BlogComment(); // no need to delete later ;-)
  c->setTitle( mCommentTitle );
  c->setContent( mCommentContent );
  c->setName( mUsername );
  c->setEmail( mCommentEmail );
  c->setCreationDateTime( mCDateTime );
  c->setModificationDateTime( mMDateTime );

  connect( b, SIGNAL(errorPost(KBlog::Blog::ErrorType,QString,KBlog::BlogPost*)),
           this, SLOT(error(KBlog::Blog::ErrorType,QString,KBlog::BlogPost*)) );

  TestGDataWarnings *warnings = new TestGDataWarnings();

  fetchProfileIdTimer = new QTimer( this );
  fetchProfileIdTimer->setSingleShot( true );
  connect( fetchProfileIdTimer, SIGNAL(timeout()),
           warnings, SLOT(fetchProfileIdTimeoutWarning()) );

  listBlogsTimer = new QTimer( this );
  listBlogsTimer->setSingleShot( true );
  connect( listBlogsTimer, SIGNAL(timeout()),
           warnings, SLOT(listBlogsTimeoutWarning()) );

  listRecentPostsTimer = new QTimer( this );
  listRecentPostsTimer->setSingleShot( true );
  connect( listRecentPostsTimer, SIGNAL(timeout()),
           warnings, SLOT(listRecentPostsTimeoutWarning()) );

  fetchPostTimer = new QTimer( this );
  fetchPostTimer->setSingleShot( true );
  connect( fetchPostTimer, SIGNAL(timeout()),
           warnings, SLOT(fetchPostTimeoutWarning()) );

  modifyPostTimer = new QTimer( this );
  modifyPostTimer->setSingleShot( true );
  connect( modifyPostTimer, SIGNAL(timeout()),
           warnings, SLOT(modifyPostTimeoutWarning()) );

  createPostTimer = new QTimer( this );
  createPostTimer->setSingleShot( true );
  connect( createPostTimer, SIGNAL(timeout()),
           warnings, SLOT(createPostTimeoutWarning()) );

  createCommentTimer = new QTimer( this );
  createCommentTimer->setSingleShot( true );
  connect( createCommentTimer, SIGNAL(timeout()),
           warnings, SLOT(createCommentTimeoutWarning()) );

  removeCommentTimer = new QTimer( this );
  removeCommentTimer->setSingleShot( true );
  connect( removeCommentTimer, SIGNAL(timeout()),
           warnings, SLOT(removeCommentTimeoutWarning()) );

  removePostTimer = new QTimer( this );
  removePostTimer->setSingleShot( true );
  connect( removePostTimer, SIGNAL(timeout()),
           warnings, SLOT(removePostTimeoutWarning()) );

  // start the chain
  connect( b, SIGNAL(fetchedProfileId(QString)),
          this, SLOT(fetchProfileId(QString)) );
  b->fetchProfileId();
  fetchProfileIdTimer->start( TIMEOUT );

// wait for all jobs to finish

  QTimer::singleShot( GLOBALTIMEOUT, eventLoop, SLOT(quit()));
  eventLoop->exec();
  delete b;
  delete p;
  delete c;
}

QTEST_KDEMAIN_CORE( TestGData )
