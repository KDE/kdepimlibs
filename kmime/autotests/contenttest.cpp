/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#include "contenttest.h"

#include <QDebug>
#include <qtest.h>

#include <kmime_content.h>
#include <kmime_headers.h>
#include <kmime_message.h>
using namespace KMime;

QTEST_MAIN(ContentTest)

void ContentTest::testGetHeaderInstance()
{
    // stuff that looks trivial but breaks if you mess with virtual method signatures (see r534381)
    Headers::From *myfrom = new Headers::From();
    QCOMPARE(myfrom->type(), "From");
    Headers::Base *mybase = myfrom;
    QCOMPARE(mybase->type(), "From");

    // getHeaderInstance() is protected, so we need to test it via KMime::Message
    Message *c = new Message();
    Headers::From *f1 = c->from(true);
    Headers::From *f2 = c->from(true);
    QCOMPARE(f1, f2);
    delete c;
}

void ContentTest::testHeaderAddRemove()
{
    // Add a Content-Description header to a content.
    Content *c = new Content;
    QVERIFY(!c->contentDescription(false));
    c->contentDescription()->from7BitString("description");

    // The content must now have the header.
    QVERIFY(c->contentDescription(false));
    QCOMPARE(c->contentDescription()->as7BitString(false), QByteArray("description"));

    // The content's head must also have the header.  Save the head.
    c->assemble();
    QByteArray head = c->head();

    // Clear the content.  It must now forget the cached header.
    c->clear();
    QVERIFY(c->head().isEmpty());
    QVERIFY(!c->contentDescription(false));

    // Put the head back.  It must now remember the header.
    c->setHead(head);
    QVERIFY(!c->contentDescription(false));
    c->parse();
    QVERIFY(c->contentDescription(false));
    c->contentDescription()->from7BitString("description");

    // Now remove the header explicitly.
    bool ret = c->removeHeader("Content-Description");
    QVERIFY(ret);

    // The content must have forgotten the header now.
    QVERIFY(!c->contentDescription(false));

    // And after assembly the header should stay gone.
    c->assemble();
    QVERIFY(c->head().isEmpty());
    QVERIFY(!c->contentDescription(false));
}

void ContentTest::testHeaderAppendPrepend()
{
    Content *c = new Content;
    QByteArray d1("Resent-From: test1@example.com");
    QByteArray d2("Resent-From: test2@example.com");
    Headers::Generic *h1 = new Headers::Generic("Resent-From", 0, "test1@example.com");
    Headers::Generic *h2 = new Headers::Generic("Resent-From", 0, "test2@example.com");
    c->appendHeader(h1);
    c->appendHeader(h2);
    c->assemble();
    QByteArray head = d1 + '\n' + d2 + '\n';
    QCOMPARE(c->head(), head);

    QByteArray d3("Resent-From: test3@example.com");
    Headers::Generic *h3 = new Headers::Generic("Resent-From", 0, "test3@example.com");
    c->prependHeader(h3);
    c->assemble();
    head.prepend(d3 + '\n');
    QCOMPARE(c->head(), head);
}

void ContentTest::testImplicitMultipartGeneration()
{
    Content *c1 = new Content();
    c1->contentType()->from7BitString("text/plain");
    c1->setBody("textpart");

    Content *c2 = new Content();
    c2->contentType()->from7BitString("text/html");
    c2->setBody("htmlpart");

    c1->addContent(c2);

    // c1 implicitly converted into a multipart/mixed node.
    QVERIFY(c1->contentType(false));
    QCOMPARE(c1->contentType()->mimeType(), QByteArray("multipart/mixed"));
    QVERIFY(c1->body().isEmpty());

    QCOMPARE(c1->contents().count(), 2);
    Content *c = c1->contents().at(0);   // Former c1.
    QVERIFY(c->contentType(false));
    QCOMPARE(c->contentType()->mimeType(), QByteArray("text/plain"));
    QCOMPARE(c->body(), QByteArray("textpart"));

    QCOMPARE(c1->contents().at(1), c2);

    // Now remove c2. c1 should be converted back to a text/plain content.
    c1->removeContent(c2, false);
    QVERIFY(c1->contents().isEmpty());
    QVERIFY(c1->contentType(false));
    QCOMPARE(c1->contentType()->mimeType(), QByteArray("text/plain"));
    QCOMPARE(c1->body(), QByteArray("textpart"));

    // c2 should not have been touched.
    QVERIFY(c2->contents().isEmpty());
    QVERIFY(c2->contentType(false));
    QCOMPARE(c2->contentType()->mimeType(), QByteArray("text/html"));
    QCOMPARE(c2->body(), QByteArray("htmlpart"));

    // Clean up.
    delete c1;
    delete c2;
}

