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

#include <qtest_kde.h>
#include "kblog/blogposting.h"
#include "kurl.h"
#include "kdatetime.h"


using namespace KBlog;

class testBlogPosting: public QObject
{
    Q_OBJECT
private slots:
    void testValidity();
    void testValidity_data();
};

#include "testblogposting.moc"

void testBlogPosting::testValidity_data()
{
    QTest::addColumn<QString>("postingId");
    QTest::addColumn<QString>("title");
    QTest::addColumn<QString>("content");
    QTest::addColumn<bool>("isPublished");
    QTest::addColumn<QString>("abbreviatedContent");
    QTest::addColumn<KUrl>("link");
    QTest::addColumn<KUrl>("permalink");
    QTest::addColumn<bool>("isCommentAllowed");
    QTest::addColumn<bool>("isTrackBackAllowed");
    QTest::addColumn<QString>("summary");
    QTest::addColumn<QString>("tags");
    QTest::addColumn<QList<KUrl> >("trackBackUrls");
    QTest::addColumn<QString>("mood");
    QTest::addColumn<QString>("music");
    QTest::addColumn<QStringList>("categories");
    QTest::addColumn<KDateTime>("creationDateTime");
    QTest::addColumn<KDateTime>("modificationDateTime");
    QTest::addColumn<BlogPosting::Status>("status");
    QTest::addColumn<QString>("error");

    QList<KUrl> url;
    url.append( KUrl( "http://track.back.url/some/path" ) );
    QStringList categories( "Category" );
    QTest::newRow("SimpleTest") << QString("123ABC") << QString("Title") 
      << QString("Content") << true << QString("Abbreviated Content")
      << KUrl( "http://my.link/in/outer/space" ) << KUrl( "http://my.perma/link/space" )
      << true << true << QString( "Summary" ) << QString( "Tags 1 2" )
      << url  << QString( "Mood" ) << QString( "Music" )
      << categories
      << KDateTime( QDateTime::currentDateTime() )
      << KDateTime( QDateTime::currentDateTime() ) << BlogPosting::New
      << QString( "Error" );
}

void testBlogPosting::testValidity()
{
    BlogPosting p;

    QFETCH(QString, postingId);
    QFETCH(QString, title);
    QFETCH(QString, content);
    QFETCH(bool, isPublished);
    QFETCH(QString, abbreviatedContent);
    QFETCH(KUrl, link);
    QFETCH(KUrl, permalink);
    QFETCH(bool, isCommentAllowed);
    QFETCH(bool, isTrackBackAllowed);
    QFETCH(QString, summary);
    QFETCH(QString, tags);
    QFETCH(QList<KUrl>, trackBackUrls);
    QFETCH(QString, mood);
    QFETCH(QString, music);
    QFETCH(QStringList, categories);
    QFETCH(KDateTime, creationDateTime);
    QFETCH(KDateTime, modificationDateTime);
    QFETCH(BlogPosting::Status, status);
    QFETCH(QString, error);

    p.setPostingId( postingId );
    p.setTitle( title );
    p.setContent( content );
    p.setPublished( isPublished );
    p.setAbbreviatedContent( abbreviatedContent );
    p.setLink( link );
    p.setPermalink( permalink );
    p.setCommentAllowed( isCommentAllowed );
    p.setTrackBackAllowed( isTrackBackAllowed );
    p.setSummary( summary );
    p.setTags( tags );
    p.setTrackBackUrls( trackBackUrls );
    p.setMood( mood );
    p.setMusic( music );
    p.setCategories( categories );
    p.setCreationDateTime( creationDateTime );
    p.setModificationDateTime( modificationDateTime );
    p.setStatus( status );
    p.setError( error );

    QCOMPARE(p.postingId(), postingId );
    QCOMPARE(p.title(), title );
    QCOMPARE(p.content(), content );
    QCOMPARE(p.isPublished(), isPublished );
    QCOMPARE(p.abbreviatedContent(), abbreviatedContent );
    QCOMPARE(p.link(), link );
    QCOMPARE(p.permalink(), permalink );
    QCOMPARE(p.isCommentAllowed(), isCommentAllowed );
    QCOMPARE(p.isTrackBackAllowed(), isTrackBackAllowed );
    QCOMPARE(p.summary(), summary );
    QCOMPARE(p.tags(), tags );
    QCOMPARE(p.trackBackUrls(), trackBackUrls );
    QCOMPARE(p.mood(), mood );
    QCOMPARE(p.music(), music );
    QCOMPARE(p.categories(), categories );
    QCOMPARE(p.creationDateTime(), creationDateTime );
    QCOMPARE(p.modificationDateTime(), modificationDateTime );
    QCOMPARE(p.status(), status );
    QCOMPARE(p.error(), error );
}

QTEST_KDEMAIN_CORE(testBlogPosting)
