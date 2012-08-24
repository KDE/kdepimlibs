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
#include "kblog/blogmedia.h"
#include "kurl.h"

Q_DECLARE_METATYPE( KBlog::BlogMedia::Status )

using namespace KBlog;

class testBlogMedia: public QObject
{
  Q_OBJECT
  private slots:
    void testValidity();
    void testValidity_data();
};

#include "testblogmedia.moc"

void testBlogMedia::testValidity_data()
{
    QTest::addColumn<QString>( "name" );
    QTest::addColumn<KUrl>( "url" );
    QTest::addColumn<QString>( "mimetype" );
    QTest::addColumn<QByteArray>( "data" );
    QTest::addColumn<BlogMedia::Status>( "status" );
    QTest::addColumn<QString>( "error" );

    QTest::newRow( "SimpleTest" )
      << QString( "FancyMedia" )
      << KUrl( "http://my.link/in/outer/space/fancyMedia.jpg" )
      << QString( "text/xml" )
      << QByteArray( "Tags 1 2" )
      << BlogMedia::New
      << QString( "Error" );
}

void testBlogMedia::testValidity()
{
    BlogMedia p;

    QFETCH( QString, name );
    QFETCH( KUrl, url );
    QFETCH( QString, mimetype );
    QFETCH( QByteArray, data );
    QFETCH( BlogMedia::Status, status );
    QFETCH( QString, error );

    p.setName( name );
    p.setUrl( url );
    p.setMimetype( mimetype );
    p.setData( data );
    p.setStatus( status );
    p.setError( error );

    QCOMPARE( p.name(), name );
    QCOMPARE( p.url(), url );
    QCOMPARE( p.mimetype(), mimetype );
    QCOMPARE( p.data(), data );
    QCOMPARE( p.status(), status );
    QCOMPARE( p.error(), error );

}

QTEST_KDEMAIN_CORE( testBlogMedia )