void ContentTest::testExplicitMultipartGeneration()
{
    Content *c1 = new Content();
    c1->contentType()->from7BitString("multipart/mixed");

    Content *c2 = new Content();
    c2->contentType()->from7BitString("text/plain");
    c2->setBody("textpart");

    Content *c3 = new Content();
    c3->contentType()->from7BitString("text/html");
    c3->setBody("htmlpart");

    c1->addContent(c2);
    c1->addContent(c3);

    // c1 should not have been changed.
    QCOMPARE(c1->contentType()->mimeType(), QByteArray("multipart/mixed"));
    QVERIFY(c1->body().isEmpty());

    QCOMPARE(c1->contents().count(), 2);
    QCOMPARE(c1->contents().at(0), c2);
    QCOMPARE(c1->contents().at(1), c3);

    // Removing c3 should turn c1 into a single-part content containing the data of c2.
    c1->removeContent(c3, false);
    QCOMPARE(c1->contentType()->mimeType(), QByteArray("text/plain"));
    QCOMPARE(c1->contents().count(), 0);
    QCOMPARE(c1->body(), QByteArray("textpart"));

    // Clean up.
    delete c1;
    // c2 was deleted when c1 turned itself single-part.
    delete c3;
}

void ContentTest::testSetContent()
{
    Content *c = new Content();
    QVERIFY(!c->hasContent());

    // head and body present
    c->setContent("head1\nhead2\n\nbody1\n\nbody2\n");
    QVERIFY(c->hasContent());
    QCOMPARE(c->head(), QByteArray("head1\nhead2\n"));
    QCOMPARE(c->body(), QByteArray("body1\n\nbody2\n"));

    QList<QByteArray> list;
    list << "head1" << "head2" << "" << "body1" << "" << "body2";
    c->setContent(list);
    QVERIFY(c->hasContent());
    QCOMPARE(c->head(), QByteArray("head1\nhead2\n"));
    QCOMPARE(c->body(), QByteArray("body1\n\nbody2\n"));     // ### the final \n is questionable

    // empty content
    c->setContent(QByteArray());
    QVERIFY(!c->hasContent());
    QVERIFY(c->head().isEmpty());
    QVERIFY(c->body().isEmpty());

    // empty head
    c->setContent("\nbody1\n\nbody2\n");
    QVERIFY(c->hasContent());
    QVERIFY(c->head().isEmpty());
    QCOMPARE(c->body(), QByteArray("body1\n\nbody2\n"));

    list.clear();
    list << "" << "body1" << "" << "body2";
    c->setContent(list);
    QVERIFY(c->hasContent());
    QVERIFY(c->head().isEmpty());
    QCOMPARE(c->body(), QByteArray("body1\n\nbody2\n"));

    // empty body
    c->setContent("head1\nhead2\n\n");
    QVERIFY(c->hasContent());
    QCOMPARE(c->head(), QByteArray("head1\nhead2\n"));
    QVERIFY(c->body().isEmpty());

    list.clear();
    list << "head1" << "head2" << "";
    c->setContent(list);
    QVERIFY(c->hasContent());
    QCOMPARE(c->head(), QByteArray("head1\nhead2\n"));
    QVERIFY(c->body().isEmpty());
}

