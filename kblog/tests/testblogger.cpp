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

#include <qtest_kde.h>

#include "testblogger.h"
#include "testblogger.moc"

QTEST_KDEMAIN( TestBlogger, NoGUI )

#include <kblog/blogger.h>

using namespace KBlog; 

void TestBlogger::testValidity() 
{
  APIBlogger *b = new APIBlogger( KUrl( "http://whiletaker.homeip.net/wordpress/xmlrpc.php" ) );
  b->setUrl( KUrl( "http://whiletaker.homeip.net/wordpress/xmlrpc.php" ) );
  b->setUsername( "admin" );
  b->setPassword( "e9f51d" );
  b->setBlogId( "1" );
  QVERIFY( b->url() == KUrl( "http://whiletaker.homeip.net/wordpress/xmlrpc.php" ) );
  QVERIFY( b->blogId() == "1" );
  QVERIFY( b->username() == "admin" );
  QVERIFY( b->password() == "e9f51d" );

  BlogPosting *p = new BlogPosting();
  p->setTitle( "TestBlogger" );
  p->setContent( "TestBlogger: posted content." );
  p->setCreationDateTime( KDateTime::currentDateTime( KDateTime::Spec() ) );
  p->setPublish( true );
  QVERIFY( p->title() == "TestBlogger" );
  QVERIFY( p->content() == "TestBlogger: posted content." );
  QVERIFY( p->creationDateTime() == KDateTime::currentDateTime( KDateTime::Spec() ) );
  QVERIFY( p->publish() == true );

  delete b;
  delete p;
}
