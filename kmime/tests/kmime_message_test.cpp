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

void MessageTest::testWillsAndTillsCrash()
{
  QByteArray deadlyMail = "From: censored@yahoogroups.com\n"
      "To: censored@yahoogroups.com\n"
      "Sender: censored@yahoogroups.com\n"
      "MIME-Version: 1.0\n"
      "Date: 29 Jan 2006 23:58:21 -0000\n"
      "Subject: [censored] Birthday Reminder\n"
      "Reply-To: censored@yahoogroups.com\n"
      "Content-Type: multipart/alternative;\n boundary=\"YCalReminder=cNM4SNTGA4Cg1MVLaPpqNF1138579098\"\n"
      "X-Length: 9594\n"
      "X-UID: 6161\n"
      "Status: RO\n"
      "X-Status: OC\n"
      "X-KMail-EncryptionState:\n"
      "X-KMail-SignatureState:\n"
      "X-KMail-MDN-Sent:\n\n";

//   QByteArray deadlyMail;
//   QFile f( "deadlymail" );
//   f.open( QFile::ReadOnly );
//   deadlyMail = f.readAll();

  KMime::Message *msg = new KMime::Message;
  msg->setContent( deadlyMail );
  msg->parse();
  QVERIFY( !msg->date()->isEmpty() );
  QCOMPARE( msg->subject()->as7BitString( false ), QByteArray( "[censored] Birthday Reminder" ) );
  QCOMPARE( msg->from()->mailboxes().count(), 1 );
  QCOMPARE( msg->sender()->mailboxes().count(), 1 );
  QCOMPARE( msg->replyTo()->mailboxes().count(), 1 );
  QCOMPARE( msg->to()->mailboxes().count(), 1 );
  QCOMPARE( msg->cc()->mailboxes().count(), 0 );
  QCOMPARE( msg->bcc()->mailboxes().count(), 0 );
  QCOMPARE( msg->inReplyTo()->identifiers().count(), 0 );
  QCOMPARE( msg->messageID()->identifiers().count(), 0 );
  delete msg;
}

void MessageTest::missingHeadersTest()
{
  // Test that the message body is OK even though some headers are missing
  KMime::Message msg;
  QString body = "Hi Donald, look at those nice pictures I found!\n";
  QString content = "From: georgebush@whitehouse.org\n"
    "To: donaldrumsfeld@whitehouse.org\n"
    "Subject: Cute Kittens\n"
    "\n" + body;
  msg.setContent( content.toAscii() );
  msg.parse();
  msg.assemble();

  QCOMPARE( body, QString::fromAscii( msg.body() ) );

  // Now create a new message, based on the content of the first one.
  // The body of the new message should still be the same.
  // (there was a bug that caused missing mandatory headers to be
  //  added as a empty newline, which caused parts of the header to
  //  leak into the body)
  KMime::Message msg2;
  qDebug() << msg.encodedContent();
  msg2.setContent( msg.encodedContent() );
  msg2.parse();
  msg2.assemble();

  QCOMPARE( body, QString::fromAscii( msg2.body() ) );
}