void ContentTest::testEncodedContent()
{
    // Example taken from RFC 2046, section 5.1.1.
    // Removed "preamble" and "epilogue", which KMime loses.
    QByteArray data =
        "From: Nathaniel Borenstein <nsb@bellcore.com>\n"
        "To: Ned Freed <ned@innosoft.com>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800 (PST)\n"
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "Content-type: multipart/mixed; boundary=\"simple boundary\"\n"
        "\n"
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
        "--simple boundary--\n";

    Message *msg = new Message;
    msg->setContent(data);
    msg->parse();

    // Test that multiple calls do not corrupt anything.
    QByteArray encc = msg->encodedContent();
    //qDebug() << "original data" << data;
    //qDebug() << "encodedContent" << encc;
    QCOMPARE(msg->encodedContent(), data);
    QCOMPARE(msg->encodedContent(), data);
    QCOMPARE(msg->encodedContent(), data);
    delete msg;

    // RFC 2822 3.5: lines are limited to 1000 characters (998 + CRLF)
    // (bug #187345)
    msg = new Message();
    data =
        "Subject:"
        "test test test test test test test test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test test test test test test test test"
        "\n"
        "References: "
        "<test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> "
        "<test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> "
        "<test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> "
        "<test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> "
        "<test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> "
        "<test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> "
        "<test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> "
        "<test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> "
        "<test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> "
        "<test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com> <test1@example.com>"
        "\n\n"
        "body\n";
    msg->setContent(data);
    QByteArray content = msg->encodedContent(true /* use CRLF */);
    QStringList lines = QString::fromLatin1(content).split(QLatin1String("\r\n"));
    foreach (const QString &line, lines) {
        QEXPECT_FAIL("", "KMime does not fold lines longer than 998 characters", Continue);
        QVERIFY(line.length() < 998 && !line.isEmpty() && line != QLatin1String("body"));
        // The test should be (after the expected failure disappears):
        //QVERIFY( line.length() < 998 );
    }
    delete msg;

}

void ContentTest::testDecodedContent()
{
    Content *c = new Content();
    c->setBody('\0');
    QVERIFY(c->decodedContent() == QByteArray());
    c->setBody(QByteArray());
    QVERIFY(c->decodedContent() == QByteArray());
    c->setBody(" ");
    QVERIFY(c->decodedContent() == QByteArray(" "));
}

void ContentTest::testMultipleHeaderExtraction()
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
    msg->setContent(data);
    // FAILS identically to ContentTest::testMultipartMixed
    //  QCOMPARE( msg->encodedContent(), data );
    msg->parse();

    QList<KMime::Headers::Base *> result = msg->headersByType("Received");
    QCOMPARE(result.count(), 3);
    QCOMPARE(result[0]->asUnicodeString(),  QString::fromLatin1("from ktown.kde.org ([192.168.100.1])"));
    QCOMPARE(result[1]->asUnicodeString(),  QString::fromLatin1("from dev1.kde.org ([192.168.100.2]) by ktown.kde.org ([192.168.100.1])"));
    QCOMPARE(result[2]->asUnicodeString(),  QString::fromLatin1("from dev2.kde.org ([192.168.100.3]) by ktown.kde.org ([192.168.100.1])"));
}

