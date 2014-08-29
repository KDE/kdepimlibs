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

#include "kblog/movabletype.h"
#include "kblog/blogpost.h"
#include "kblog/blogmedia.h"

#include <qtest.h>

#include <unistd.h>
#include <ktimezone.h>
#include <kdatetime.h>
#include <QTimer>

#define TIMEOUT 20000
#define GLOBALTIMEOUT 140000
#define DOWNLOADCOUNT 5

using namespace KBlog;

class TestMovableType : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    // use this functions as a chain to go through network traffic.
    void fetchUserInfo(const QMap<QString, QString> &);
    void listBlogs(const QList<QMap<QString, QString> > &);
    void listCategories(const QList<QMap<QString, QString> > &categories);
    void listRecentPosts(const QList<KBlog::BlogPost> &posts);
    void createPost(KBlog::BlogPost *post);
    void modifyPost(KBlog::BlogPost *post);
    void fetchPost(KBlog::BlogPost *post);
    void removePost(KBlog::BlogPost *post);
    // end chain
    void errorPost(KBlog::Blog::ErrorType type, const QString &errStr, KBlog::BlogPost *);

private Q_SLOTS:
    void testValidity();
    void testNetwork();

private:
    void dumpPost(const KBlog::BlogPost *);
    KBlog::MovableType *b;
    KBlog::BlogPost *p;
    QEventLoop *eventLoop;
    QTimer *fetchUserInfoTimer;
    QTimer *listBlogsTimer;
    QTimer *listRecentPostsTimer;
    QTimer *listCategoriesTimer;
    QTimer *fetchPostTimer;
    QTimer *modifyPostTimer;
    QTimer *createPostTimer;
    QTimer *removePostTimer;
};

class TestMovableTypeWarnings : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void fetchUserInfoTimeoutWarning();
    void listBlogsTimeoutWarning();
    void listRecentPostsTimeoutWarning();
    void listCategoriesTimeoutWarning();
    void fetchPostTimeoutWarning();
    void modifyPostTimeoutWarning();
    void createPostTimeoutWarning();
    void removePostTimeoutWarning();

};

#include "testmovabletype.moc"

