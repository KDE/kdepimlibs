/*
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kmime_content_test.h"
#include <qtest_kde.h>

#include <kmime_content.h>
#include <kmime_headers.h>
#include <kmime_message.h>
using namespace KMime;

QTEST_KDEMAIN( KMimeContentTest, NoGUI )

void KMimeContentTest::testGetHeaderInstance( )
{
  // stuff that looks trivial but breaks if you mess with virtual method signatures (see r534381)
  Headers::From *myfrom = new Headers::From();
  QCOMPARE( myfrom->type(), "From" );
  Headers::Base *mybase = myfrom;
  QCOMPARE( mybase->type(), "From" );

  // getHeaderInstance() is protected, so we need to test it via KMime::Message
  Message *c = new Message();
  Headers::From *f1 = c->from( true );
  Headers::From *f2 = c->from( true );
  QCOMPARE( f1, f2 );
  delete c;
}

void KMimeContentTest::testSetContent()
{
  Content *c = new Content();
  QVERIFY( !c->hasContent() );

  // head and body present
  c->setContent( "head1\nhead2\n\nbody1\n\nbody2\n" );
  QVERIFY( c->hasContent() );
  QCOMPARE( c->head(), QByteArray( "head1\nhead2\n" ) );
  QCOMPARE( c->body(), QByteArray( "body1\n\nbody2\n" ) );

  QList<QByteArray> list;
  list << "head1" << "head2" << "" << "body1" << "" << "body2";
  c->setContent( list );
  QVERIFY( c->hasContent() );
  QCOMPARE( c->head(), QByteArray( "head1\nhead2\n" ) );
  QCOMPARE( c->body(), QByteArray( "body1\n\nbody2\n" ) ); // ### the final \n is questionable

  // empty content
  c->setContent( QByteArray() );
  QVERIFY( !c->hasContent() );
  QVERIFY( c->head().isEmpty() );
  QVERIFY( c->body().isEmpty() );

  // empty head
  c->setContent( "\nbody1\n\nbody2\n" );
  QVERIFY( c->hasContent() );
  QVERIFY( c->head().isEmpty() );
  QCOMPARE( c->body(), QByteArray( "body1\n\nbody2\n" ) );

  list.clear();
  list << "" << "body1" << "" << "body2";
  c->setContent( list );
  QVERIFY( c->hasContent() );
  QVERIFY( c->head().isEmpty() );
  QCOMPARE( c->body(), QByteArray( "body1\n\nbody2\n" ) );

  // empty body
  c->setContent( "head1\nhead2\n\n" );
  QVERIFY( c->hasContent() );
  QCOMPARE( c->head(), QByteArray( "head1\nhead2\n" ) );
  QVERIFY( c->body().isEmpty() );

  list.clear();
  list << "head1" << "head2" << "";
  c->setContent( list );
  QVERIFY( c->hasContent() );
  QCOMPARE( c->head(), QByteArray( "head1\nhead2\n" ) );
  QVERIFY( c->body().isEmpty() );
}

void KMimeContentTest::testMultipleHeaderExtraction()
{
  QByteArray data =
    "From: Nathaniel Borenstein <nsb@bellcore.com>\n"
    "To: Ned Freed <ned@innosoft.com>\n"
    "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
    "Subject: Sample message\n"
    "Received: from ktown.kde.org ([192.168.100.1])\n"
    "Received: from dev1.kde.org ([192.168.100.2])\n"
    "\t by ktown.kde.org ([192.168.100.1])\n"
    "Received: from dev2.kde.org ([192.168.100.3])\n"
    "           by ktown.kde.org ([192.168.100.1])\n";

  Message *msg = new Message();
  msg->setContent( data );
    // FAILS identically to KMimeContentTest::testMultipartMixed
    //  QCOMPARE( msg->encodedContent(), data );
  msg->parse();

  QList<KMime::Headers::Base*> result = msg->headersByType( "Received" );
  QCOMPARE( result.count(), 3 );
  QCOMPARE( result[0]->asUnicodeString(),  QString("from ktown.kde.org ([192.168.100.1])") );
  QCOMPARE( result[1]->asUnicodeString(),  QString("from dev1.kde.org ([192.168.100.2]) by ktown.kde.org ([192.168.100.1])") );
  QCOMPARE( result[2]->asUnicodeString(),  QString("from dev2.kde.org ([192.168.100.3]) by ktown.kde.org ([192.168.100.1])") );
}

void KMimeContentTest::testMultipartMixed()
{
  // example taken from RFC 2046, section 5.1.1.
  QByteArray data =
    "From: Nathaniel Borenstein <nsb@bellcore.com>\n"
    "To: Ned Freed <ned@innosoft.com>\n"
    "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
    "Subject: Sample message\n"
    "MIME-Version: 1.0\n"
    "Content-type: multipart/mixed; boundary=\"simple boundary\"\n"
    "\n"
    "This is the preamble.  It is to be ignored, though it\n"
    "is a handy place for composition agents to include an\n"
    "explanatory note to non-MIME conformant readers.\n"
    "\n"
    "--simple boundary\n"
    "\n"
    "This is implicitly typed plain US-ASCII text.\n"
    "It does NOT end with a linebreak.\n"
    "--simple boundary\n"
    "Content-type: text/plain; charset=us-ascii\n"
    "\n"
    "This is explicitly typed plain US-ASCII text.\n"
    "It DOES end with a linebreak.\n"
    "\n"
    "--simple boundary--\n"
    "\n"
    "This is the epilogue.  It is also to be ignored.\n";

  QByteArray part1 =
    "This is implicitly typed plain US-ASCII text.\n"
    "It does NOT end with a linebreak.";

  QByteArray part2 =
    "This is explicitly typed plain US-ASCII text.\n"
    "It DOES end with a linebreak.\n";

  // slightly diffrent from original data
  QByteArray assembled =
    "From: Nathaniel Borenstein <nsb@bellcore.com>\n"
    "Subject: Sample message\n"
    "To: Ned Freed <ned@innosoft.com>\n"
    "Date: Sun, 21 Mar 1993 23:56:48 -0800\n"
    "MIME-Version: 1.0\n"
    "Content-Type: multipart/mixed; boundary=\"simple boundary\"\n"
    "\n"
    "\n"
    "--simple boundary\n"
    "\n"
    "This is implicitly typed plain US-ASCII text.\n"
    "It does NOT end with a linebreak.\n"
    "--simple boundary\n"
    "Content-Type: text/plain; charset=\"us-ascii\"\n"
    "\n"
    "This is explicitly typed plain US-ASCII text.\n"
    "It DOES end with a linebreak.\n"
    "\n"
    "--simple boundary--\n";

  // test parsing
  Message *msg = new Message();
  msg->setContent( data );
  QCOMPARE( msg->encodedContent(), data );
  msg->parse();
  QVERIFY( msg->contentType()->isMultipart() );

  Content::List list = msg->contents();
  QCOMPARE( list.count(), 2 );
  Content *c = list.takeFirst();
  QCOMPARE( c->body(), part1 );
  c = list.takeFirst();
  QCOMPARE( c->body(), part2 );

  // assemble again
  msg->assemble();
  QCOMPARE( msg->encodedContent(), assembled );
  delete msg;

  // assembling from scratch
  msg = new Message();
  msg->from()->from7BitString( "Nathaniel Borenstein <nsb@bellcore.com>" );
  msg->to()->from7BitString( "Ned Freed <ned@innosoft.com>" );
  msg->subject()->from7BitString( "Sample message" );
  msg->date()->from7BitString( "Sun, 21 Mar 1993 23:56:48 -0800 (PST)" );
  msg->setBody( part1 );
  c = new Content();
  c->setBody( part2 );
  c->contentType()->setMimeType( "text/plain" );
  c->contentType()->setCharset( "us-ascii" );
  msg->addContent( c );
  msg->contentType()->setBoundary( "simple boundary" );

  list = msg->contents();
  QCOMPARE( list.count(), 2 );
  c = list.takeFirst();
  QCOMPARE( c->body(), part1 );
  c = list.takeFirst();
  QCOMPARE( c->body(), part2 );

  msg->assemble();
  QCOMPARE( msg->encodedContent(), assembled );
}

void KMimeContentTest::testImplicitMultipartGeneration()
{
  Content *c1 = new Content();
  c1->contentType()->from7BitString( "text/plain" );
  c1->setBody( "textpart" );

  Content *c2 = new Content();
  c2->contentType()->from7BitString( "text/html" );
  c2->setBody( "htmlpart" );

  c1->addContent( c2 );

  // c1 implicitly converted into a multipart/mixed node
  QCOMPARE( c1->contentType()->mimeType(), QByteArray("multipart/mixed") );
  QVERIFY( c1->body().isEmpty() );

  Content *c = c1->contents().at( 0 ); // former c1
  QCOMPARE( c->contentType()->mimeType(), QByteArray("text/plain") );
  QCOMPARE( c->body(), QByteArray("textpart") );

  QCOMPARE( c1->contents().at( 1 ), c2 );
}

void KMimeContentTest::testExplicitMultipartGeneration()
{
  Content *c1 = new Content();
  c1->contentType()->from7BitString( "multipart/mixed" );

  Content *c2 = new Content();
  c2->contentType()->from7BitString( "text/plain" );
  c2->setBody( "textpart" );

  Content *c3 = new Content();
  c3->contentType()->from7BitString( "text/html" );
  c3->setBody( "htmlpart" );

  c1->addContent( c2 );
  c1->addContent( c3 );

  // c1 should not be changed
  QCOMPARE( c1->contentType()->mimeType(), QByteArray("multipart/mixed") );
  QVERIFY( c1->body().isEmpty() );

  QCOMPARE( c1->contents().at( 0 ), c2 );
  QCOMPARE( c1->contents().at( 1 ), c3 );
}

#include "kmime_content_test.moc"