void ContentTest::testMultipartMixed()
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

    // What we expect KMime to parse the above data into.
    QByteArray parsedWithPreambleAndEpilogue =
        "From: Nathaniel Borenstein <nsb@bellcore.com>\n"
        "To: Ned Freed <ned@innosoft.com>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800\n"
        "Subject: Sample message\n"
        "MIME-Version: 1.0\n"
        "Content-Type: multipart/mixed; boundary=\"simple boundary\"\n"
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
        "Content-Type: text/plain; charset=\"us-ascii\"\n"
        "\n"
        "This is explicitly typed plain US-ASCII text.\n"
        "It DOES end with a linebreak.\n"
        "\n"
        "--simple boundary--\n"
        "\n"
        "This is the epilogue.  It is also to be ignored.\n";

    // What we expect KMime to assemble the above data into.
    QByteArray assembled =
        "From: Nathaniel Borenstein <nsb@bellcore.com>\n"
        "To: Ned Freed <ned@innosoft.com>\n"
        "Date: Sun, 21 Mar 1993 23:56:48 -0800\n"
        "Subject: Sample message\n"
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
    msg->setContent(data);
    QCOMPARE(msg->encodedContent(), data);
    msg->parse();
    QVERIFY(msg->contentType()->isMultipart());

    Content::List list = msg->contents();
    QCOMPARE(list.count(), 2);
    Content *c = list.takeFirst();
    QCOMPARE(c->body(), part1);
    c = list.takeFirst();
    QCOMPARE(c->body(), part2);

    // assemble again
    msg->assemble();
    //qDebug() << "expected assembled content" << parsedWithPreambleAndEpilogue;
    //qDebug() << "actual new encoded content" << msg->encodedContent();
    QCOMPARE(msg->encodedContent(), parsedWithPreambleAndEpilogue);
    delete msg;

    // assembling from scratch
    // (The headers have to be in the same order, as we compare with the above assembled.)
    msg = new Message();
    msg->from()->from7BitString("Nathaniel Borenstein <nsb@bellcore.com>");
    msg->to()->from7BitString("Ned Freed <ned@innosoft.com>");
    msg->date()->from7BitString("Sun, 21 Mar 1993 23:56:48 -0800 (PST)");
    msg->subject()->from7BitString("Sample message");
    // HACK to make MIME-Version appear before Content-Type, as in the expected message.
    msg->setHeader(new Headers::MIMEVersion(msg, "1.234"));
    msg->setBody(part1);
    c = new Content();
    c->setBody(part2);
    c->contentType()->setMimeType("text/plain");
    c->contentType()->setCharset("us-ascii");
    msg->addContent(c);
    msg->contentType()->setBoundary("simple boundary");

    list = msg->contents();
    QCOMPARE(list.count(), 2);
    c = list.takeFirst();
    QCOMPARE(c->body(), part1);
    c = list.takeFirst();
    QCOMPARE(c->body(), part2);

    msg->assemble();
    QByteArray encc = msg->encodedContent();
    //qDebug() << "expected assembled content" << assembled;
    //qDebug() << "actual encoded content" << encc;
    QCOMPARE(msg->encodedContent(), assembled);
}

