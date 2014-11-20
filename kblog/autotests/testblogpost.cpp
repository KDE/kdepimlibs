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

#include <QtTest>
#include <QtCore>

#include <qtest.h>
#include "kblog/blogpost.h"
#include "kdatetime.h"

Q_DECLARE_METATYPE(KBlog::BlogPost::Status)

using namespace KBlog;

class testBlogPost: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testValidity_data();
};

#include "testblogpost.moc"

void testBlogPost::testValidity_data()
{
    QTest::addColumn<QString>("postId");
    QTest::addColumn<QString>("title");
    QTest::addColumn<QString>("content");
    QTest::addColumn<bool>("isPrivate");
//     QTest::addColumn<QString>( "abbreviatedContent" );
    QTest::addColumn<QUrl>("link");
    QTest::addColumn<QUrl>("permalink");
    QTest::addColumn<bool>("isCommentAllowed");
    QTest::addColumn<bool>("isTrackBackAllowed");
    QTest::addColumn<QString>("summary");
    QTest::addColumn<QStringList>("tags");
//     QTest::addColumn<QList<KUrl> >( "trackBackUrls" );
    QTest::addColumn<QString>("mood");
    QTest::addColumn<QString>("music");
    QTest::addColumn<QStringList>("categories");
    QTest::addColumn<KDateTime>("creationDateTime");
    QTest::addColumn<KDateTime>("modificationDateTime");
    QTest::addColumn<BlogPost::Status>("status");
    QTest::addColumn<QString>("error");

//     QList<KUrl> url;
//     url.append( QUrl("http://track.back.url/some/path") );
    QTest::newRow("SimpleTest")
            << QString::fromLatin1("123ABC")
            << QString::fromLatin1("Title")
            << QString::fromLatin1("Content")
            << true //<< QString("Abbreviated Content")
            << QUrl(QLatin1String("http://my.link/in/outer/space"))
            << QUrl(QLatin1String("http://my.perma/link/space"))
            << true
            << true
            << QString::fromLatin1("Summary")
            << QStringList(QLatin1String("Tags"))   //<< url
            << QString::fromLatin1("Mood") << QString::fromLatin1("Music")
            << QStringList(QLatin1String("Category"))
            << KDateTime(QDateTime::currentDateTime())
            << KDateTime(QDateTime::currentDateTime())
            << BlogPost::New
            << QString::fromLatin1("Error");
}

void testBlogPost::testValidity()
{
    BlogPost p;

    QFETCH(QString, postId);
    QFETCH(QString, title);
    QFETCH(QString, content);
    QFETCH(bool, isPrivate);
//     QFETCH( QString, abbreviatedContent );
    QFETCH(QUrl, link);
    QFETCH(QUrl, permalink);
    QFETCH(bool, isCommentAllowed);
    QFETCH(bool, isTrackBackAllowed);
    QFETCH(QString, summary);
    QFETCH(QStringList, tags);
//     QFETCH( QList<KUrl>, trackBackUrls );
    QFETCH(QString, mood);
    QFETCH(QString, music);
    QFETCH(QStringList, categories);
    QFETCH(KDateTime, creationDateTime);
    QFETCH(KDateTime, modificationDateTime);
    QFETCH(BlogPost::Status, status);
    QFETCH(QString, error);
    p.setPostId(postId);
    p.setTitle(title);
    p.setContent(content);
    p.setPrivate(isPrivate);
//     p.setAbbreviatedContent( abbreviatedContent );
    p.setLink(link);
    p.setPermaLink(permalink);
    p.setCommentAllowed(isCommentAllowed);
    p.setTrackBackAllowed(isTrackBackAllowed);
    p.setSummary(summary);
    p.setTags(tags);
//     p.setTrackBackUrls( trackBackUrls );
    p.setMood(mood);
    p.setMusic(music);
    p.setCategories(categories);
    p.setCreationDateTime(creationDateTime);
    p.setModificationDateTime(modificationDateTime);
    p.setStatus(status);
    p.setError(error);

    QCOMPARE(p.postId(), postId);
    QCOMPARE(p.title(), title);
    QCOMPARE(p.content(), content);
    QCOMPARE(p.isPrivate(), isPrivate);
//     QCOMPARE( p.abbreviatedContent(), abbreviatedContent );
    QCOMPARE(p.link(), link);
    QCOMPARE(p.permaLink(), permalink);
    QCOMPARE(p.isCommentAllowed(), isCommentAllowed);
    QCOMPARE(p.isTrackBackAllowed(), isTrackBackAllowed);
    QCOMPARE(p.summary(), summary);
    QCOMPARE(p.tags(), tags);
//     QCOMPARE( p.trackBackUrls(), trackBackUrls );
    QCOMPARE(p.mood(), mood);
    QCOMPARE(p.music(), music);
    QCOMPARE(p.categories(), categories);
    QCOMPARE(p.creationDateTime(), creationDateTime);
    QCOMPARE(p.modificationDateTime(), modificationDateTime);
    QCOMPARE(p.status(), status);
    QCOMPARE(p.error(), error);
}

QTEST_GUILESS_MAIN(testBlogPost)
