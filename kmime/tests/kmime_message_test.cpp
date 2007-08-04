/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "kmime_message_test.h"
#include "kmime_message_test.moc"
#include <qtest_kde.h>

#include <kmime/kmime_message.h>

using namespace KMime;

QTEST_KDEMAIN( MessageTest, NoGUI )

void MessageTest::testMainBodyPart()
{
  Message *msg = new Message();
  Message *msg2 = new Message();
  Content *text = new Content();
  text->contentType()->setMimeType( "text/plain" );
  Content *html = new Content();
  html->contentType()->setMimeType( "text/html" );

  // empty message
  QCOMPARE( msg->mainBodyPart(), msg );
  QCOMPARE( msg->mainBodyPart( "text/plain" ), (Content*)0 );

  // non-multipart
  msg->contentType()->setMimeType( "text/html" );

  QCOMPARE( msg->mainBodyPart(), msg );
  QCOMPARE( msg->mainBodyPart( "text/plain" ), (Content*)0 );
  QCOMPARE( msg->mainBodyPart( "text/html" ), msg );

  // multipart/mixed
  msg2->contentType()->setMimeType( "multipart/mixed" );
  msg2->addContent( text );
  msg2->addContent( html );

  QCOMPARE( msg2->mainBodyPart(), text );
  QCOMPARE( msg2->mainBodyPart( "text/plain" ), text );
  QCOMPARE( msg2->mainBodyPart( "text/html" ), (Content*)0 );

  // mulitpart/alternative
  msg->contentType()->setMimeType( "multipart/alternative" );
  msg->addContent( html );
  msg->addContent( text );

  QCOMPARE( msg->mainBodyPart(), html );
  QCOMPARE( msg->mainBodyPart( "text/plain" ), text );
  QCOMPARE( msg->mainBodyPart( "text/html" ), html );

  // mulitpart/alternative inside multipart/mixed
  Message* msg3 = new Message();
  msg3->contentType()->setMimeType( "multipart/mixed" );
  msg3->addContent( msg );
  Content *attach = new Content();
  attach->contentType()->setMimeType( "text/plain" );

  QCOMPARE( msg3->mainBodyPart(), html );
  QCOMPARE( msg3->mainBodyPart( "text/plain" ), text );
  QCOMPARE( msg3->mainBodyPart( "text/html" ), html );
}

void MessageTest::testBrunosMultiAssembleBug()
{
  QByteArray data =
    "From: Sender <sender@test.org>\n"
    "Subject: Sample message\n"
    "To: Receiver <receiver@test.org>\n"
    "Date: Sat, 04 Aug 2007 12:44 +0200\n"
    "MIME-Version: 1.0\n"
    "Content-Type: text/plain\n"
    "X-Foo: bla\n"
    "X-Bla: foo\n"
    "\n"
    "body";

  Message *msg = new Message;
  msg->setContent( data );
  msg->parse();
  msg->assemble();
  QCOMPARE( msg->encodedContent(), data );

  msg->inReplyTo();
  msg->assemble();
  QCOMPARE( msg->encodedContent(), data );

  delete msg;
}