void ContentTest::testParsingUuencoded()
{
    const QByteArray body =
        "This is a test message that should appears as a text/plain part\n"
        "once this message is parsed and convert to a MIME tree.\n"
        "\n"
        "\n";

    const QString imageName = QLatin1String("Name of the encoded file (oxygen 22x22 kde.png)");
    const QByteArray imageBase64 =
        "\n"
        "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\n"
        ;

    const QByteArray uuencodedMsg =
        "Path: news.example.net!not-for-mail\n"
        "From: Coin coin <meuh@example.net>\n"
        "Newsgroups: test.kmime.uuencoded\n"
        "Subject: Kmime test\n"
        "Date: Thu, 14 Apr 2005 20:12:47 -0700\n"
        "Message-ID: <xxxxxxxxxxxxxxxxxx@xxxxx.kmime.example.net>\n"
        "X-Newsreader: Forte Agent 2.0/32.640\n"
        "Lines: 1283\n"
        "Organization: Ament\n"
        "Xref: news.example.net test.kmime.uuencoded:4584\n"
        "\n"
        "This is a test message that should appears as a text/plain part\n"
        "once this message is parsed and convert to a MIME tree.\n"
        "\n"
        "begin 644 Name of the encoded file (oxygen 22x22 kde.png)\n"
        "MB5!.1PT*&@H````-24A$4@```!8````6\"`8```#$M&P[````!'-\"250(\"`@(\n"
        "M?`ADB`````EP2%ES```#=@```W8!?=6\"S````!ET15AT4V]F='=A<F4`=W=W\n"
        "M+FEN:W-C87!E+F]R9YON/!H```4X241!5!@9C<%;B%U7&<#Q_[?6VI=SG3.7\n"
        "M-)E)<X665*W1JB\"V%.R3B&\"L^E*DHA63(EY06K`I]<D+*I8B*%CPH=@7H1#,\n"
        "MDU5$\"C[4ID(J49N\"34R<R67.S)QS]MG7M=?GQ-JGOOC[B:KR-A&Q@/!.`A_@\n"
        "M+:^R2WDGW=7R/Z*JR$>^U1FZM2<ZG85'.^EH.781UABL%81`(.!ECM><R/:(\n"
        "MM$,D7>*X2V035`Q5.9UM3S:>&Z__]?36N1],G.QRQ[_QS:6C!YX\\<O@`4>R)\n"
        "M;&#8C]FS/&)EU&=IH<.PEQ\"\",LUJMK*2K9V<S>T9UVZ.V9D62)H.TK#RE4:;\n"
        "M1$1..L\"H&WSY\\)$#K*X-(%0,!ST6!EV&W91>&B-B^=>;.Z@*\"WL'#`<Q-NZ0\n"
        "MI%U&BPML3S,FTXPK5R>8[NIG@$<=8(RXI2@5)MF,8;^'F!B5&$]$HX[9=L')\n"
        "MCW>)G.'I,W/Z>Q;()QF??K_B;,*Y\\S-^,XGPE&\"3/F`,N\\08*:L\"%8=U\"6H2\n"
        "M@D3X8&F\"H6@@31+B.*'T4%3*`\\<L=QT9<G!?E_/7(N)^CVYW@!$#W\"$&#ENQ\n"
        "M5MJ@&!<3)\"*(9;HQ)J];LLHS+QJB*,)92U&W+(<M[GO?`LY%O/#2C'EG$;$1\n"
        "MW?X`L498&UK#VMW&&(NX&(AHL42S\"=]_Y'9.O*ME=GF=[>O;1,[AG$.S\"5_\\\n"
        "MQ\"K.6OYVJ>*5&QT\"!I4(%W<PQL+P=FM8/&2,M:+6T6((*N2M(7+\"A]^]P$^^\n"
        "M?@?65XCP7X\\]?)0D,LPKY>>_VZ$,@::%5H6Z5L1:&766K%GJ6R/625TKK4+E\n"
        "M`[,XY==_N$'9P.E?O,&6ZR`BB`AK*PD@&($B+\\CKFKRNJ6I/6;6(M83E-6O:\n"
        "MN&>-L\\Q+3]6V%%7#=&/,P;TISYZ]PL4JIJ@;0`#AF><N(@*]U''J8[<QSTNF\n"
        "M\\YSQK*`5,-82;&*-MJD(AM!ZQIM3OGMBR)GOW,5'[UGB3W^?4'O0UO\"V%U^O\n"
        "M>?'/F]QRWWL7N;M7LCW-:)J*-C2`T*>/$5LJ*$$]=963.!\"4M@UL9H:FAK8%\n"
        "M5064IA5^=/8FEZ[EW/+MS]])K\\SQ=8GW-2)*1H8QA09V^:9\"M>6A9R[RR=.O\n"
        "M<>'-.7>N6-1;0F,((:\"J&&)<.N+TLY?)2\\^@8WC\\4X?0VA/J$@T!J0G\"L4<&\n"
        "M@[7C-Q975U,;)0QZ(U96]O/@>X0'/KC,X[_<P79Z5%F!:$LRZ&/J@J$MJ)J6\n"
        "MLFJX.IF3^2EU73'='(>F>&W9,+W1!N^UKDI\\F1.:AJB8<>+^10[>9OG>PPNL\n"
        "MRH1A?\\A@M,RPF?/40T.>^L(^6E)TM(KK)C1U2=,4!._9V?AGZU@_Z\\/1>[4J\n"
        "M\"X)K:#HUAWH5L6UXXTK#[U\\N.?7@B+U+$(*B.L(Y(8T\"3WZNSQ._*O%U35W,\n"
        "MJ7U#\\%Z9_\\4[0$/KU6\"IRSE557)NH^6K/[Q$D>YG>>4`YY^_RM-?2Q`G//:S\n"
        "M$G5=/GMOP>5_YV1YH*X*JBJGJAM0KURYTCI`0U/FO;3?&X]WF$ZVH85_^\"6&\n"
        "M@S%-\\$0FYOHX1Q6F+*!MRT]_ZYG/\"Z;9#CN3+>99AK,)6L\\K=CD@A.FE%WQY\n"
        "M[-3RXAK7;ZY3YSEU65`6<[+NA#3I<O+'%<88Z$[Q34-9Y>3YC%FVPW2Z1613\n"
        "M]HSV<77]E3\\\"K:@JLO]#*]W%X\\_W]]YS?[>_)XU=A#,6:P0DH-KB38;7G,CV\n"
        "MB;1+)%WBN$MD$U2$?+[57+OTTJLW7S_SI6I\\X8*H*K>(B`-&P'Y``.'_H[QE\n"
        ";$[@.>-WU'U(#IV=EWM`H`````$E%3D2N0F\"\"\n"
        "`\n"
        "end\n"
        "\n";

    Message *msg = new Message();
    msg->setContent(uuencodedMsg);
    msg->parse();
    Content::List contents = msg->contents();

    // text + image
    QCOMPARE(contents.size(), 2);

    Content *c = 0;

    // Check the first text part
    c = contents.at(0);
    QVERIFY(c->contentType()->isPlainText());
    QCOMPARE(c->body(), body);

    // Check the image part
    c = contents.at(1);
    QVERIFY(!c->contentType()->isText());
    QCOMPARE(c->contentType()->name(), imageName);
    // The uuencoded content as been recoded as base64
    QCOMPARE(c->encodedContent(), imageBase64);

    delete msg;
}

