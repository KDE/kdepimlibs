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
#include "kblog/blogcomment.h"
#include "kurl.h"
#include "kdatetime.h"

Q_DECLARE_METATYPE( KBlog::BlogComment::Status )

using namespace KBlog;

class testBlogComment: public QObject
{
   Q_OBJECT
  private Q_SLOTS:
    void testValidity();
    void testValidity_data();
};

#include "testblogcomment.moc"

void testBlogComment::testValidity_data()
{
    QTest::addColumn<QString>( "commentId" );
    QTest::addColumn<QString>( "title" );
    QTest::addColumn<QString>( "content" );
    QTest::addColumn<QString>( "name" );
    QTest::addColumn<QString>( "email" );
    QTest::addColumn<KUrl>( "url" );
    QTest::addColumn<BlogComment::Status>( "status" );
    QTest::addColumn<QString>( "error" );
    QTest::addColumn<KDateTime>( "creationDateTime" );
    QTest::addColumn<KDateTime>( "modificationDateTime" );

    QTest::newRow( "SimpleTest" )
      << QString( "ABC123" )
      << QString( "Title" )
      << QString( "Content" )
      << QString( "Name" )
      << QString( "E-Mail" )
      << QUrl("http://my.link/in/outer/space/fancy/ABC123")
      << BlogComment::New
      << QString( "Error" )
      << KDateTime( QDateTime::currentDateTime() )
      << KDateTime( QDateTime::currentDateTime() );
}

void testBlogComment::testValidity()
{
    BlogComment p;

    QFETCH( QString, commentId );
    QFETCH( QString, title );
    QFETCH( QString, content );
    QFETCH( QString, name );
    QFETCH( QString, email );
    QFETCH( QUrl, url );
    QFETCH( BlogComment::Status, status );
    QFETCH( QString, error );
    QFETCH( KDateTime, creationDateTime );
    QFETCH( KDateTime, modificationDateTime );

    p.setCommentId( commentId );
    p.setTitle( title );
    p.setContent( content );
    p.setName( name );
    p.setEmail( email );
    p.setUrl( url );
    p.setStatus( status );
    p.setError( error );
    p.setCreationDateTime( creationDateTime );
    p.setModificationDateTime( modificationDateTime );

    QCOMPARE( p.commentId(), commentId );
    QCOMPARE( p.title(), title );
    QCOMPARE( p.content(), content );
    QCOMPARE( p.name(), name );
    QCOMPARE( p.email(), email );
    QCOMPARE( p.url(), url );
    QCOMPARE( p.status(), status );
    QCOMPARE( p.error(), error );
    QCOMPARE( p.creationDateTime(), creationDateTime );
    QCOMPARE( p.modificationDateTime (), modificationDateTime );

}

QTEST_KDEMAIN_CORE( testBlogComment )