void TestMovableType::dumpPost(const BlogPost *post)
{
    qDebug() << "########### post ############";
    qDebug() << "# postId: " << post->postId();
    qDebug() << "# title: " << post->title();
    qDebug() << "# content: " << post->content();
    qDebug() << "# private: " << post->isPrivate();
    qDebug() << "# categories: " << post->categories().join(QLatin1String(" "));
    qDebug() << "# error: " << post->error();
    qDebug() << "# journalId: " << post->journalId();
    qDebug() << "# allowTrackBack: " << post->isTrackBackAllowed();
    qDebug() << "# allowComment: " << post->isCommentAllowed();
    qDebug() << "# summary: " << post->summary();
    qDebug() << "# tags: " << post->tags();
    qDebug() << "# link: " << post->link().url();
    qDebug() << "# permalink: " << post->permaLink().url();
    switch (post->status()) {
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

// the chain starts here

void TestMovableType::fetchUserInfo(const QMap<QString, QString> &userInfo)
{
    fetchUserInfoTimer->stop();
    qDebug() << "########### fetchUserInfo ###########";
    qDebug() << "# nickname: " << userInfo[QLatin1String("nickname")];
    qDebug() << "# userid: "  << userInfo[QLatin1String("userid")];
    qDebug() << "# url: " <<  userInfo[QLatin1String("url")];
    qDebug() << "# email: " <<  userInfo[QLatin1String("email")];
    qDebug() << "# lastname: " << userInfo[QLatin1String("lastname")];
    qDebug() << "# firstname: " <<  userInfo[QLatin1String("firstname")];
    qDebug() << "##############################\n";

    connect(b, SIGNAL(listedBlogs(QList<QMap<QString,QString> >)),
            this, SLOT(listBlogs(QList<QMap<QString,QString> >)));
    b->listBlogs();
    listBlogsTimer->start(TIMEOUT);
}

void TestMovableType::listBlogs(const QList<QMap<QString, QString> > &listedBlogs)
{
    listBlogsTimer->stop();
    qDebug() << "########### listBlogs ###########";
    QList<QMap<QString, QString> >::ConstIterator it = listedBlogs.begin();
    QList<QMap<QString, QString> >::ConstIterator end = listedBlogs.end();
    for (; it != end; ++it) {
        qDebug() << "# " << (*it).keys().first() << ": " << (*it).values().first();
    }
    qDebug() << "###########################\n";

    connect(b, SIGNAL(listedRecentPosts(QList<KBlog::BlogPost>)),
            this, SLOT(listRecentPosts(QList<KBlog::BlogPost>)));
    b->listRecentPosts(DOWNLOADCOUNT);
    listRecentPostsTimer->start(TIMEOUT);
}

void TestMovableType::listRecentPosts(
    const QList<KBlog::BlogPost> &posts)
{
    listRecentPostsTimer->stop();
    qDebug() << "########### listRecentPosts ###########";
    QList<KBlog::BlogPost>::ConstIterator it = posts.begin();
    QList<KBlog::BlogPost>::ConstIterator end = posts.end();
    for (; it != end; ++it) {
        dumpPost(&(*it));
    }
    qDebug() << "#################################\n";

    connect(b, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
            this, SLOT(listCategories(QList<QMap<QString,QString> >)));
    b->listCategories(); // start chain
    listCategoriesTimer->start(TIMEOUT);
}

void TestMovableType::listCategories(
    const QList<QMap<QString, QString> > &categories)
{
    listCategoriesTimer->stop();
    qDebug() << "########### listCategories ###########";
    QList<QMap<QString, QString> >::ConstIterator it = categories.begin();
    QList<QMap<QString, QString> >::ConstIterator end = categories.end();
    for (; it != end; ++it) {
        qDebug() << "# category name: " << (*it)[QLatin1String("name")];
    }
    qDebug() << "###############################\n";

    connect(b, SIGNAL(createdPost(KBlog::BlogPost*)),
            this, SLOT(createPost(KBlog::BlogPost*)));
    b->createPost(p);   // start chain
    createPostTimer->start(TIMEOUT);
}

void TestMovableType::createPost(KBlog::BlogPost *post)
{
    createPostTimer->stop();
    qDebug() << "########### createPost ############";
    dumpPost(post);
    qDebug() << "################################\n";
    QVERIFY(post->status() == BlogPost::Created);

    connect(b, SIGNAL(modifiedPost(KBlog::BlogPost*)),
            this, SLOT(modifyPost(KBlog::BlogPost*)));
    p->setContent(mModifiedContent);
    b->modifyPost(p);
    modifyPostTimer->start(TIMEOUT);
}

void TestMovableType::modifyPost(KBlog::BlogPost *post)
{
    modifyPostTimer->stop();
    qDebug() << "########### modifyPost ############";
    dumpPost(post);
    qDebug() << "################################\n";
    QVERIFY(post->status() == BlogPost::Modified);

    connect(b, SIGNAL(fetchedPost(KBlog::BlogPost*)),
            this, SLOT(fetchPost(KBlog::BlogPost*)));
    p->setContent(QLatin1String("TestMovableType: created content."));
    b->fetchPost(p);
    fetchPostTimer->start(TIMEOUT);
}

void TestMovableType::fetchPost(KBlog::BlogPost *post)
{
    fetchPostTimer->stop();
    qDebug() << "########### fetchPost ############";
    dumpPost(post);
    qDebug() << "###############################\n";
    QVERIFY(post->status() == BlogPost::Fetched);
//   QVERIFY( post->content() == mModifiedContent );

    connect(b, SIGNAL(removedPost(KBlog::BlogPost*)),
            this, SLOT(removePost(KBlog::BlogPost*)));
    b->removePost(p);
    removePostTimer->start(TIMEOUT);
}

void TestMovableType::removePost(KBlog::BlogPost *post)
{
    removePostTimer->stop();
    qDebug() << "########### removePost ###########";
    dumpPost(post);
    qDebug() << "################################\n";
    QVERIFY(post->status() == BlogPost::Removed);
    eventLoop->quit();
}

void TestMovableType::errorPost(KBlog::Blog::ErrorType type, const QString &errStr,
                                KBlog::BlogPost *post)
{
    qDebug() << "############ error #############";
    switch (type) {
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
    if (post != 0) {
        dumpPost(post);
    }
    qDebug() << "#############################\n";
}

// Warnings for Timouts:

void TestMovableTypeWarnings::fetchUserInfoTimeoutWarning()
{
    QWARN("fetchUserInfo() timeout. This can be caused by an error, too. Any following calls will fail.");
}

void TestMovableTypeWarnings::listBlogsTimeoutWarning()
{
    QWARN("listBlogs()  timeout. This can be caused by an error, too. Any following calls will fail.");
}

void TestMovableTypeWarnings::listRecentPostsTimeoutWarning()
{
    QWARN("listRecentPosts() timeout. This can be caused by an error, too. Any following calls will fail.");
}

void TestMovableTypeWarnings::listCategoriesTimeoutWarning()
{
    QWARN("listCategories() timeout. This can be caused by an error, too. Any following calls will fail.");
}

void TestMovableTypeWarnings::fetchPostTimeoutWarning()
{
    QWARN("fetchPost() timeout. This can be caused by an error, too. Any following calls will fail.");
}

void TestMovableTypeWarnings::modifyPostTimeoutWarning()
{
    QWARN("modifyPost() timeout. This can be caused by an error, too. Any following calls will fail.");
}

void TestMovableTypeWarnings::createPostTimeoutWarning()
{
    QWARN("createPost() timeout. This can be caused by an error, too. Any following calls will fail.");
}

void TestMovableTypeWarnings::removePostTimeoutWarning()
{
    QWARN("removePost() timeout. This can be caused by an error, too. Any following calls will fail.");
}

void TestMovableType::testValidity()
{
    eventLoop = new QEventLoop(this);

    // no need to delete later ;-):
    b = new MovableType(QUrl(QLatin1String("http://wrong.url.org/somegateway")));
    QVERIFY(b->url() == QUrl(QLatin1String("http://wrong.url.org/somegateway")));
    KTimeZone mTimeZone(KTimeZone(QLatin1String("UTC")));
    b->setUrl(mUrl);
    b->setUsername(mUsername);
    b->setPassword(mPassword);
    b->setBlogId(mBlogId);
    b->setTimeZone(mTimeZone);
    QVERIFY(b->url() == mUrl);
    QVERIFY(b->blogId() == mBlogId);
    QVERIFY(b->username() == mUsername);
    QVERIFY(b->password() == mPassword);
    QVERIFY(b->interfaceName() == QLatin1String("Movable Type"));
    QVERIFY(b->timeZone().name() == mTimeZone.name());
}

void TestMovableType::testNetwork()
{
    KDateTime mCDateTime(mCreationDateTime);
    KDateTime mMDateTime(mModificationDateTime);
    p = new BlogPost(); // no need to delete later ;-)
    p->setTitle(mTitle);
    p->setContent(mContent);
    p->setPrivate(mPrivate);
    p->setPostId(mPostId);
    p->setCreationDateTime(mCDateTime);
    p->setModificationDateTime(mMDateTime);
    p->setCommentAllowed(mCommentAllowed);
    p->setTrackBackAllowed(mTrackBackAllowed);
    p->setSummary(mSummary);
    p->setTags(mTags);
    p->setCategories(mCategories << QLatin1String("Blogroll"));

    BlogMedia *m = new BlogMedia();
    m->setName(QLatin1String("testMovableType.txt"));
    m->setMimetype(QLatin1String("text/plain"));
    m->setData("YTM0NZomIzI2OTsmIzM0NTueYQ==");
    QVERIFY(m->mimetype() == QLatin1String("text/plain"));
    QVERIFY(m->data() == "YTM0NZomIzI2OTsmIzM0NTueYQ==");
    QVERIFY(m->name() == QLatin1String("testMovableType.txt"));

    connect(b, SIGNAL(errorPost(KBlog::Blog::ErrorType,QString,KBlog::BlogPost*)),
            this, SLOT(errorPost(KBlog::Blog::ErrorType,QString,KBlog::BlogPost*)));

    TestMovableTypeWarnings *warnings = new TestMovableTypeWarnings();

    fetchUserInfoTimer = new QTimer(this);
    fetchUserInfoTimer->setSingleShot(true);
    connect(fetchUserInfoTimer, SIGNAL(timeout()),
            warnings, SLOT(fetchUserInfoTimeoutWarning()));

    listBlogsTimer = new QTimer(this);
    listBlogsTimer->setSingleShot(true);
    connect(listBlogsTimer, SIGNAL(timeout()),
            warnings, SLOT(listBlogsTimeoutWarning()));

    listRecentPostsTimer = new QTimer(this);
    listRecentPostsTimer->setSingleShot(true);
    connect(listRecentPostsTimer, SIGNAL(timeout()),
            warnings, SLOT(listRecentPostsTimeoutWarning()));

    listCategoriesTimer = new QTimer(this);
    listCategoriesTimer->setSingleShot(true);
    connect(listCategoriesTimer, SIGNAL(timeout()),
            warnings, SLOT(listCategoriesTimeoutWarning()));

    fetchPostTimer = new QTimer(this);
    fetchPostTimer->setSingleShot(true);
    connect(fetchPostTimer, SIGNAL(timeout()),
            warnings, SLOT(fetchPostTimeoutWarning()));

    modifyPostTimer = new QTimer(this);
    modifyPostTimer->setSingleShot(true);
    connect(modifyPostTimer, SIGNAL(timeout()),
            warnings, SLOT(modifyPostTimeoutWarning()));

    createPostTimer = new QTimer(this);
    createPostTimer->setSingleShot(true);
    connect(createPostTimer, SIGNAL(timeout()),
            warnings, SLOT(createPostTimeoutWarning()));

    removePostTimer = new QTimer(this);
    removePostTimer->setSingleShot(true);
    connect(removePostTimer, SIGNAL(timeout()),
            warnings, SLOT(removePostTimeoutWarning()));

    // start the chain
    connect(b, SIGNAL(fetchedUserInfo(QMap<QString,QString>)),
            this, SLOT(fetchUserInfo(QMap<QString,QString>)));
    b->fetchUserInfo();
    fetchUserInfoTimer->start(TIMEOUT);

// wait for all jobs to finish

    QTimer::singleShot(GLOBALTIMEOUT, eventLoop, SLOT(quit()));
    eventLoop->exec();
    delete b;
    delete p;
}

QTEST_GUILESS_MAIN(TestMovableType)