void ContentTest::testParent()
{
    Content *c1 = new Content();
    c1->contentType()->from7BitString("multipart/mixed");

    Content *c2 = new Content();
    c2->contentType()->from7BitString("text/plain");
    c2->setBody("textpart");

    Content *c3 = new Content();
    c3->contentType()->from7BitString("text/html");
    c3->setBody("htmlpart");

    Content *c4 = new Content();
    c4->contentType()->from7BitString("text/html");
    c4->setBody("htmlpart2");

    Content *c5 = new Content();
    c5->contentType()->from7BitString("multipart/mixed");

    //c2 doesn't have a parent yet
    QCOMPARE(c2->parent(), (Content *)(0L));

    c1->addContent(c2);
    c1->addContent(c3);
    c1->addContent(c4);

    // c1 is the parent of those
    QCOMPARE(c2->parent(), c1);
    QCOMPARE(c3->parent(), c1);

    //test removal
    c1->removeContent(c2, false);
    QCOMPARE(c2->parent(), (Content *)(0L));
    QCOMPARE(c1->contents().at(0), c3);

//check if the content is moved correctly to another parent
    c5->addContent(c4);
    QCOMPARE(c4->parent(), c5);
    QCOMPARE(c1->contents().count(), 0);   //yes, it should be 0
    QCOMPARE(c5->contents().at(0), c4);

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

    // test parsing
    Message *msg = new Message();
    msg->setContent(data);
    msg->parse();
    QCOMPARE(msg->parent(), (Content *)(0L));
    QCOMPARE(msg->contents().at(0)->parent(), msg);
    QCOMPARE(msg->contents().at(1)->parent(), msg);
    delete msg;
}

void ContentTest::testFreezing()
{
    // Example taken from RFC 2046, section 5.1.1.
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

    Message *msg = new Message;
    msg->setContent(data);
    msg->setFrozen(true);

    // The data should be untouched before parsing.
    //qDebug() << "original data" << data;
    //qDebug() << "data from message" << msg->encodedContent();
    QCOMPARE(msg->encodedContent(), data);

    // The data should remain untouched after parsing.
    msg->parse();
    QVERIFY(msg->contentType()->isMultipart());
    QCOMPARE(msg->contents().count(), 2);
    QCOMPARE(msg->encodedContent(), data);

    // Calling assemble() should not alter the data.
    msg->assemble();
    QCOMPARE(msg->encodedContent(), data);
}

