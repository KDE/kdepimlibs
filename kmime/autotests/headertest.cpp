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

#include "headertest.h"

#include <QDebug>
#include <qtest.h>

#include <kmime_headers.h>

using namespace KMime;
using namespace KMime::Headers;
using namespace KMime::Headers::Generics;

// the following test cases are taken from KDE mailinglists, bug reports, RFC 2045,
// RFC 2183 and RFC 2822, Appendix A

QTEST_MAIN(HeaderTest)

void HeaderTest::testIdentHeader()
{
    // empty header
    Headers::Generics::Ident *h = new Headers::Generics::Ident();
    QVERIFY(h->isEmpty());

    // parse single identifier
    h->from7BitString(QByteArray("<1162746587.784559.5038.nullmailer@svn.kde.org>"));
    QCOMPARE(h->identifiers().count(), 1);
    QCOMPARE(h->identifiers().first(), QByteArray("1162746587.784559.5038.nullmailer@svn.kde.org"));
    QCOMPARE(h->asUnicodeString(), QString::fromLatin1("<1162746587.784559.5038.nullmailer@svn.kde.org>"));
    QVERIFY(!h->isEmpty());

    // clearing a header
    h->clear();
    QVERIFY(h->isEmpty());
    QVERIFY(h->identifiers().isEmpty());
    delete h;

    // parse multiple identifiers
    h = new Headers::Generics::Ident();
    h->from7BitString(QByteArray("<1234@local.machine.example> <3456@example.net>"));
    QCOMPARE(h->identifiers().count(), 2);
    QList<QByteArray> ids = h->identifiers();
    QCOMPARE(ids.takeFirst(), QByteArray("1234@local.machine.example"));
    QCOMPARE(ids.first(), QByteArray("3456@example.net"));
    delete h;

    // parse multiple identifiers with folded headers
    h = new Headers::Generics::Ident();
    h->from7BitString(QByteArray("<1234@local.machine.example>\n  <3456@example.net>"));
    QCOMPARE(h->identifiers().count(), 2);
    ids = h->identifiers();
    QCOMPARE(ids.takeFirst(), QByteArray("1234@local.machine.example"));
    QCOMPARE(ids.first(), QByteArray("3456@example.net"));

    // appending of new identifiers (with and without angle-brackets)
    h->appendIdentifier("<abcd.1234@local.machine.tld>");
    h->appendIdentifier("78910@example.net");
    QCOMPARE(h->identifiers().count(), 4);

    // assemble the final header
    QCOMPARE(h->as7BitString(false), QByteArray("<1234@local.machine.example> <3456@example.net> <abcd.1234@local.machine.tld> <78910@example.net>"));
    delete h;

    // parsing of ident with literal domain
    h = new Headers::Generics::Ident();
    const QByteArray ident = QByteArray("<O55F3Y9E5MmKFwBN@[127.0.0.1]>");
    h->appendIdentifier(ident);
    QEXPECT_FAIL("", "Parsing strips square brackets.", Continue);
    QCOMPARE(h->as7BitString(false), QByteArray(ident));
    delete h;
}

void HeaderTest::testAddressListHeader()
{
    // empty header
    Headers::Generics::AddressList *h = new Headers::Generics::AddressList();
    QVERIFY(h->isEmpty());

    // parse single simple address
    h->from7BitString("joe@where.test");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("joe@where.test"));
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(), QString());
    QCOMPARE(h->prettyAddresses().count(), 1);
    QCOMPARE(h->prettyAddresses().first(), QLatin1String("joe@where.test"));

    // clearing a header
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // parsing and re-assembling a single address with display name
    h = new Headers::Generics::AddressList();
    h->from7BitString("Pete <pete@silly.example>");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("pete@silly.example"));
    QCOMPARE(h->displayNames().first(), QLatin1String("Pete"));
    QCOMPARE(h->prettyAddresses().first(), QLatin1String("Pete <pete@silly.example>"));
    QCOMPARE(h->as7BitString(false), QByteArray("Pete <pete@silly.example>"));
    delete h;

    // parsing a single address with legacy comment style display name
    h = new Headers::Generics::AddressList();
    h->from7BitString("jdoe@machine.example (John Doe)");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("jdoe@machine.example"));
    QCOMPARE(h->displayNames().first(), QLatin1String("John Doe"));
    QCOMPARE(h->prettyAddresses().first(), QLatin1String("John Doe <jdoe@machine.example>"));
    delete h;

    // parsing and re-assembling list of diffrent addresses
    h = new Headers::Generics::AddressList();
    h->from7BitString("Mary Smith <mary@x.test>, jdoe@example.org, Who? <one@y.test>");
    QCOMPARE(h->addresses().count(), 3);
    QStringList names = h->displayNames();
    QCOMPARE(names.takeFirst(), QLatin1String("Mary Smith"));
    QCOMPARE(names.takeFirst(), QString());
    QCOMPARE(names.takeFirst(), QLatin1String("Who?"));
    QCOMPARE(h->as7BitString(false), QByteArray("Mary Smith <mary@x.test>, jdoe@example.org, Who? <one@y.test>"));
    delete h;

    // same again with some interessting quoting
    h = new Headers::Generics::AddressList();
    h->from7BitString("\"Joe Q. Public\" <john.q.public@example.com>, <boss@nil.test>, \"Giant; \\\"Big\\\" Box\" <sysservices@example.net>");
    QCOMPARE(h->addresses().count(), 3);
    names = h->displayNames();
    QCOMPARE(names.takeFirst(), QLatin1String("Joe Q. Public"));
    QCOMPARE(names.takeFirst(), QString());
    QCOMPARE(names.takeFirst(), QLatin1String("Giant; \"Big\" Box"));
    QCOMPARE(h->as7BitString(false), QByteArray("\"Joe Q. Public\" <john.q.public@example.com>, boss@nil.test, \"Giant; \\\"Big\\\" Box\" <sysservices@example.net>"));
    delete h;

    // a display name with non-latin1 content
    h = new Headers::Generics::AddressList();
    h->from7BitString("Ingo =?iso-8859-15?q?Kl=F6cker?= <kloecker@kde.org>");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("kloecker@kde.org"));
    QCOMPARE(h->displayNames().first(), QString::fromUtf8("Ingo Klöcker"));
    QCOMPARE(h->asUnicodeString(), QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"));
    QCOMPARE(h->as7BitString(false), QByteArray("Ingo =?ISO-8859-1?Q?Kl=F6cker?= <kloecker@kde.org>"));
    delete h;

    // a display name with non-latin1 content in both name components
    h = new Headers::Generics::AddressList();
    const QString testAddress = QString::fromUtf8("Ingö Klöcker <kloecker@kde.org>");
    h->fromUnicodeString(testAddress, "utf-8");
    QCOMPARE(h->asUnicodeString(), testAddress);
    delete h;

    {
        // a display name with non-latin1 content in both name components
        h = new Headers::Generics::AddressList();
        const QString testAddress = QString::fromUtf8("\"Rüedi-Huser, Thomas\" <test@test.org>");
        h->fromUnicodeString(testAddress, "utf-8");
        QEXPECT_FAIL("", "AddressList::prettyAddresses() does not quote the mailbox correctly", Continue);
        QCOMPARE(h->asUnicodeString(), testAddress);
        delete h;
    }

    // again, this time legacy style
    h = new Headers::Generics::AddressList();
    h->from7BitString("kloecker@kde.org (Ingo =?iso-8859-15?q?Kl=F6cker?=)");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("kloecker@kde.org"));
    QCOMPARE(h->displayNames().first(), QString::fromUtf8("Ingo Klöcker"));
    delete h;

    // parsing a empty group
    h = new Headers::Generics::AddressList();
    h->from7BitString("Undisclosed recipients:;");
    QCOMPARE(h->addresses().count(), 0);
    delete h;

    // parsing and re-assembling a address list with a group
    h = new Headers::Generics::AddressList();
    h->from7BitString("A Group:Chris Jones <c@a.test>,joe@where.test,John <jdoe@one.test>;");
    QCOMPARE(h->addresses().count(), 3);
    names = h->displayNames();
    QCOMPARE(names.takeFirst(), QLatin1String("Chris Jones"));
    QCOMPARE(names.takeFirst(), QString());
    QCOMPARE(names.takeFirst(), QLatin1String("John"));
    QCOMPARE(h->as7BitString(false), QByteArray("Chris Jones <c@a.test>, joe@where.test, John <jdoe@one.test>"));
    delete h;

    // modifying a header
    h = new Headers::Generics::AddressList();
    h->from7BitString("John <jdoe@one.test>");
    h->addAddress("<kloecker@kde.org>", QString::fromUtf8("Ingo Klöcker"));
    h->addAddress("c@a.test");
    QCOMPARE(h->addresses().count(), 3);
    QCOMPARE(h->asUnicodeString(), QString::fromUtf8("John <jdoe@one.test>, Ingo Klöcker <kloecker@kde.org>, c@a.test"));
    QCOMPARE(h->as7BitString(false), QByteArray("John <jdoe@one.test>, Ingo =?ISO-8859-1?Q?Kl=F6cker?= <kloecker@kde.org>, c@a.test"));
    delete h;

    // parsing from utf-8
    h = new Headers::Generics::AddressList();
    h->fromUnicodeString(QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"), "utf-8");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("kloecker@kde.org"));
    QCOMPARE(h->displayNames().first(), QString::fromUtf8("Ingo Klöcker"));
    delete h;

    // based on bug #137033, a header broken in various ways: ';' as list separator,
    // unquoted '.' in display name
    h = new Headers::Generics::AddressList();
    h->from7BitString("Vice@censored.serverkompetenz.net,\n    President@mail2.censored.net;\"Int\\\\\\\\\\\\\\\\\\\\'l\" Lotto Commission. <censored@yahoo.fr>");
    QCOMPARE(h->addresses().count(), 3);
    names = h->displayNames();
    QCOMPARE(names.takeFirst(), QString());
    QCOMPARE(names.takeFirst(), QString());
    // there is an wrong ' ' after the name, but since the header is completely
    // broken we can be happy it parses at all...
    QCOMPARE(names.takeFirst(), QLatin1String("Int\\\\\\\\\\'l Lotto Commission. "));
    QList<QByteArray> addrs = h->addresses();
    QCOMPARE(addrs.takeFirst(), QByteArray("Vice@censored.serverkompetenz.net"));
    QCOMPARE(addrs.takeFirst(), QByteArray("President@mail2.censored.net"));
    QCOMPARE(addrs.takeFirst(), QByteArray("censored@yahoo.fr"));
    delete h;

    // based on bug #102010, a display name containing '<'
    h = new Headers::Generics::AddressList(0, QByteArray("\"|<onrad\" <censored@censored.dy>"));
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("censored@censored.dy"));
    QCOMPARE(h->displayNames().first(), QLatin1String("|<onrad"));
    QCOMPARE(h->as7BitString(false), QByteArray("\"|<onrad\" <censored@censored.dy>"));

    // based on bug #93790 (legacy display name with nested comments)
    h = new Headers::Generics::AddressList(0, QByteArray("first.name@domain.tld (first name (nickname))"));
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(), QLatin1String("first name (nickname)"));
    QCOMPARE(h->as7BitString(false), QByteArray("\"first name (nickname)\" <first.name@domain.tld>"));
    delete h;

    // rfc 2047 encoding in quoted name (it is not allowed there as per the RFC, but it happens)
    // some software == current KMail (v1.12.90) ...
    h = new Headers::Generics::AddressList();
    h->from7BitString(QByteArray("\"Ingo =?iso-8859-15?q?Kl=F6cker?=\" <kloecker@kde.org>"));
    QCOMPARE(h->mailboxes().count(), 1);
    QCOMPARE(h->asUnicodeString(), QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"));
    delete h;

    // corner case of almost-rfc2047 encoded string in quoted string but not
    h = new Headers::Generics::AddressList(0, QByteArray("\"Some =Use ?r\" <user@example.com>"));
    QCOMPARE(h->mailboxes().count(), 1);
    QCOMPARE(h->as7BitString(false), QByteArray("\"Some =Use ?r\" <user@example.com>"));
    delete h;

    // corner case of almost-rfc2047 encoded string in quoted string but not
    h = new Headers::Generics::AddressList(0, QByteArray("\"Some ?=U=?se =?r\" <user@example.com>"));
    QCOMPARE(h->mailboxes().count(), 1);
    QCOMPARE(h->as7BitString(false), QByteArray("\"Some ?=U=?se =?r\" <user@example.com>"));
    delete h;

    // based on bug #139477, trailing '.' in domain name (RFC 3696, section 2 - http://tools.ietf.org/html/rfc3696#page-4)
    h = new Headers::Generics::AddressList();
    h->from7BitString("joe@where.test.");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("joe@where.test."));
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(), QString());
    QCOMPARE(h->prettyAddresses().count(), 1);
    QCOMPARE(h->prettyAddresses().first(), QLatin1String("joe@where.test."));
    delete h;

    h = new Headers::Generics::AddressList();
    h->from7BitString("Mary Smith <mary@x.test>, jdoe@example.org., Who? <one@y.test>");
    QCOMPARE(h->addresses().count(), 3);
    names = h->displayNames();
    QCOMPARE(names.takeFirst(), QLatin1String("Mary Smith"));
    QCOMPARE(names.takeFirst(), QString());
    QCOMPARE(names.takeFirst(), QLatin1String("Who?"));
    QCOMPARE(h->as7BitString(false), QByteArray("Mary Smith <mary@x.test>, jdoe@example.org., Who? <one@y.test>"));
    delete h;
}

void HeaderTest::testMailboxListHeader()
{
    // empty header
    Headers::Generics::MailboxList *h = new Headers::Generics::MailboxList();
    QVERIFY(h->isEmpty());

    // parse single simple address
    h->from7BitString("joe_smith@where.test");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mailboxes().count(), 1);
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("joe_smith@where.test"));
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(), QString());
    QCOMPARE(h->prettyAddresses().count(), 1);
    QCOMPARE(h->prettyAddresses().first(), QLatin1String("joe_smith@where.test"));

    // https://bugzilla.novell.com/show_bug.cgi?id=421057 (but apparently this was not the cause of the bug)
    h->from7BitString("fr...@ce.sco (Francesco)");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mailboxes().count(), 1);
    QCOMPARE(h->prettyAddresses().first(), QLatin1String("Francesco <fr...@ce.sco>"));

    delete h;
}

void HeaderTest::testSingleMailboxHeader()
{
    // empty header
    Headers::Generics::SingleMailbox *h = new Headers::Generics::SingleMailbox();
    QVERIFY(h->isEmpty());

    // parse single simple address
    h->from7BitString("joe_smith@where.test");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("joe_smith@where.test"));
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(), QString());
    QCOMPARE(h->prettyAddresses().count(), 1);
    QCOMPARE(h->prettyAddresses().first(), QLatin1String("joe_smith@where.test"));

    // parse single simple address with display name
    h->from7BitString("John Smith <joe_smith@where.test>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("joe_smith@where.test"));
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(), QLatin1String("John Smith"));
    QCOMPARE(h->prettyAddresses().count(), 1);
    QCOMPARE(h->prettyAddresses().first(), QLatin1String("John Smith <joe_smith@where.test>"));
    QCOMPARE(h->mailboxes().first().prettyAddress(Types::Mailbox::QuoteAlways),
             QLatin1String("\"John Smith\" <joe_smith@where.test>"));

    // parse quoted display name with \ in it
    h->from7BitString("\"Lastname\\, Firstname\" <firstname.lastname@example.com>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("firstname.lastname@example.com"));
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(), QLatin1String("Lastname, Firstname"));
    QCOMPARE(h->asUnicodeString().toLatin1().data(),
             "Lastname, Firstname <firstname.lastname@example.com>");
    QCOMPARE(h->mailboxes().first().prettyAddress().toLatin1().data(),
             "Lastname, Firstname <firstname.lastname@example.com>");
    QCOMPARE(h->mailboxes().first().prettyAddress(Types::Mailbox::QuoteWhenNecessary).toLatin1().data(),
             "\"Lastname, Firstname\" <firstname.lastname@example.com>");

    // parse quoted display name with " in it
    h->from7BitString("\"John \\\"the guru\\\" Smith\" <john.smith@mail.domain>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first().data(), "john.smith@mail.domain");
    QCOMPARE(h->displayNames().first().toLatin1().data(), "John \"the guru\" Smith");
    QCOMPARE(h->mailboxes().first().prettyAddress(Types::Mailbox::QuoteWhenNecessary).toLatin1().data(),
             "\"John \\\"the guru\\\" Smith\" <john.smith@mail.domain>");
    QCOMPARE(h->as7BitString(false).data(),
             "\"John \\\"the guru\\\" Smith\" <john.smith@mail.domain>");

    // The following tests are for broken clients that by accident add quotes inside of encoded words that enclose the
    // display name. We strip away those quotes, which is not strictly correct, but much nicer.
    h->from7BitString("=?iso-8859-1?Q?=22Andre_Woebbeking=22?= <woebbeking@example.com>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->mailboxes().first().name().toLatin1().data(), "Andre Woebbeking");
    h->from7BitString("=?iso-8859-1?Q?=22Andre_=22Mr._Tall=22_Woebbeking=22?= <woebbeking@example.com>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->mailboxes().first().name().toLatin1().data(), "Andre \"Mr. Tall\" Woebbeking");
    h->from7BitString("=?iso-8859-1?Q?=22Andre_=22?= =?iso-8859-1?Q?Mr._Tall?= =?iso-8859-1?Q?=22_Woebbeking=22?= <woebbeking@example.com>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->mailboxes().first().name().toLatin1().data(), "Andre \"Mr. Tall\" Woebbeking");

    delete h;
}

void HeaderTest::testMailCopiesToHeader()
{
    Headers::MailCopiesTo *h;

    // empty header
    h = new Headers::MailCopiesTo();
    QVERIFY(h->isEmpty());
    QVERIFY(!h->alwaysCopy());
    QVERIFY(!h->neverCopy());

    // set to always copy to poster
    h->setAlwaysCopy();
    QVERIFY(!h->isEmpty());
    QVERIFY(h->alwaysCopy());
    QVERIFY(!h->neverCopy());
    QCOMPARE(h->as7BitString(), QByteArray("Mail-Copies-To: poster"));

    // set to never copy
    h->setNeverCopy();
    QVERIFY(!h->isEmpty());
    QVERIFY(!h->alwaysCopy());
    QVERIFY(h->neverCopy());
    QCOMPARE(h->as7BitString(), QByteArray("Mail-Copies-To: nobody"));

    // clear header
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // parse copy to poster
    h = new MailCopiesTo(0, "always");
    QVERIFY(h->addresses().isEmpty());
    QVERIFY(!h->isEmpty());
    QVERIFY(h->alwaysCopy());
    delete h;

    h = new MailCopiesTo(0, "poster");
    QVERIFY(h->addresses().isEmpty());
    QVERIFY(!h->isEmpty());
    QVERIFY(h->alwaysCopy());
    delete h;

    // parse never copy
    h = new MailCopiesTo(0, "never");
    QVERIFY(h->addresses().isEmpty());
    QVERIFY(!h->isEmpty());
    QVERIFY(h->neverCopy());
    delete h;

    h = new MailCopiesTo(0, "nobody");
    QVERIFY(h->addresses().isEmpty());
    QVERIFY(!h->isEmpty());
    QVERIFY(h->neverCopy());
    delete h;

    // parsing is case-insensitive
    h = new MailCopiesTo(0, "AlWays");
    QVERIFY(h->alwaysCopy());
    delete h;

    // parse address
    h = new MailCopiesTo(0, "vkrause@kde.org");
    QVERIFY(!h->addresses().isEmpty());
    QVERIFY(h->alwaysCopy());
    QVERIFY(!h->neverCopy());
    QCOMPARE(h->as7BitString(), QByteArray("Mail-Copies-To: vkrause@kde.org"));
    delete h;
}

void HeaderTest::testParametrizedHeader()
{
    Parametrized *h;

    // empty header
    h = new Parametrized();
    QVERIFY(h->isEmpty());
    QVERIFY(!h->hasParameter(QLatin1String("foo")));

    // add a parameter
    h->setParameter(QLatin1String("filename"), QLatin1String("bla.jpg"));
    QVERIFY(!h->isEmpty());
    QVERIFY(h->hasParameter(QLatin1String("filename")));
    QVERIFY(h->hasParameter(QLatin1String("FiLeNaMe")));
    QVERIFY(!h->hasParameter(QLatin1String("bla.jpg")));
    QCOMPARE(h->parameter(QLatin1String("filename")), QLatin1String("bla.jpg"));
    QCOMPARE(h->as7BitString(false), QByteArray("filename=\"bla.jpg\""));

    // clear again
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // parse a parameter list
    h = new Parametrized(0, "filename=genome.jpeg;\n modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"");
    QCOMPARE(h->parameter(QLatin1String("filename")), QLatin1String("genome.jpeg"));
    QCOMPARE(h->parameter(QLatin1String("modification-date")), QLatin1String("Wed, 12 Feb 1997 16:29:51 -0500"));
    QCOMPARE(h->as7BitString(false), QByteArray("filename=\"genome.jpeg\"; modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\""));
    delete h;

    // quoting of whitespaces in parameter value
    h = new Parametrized();
    h->setParameter(QLatin1String("boundary"), QLatin1String("simple boundary"));
    QCOMPARE(h->as7BitString(false), QByteArray("boundary=\"simple boundary\""));
    delete h;

    // TODO: test RFC 2047 encoded values
    // TODO: test case-insensitive key-names
}

void HeaderTest::testContentDispositionHeader()
{
    ContentDisposition *h;

    // empty header
    h = new ContentDisposition();
    QVERIFY(h->isEmpty());

    // set some values
    h->setFilename(QLatin1String("test.jpg"));
    QVERIFY(h->isEmpty());
    QVERIFY(h->as7BitString(false).isEmpty());
    h->setDisposition(CDattachment);
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(false), QByteArray("attachment; filename=\"test.jpg\""));
    delete h;

    // parse parameter-less header
    h = new ContentDisposition(0, "inline");
    QCOMPARE(h->disposition(), CDinline);
    QVERIFY(h->filename().isEmpty());
    QCOMPARE(h->as7BitString(true), QByteArray("Content-Disposition: inline"));
    delete h;

    // parse header with parameter
    h = new ContentDisposition(0, "attachment; filename=genome.jpeg;\n modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\";");
    QCOMPARE(h->disposition(), CDattachment);
    QCOMPARE(h->filename(), QLatin1String("genome.jpeg"));
    delete h;

    // TODO: test for case-insensitive disposition value
}

void HeaderTest::testContentTypeHeader()
{
    ContentType *h;

    // empty header
    h = new ContentType();
    QVERIFY(h->isEmpty());

    // Empty content-type means text/plain (RFC 2045 §5.2)
    QVERIFY(h->isPlainText());
    QVERIFY(h->isText());

    // set a mimetype
    h->setMimeType("text/plain");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mimeType(), QByteArray("text/plain"));
    QCOMPARE(h->mediaType(), QByteArray("text"));
    QCOMPARE(h->subType(), QByteArray("plain"));
    QVERIFY(h->isText());
    QVERIFY(h->isPlainText());
    QVERIFY(!h->isMultipart());
    QVERIFY(!h->isPartial());
    QVERIFY(h->isMediatype("text"));
    QVERIFY(h->isSubtype("plain"));
    QCOMPARE(h->as7BitString(true), QByteArray("Content-Type: text/plain"));

    // add some parameters
    h->setId("bla");
    h->setCharset("us-ascii");
    QCOMPARE(h->as7BitString(false), QByteArray("text/plain; charset=\"us-ascii\"; id=\"bla\""));

    // clear header
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // parse a complete header
    h = new ContentType(0, "text/plain; charset=us-ascii (Plain text)");
    QVERIFY(h->isPlainText());
    QCOMPARE(h->charset(), QByteArray("us-ascii"));
    delete h;

    // bug #136631 (name with rfc 2231 style parameter wrapping)
    h = new ContentType(0, "text/plain;\n name*0=\"PIN_Brief_box1@xx.xxx.censored_Konfigkarte.confi\";\n name*1=\"guration.txt\"");
    QVERIFY(h->isPlainText());
    QCOMPARE(h->name(), QLatin1String("PIN_Brief_box1@xx.xxx.censored_Konfigkarte.configuration.txt"));
    delete h;

    // bug #197958 (name of Content-Type sent by Mozilla Thunderbird are not parsed -- test case generated with v2.0.0.22)
    h = new ContentType(0, "text/plain;\n name=\"=?ISO-8859-1?Q?lor=E9m_ipsum=2Etxt?=\"");
    QCOMPARE(h->name(), QString::fromUtf8("lorém ipsum.txt"));
    delete h;

    // bug #197958 (name of Content-Type sent by Mozilla Thunderbird are not parsed -- test case generated with v2.0.0.22)
    // But with unquoted string
    QEXPECT_FAIL("", "Unqouted rfc2047 strings are not supported as of now", Continue);
    h = new ContentType(0, "text/plain;\n name==?ISO-8859-1?Q?lor=E9m_ipsum=2Etxt?=");
    QCOMPARE(h->name(), QString::fromUtf8("lorém ipsum.txt"));
    delete h;

    // make ervin's unit test happy
    h = new ContentType;
    h->setMimeType("MULTIPART/MIXED");
    QVERIFY(h->isMultipart());
    QVERIFY(h->isMediatype("multipart"));
    QVERIFY(h->isMediatype("Multipart"));
    QVERIFY(h->isMediatype("MULTIPART"));
    QVERIFY(h->isSubtype("mixed"));
    QVERIFY(h->isSubtype("Mixed"));
    QVERIFY(h->isSubtype("MIXED"));
    QCOMPARE(h->mimeType(), QByteArray("MULTIPART/MIXED"));
    QCOMPARE(h->mediaType(), QByteArray("MULTIPART"));
    QCOMPARE(h->subType(), QByteArray("MIXED"));
    delete h;
}

void HeaderTest::testTokenHeader()
{
    Token *h;

    // empty header
    h = new Token();
    QVERIFY(h->isEmpty());

    // set a token
    h->setToken("bla");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(false), QByteArray("bla"));

    // clear it again
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // parse a header
    h = new Token(0, "value (comment)");
    QCOMPARE(h->token(), QByteArray("value"));
    QCOMPARE(h->as7BitString(false), QByteArray("value"));
    delete h;
}

void HeaderTest::testContentTransferEncoding()
{
    ContentTransferEncoding *h;

    // empty header
    h = new ContentTransferEncoding();
    QVERIFY(h->isEmpty());

    // set an encoding
    h->setEncoding(CEbinary);
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(true), QByteArray("Content-Transfer-Encoding: binary"));

    // clear again
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // parse a header
    h = new ContentTransferEncoding(0, "(comment) base64");
    QCOMPARE(h->encoding(), CEbase64);
    QCOMPARE(h->as7BitString(false), QByteArray("base64"));
    delete h;
}

void HeaderTest::testPhraseListHeader()
{
    PhraseList *h;

    // empty header
    h = new PhraseList();
    QVERIFY(h->isEmpty());
    delete h;

    // parse a simple phrase list
    h = new PhraseList(0, "foo,\n bar");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->phrases().count(), 2);
    QStringList phrases = h->phrases();
    QCOMPARE(phrases.takeFirst(), QLatin1String("foo"));
    QCOMPARE(phrases.takeFirst(), QLatin1String("bar"));
    QCOMPARE(h->as7BitString(false), QByteArray("foo, bar"));

    // clear header
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // TODO: encoded/quoted phrases
}

void HeaderTest::testDotAtomHeader()
{
    DotAtom *h;

    // empty header
    h = new DotAtom;
    QVERIFY(h->isEmpty());

    // parse a simple dot atom
    h->from7BitString("1.0 (mime version)");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->asUnicodeString(), QLatin1String("1.0"));

    // clear again
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // TODO: more complex atoms
}

void HeaderTest::testDateHeader()
{
    Date *h;

    // empty header
    h = new Date();
    QVERIFY(h->isEmpty());

    // parse a simple date
    h->from7BitString("Fri, 21 Nov 1997 09:55:06 -0600");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(1997, 11, 21));
    QCOMPARE(h->dateTime().time(), QTime(9, 55, 6));
    QCOMPARE(h->dateTime().utcOffset(), -6 * 3600);
    QCOMPARE(h->as7BitString(), QByteArray("Date: Fri, 21 Nov 1997 09:55:06 -0600"));

    // clear it again
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // white spaces and comment (from RFC 2822, Appendix A.5)
    h = new Date(0, "Thu,\n  13\n    Feb\n  1969\n  23:32\n  -0330 (Newfoundland Time)");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(1969, 2, 13));
    QCOMPARE(h->dateTime().time(), QTime(23, 32));
    QCOMPARE(h->dateTime().utcOffset(), -12600);
    QCOMPARE(h->as7BitString(false), QByteArray("Thu, 13 Feb 1969 23:32 -0330"));
    delete h;

    // obsolete date format (from RFC 2822, Appendix A.6.2)
    h = new Date(0, "21 Nov 97 09:55:06 GMT");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(1997, 11, 21));
    QCOMPARE(h->dateTime().time(), QTime(9, 55, 6));
    QCOMPARE(h->dateTime().utcOffset(), 0);
    delete h;

    // obsolete whitespaces and commnets (from RFC 2822, Appendix A.6.3)
    h = new Date(0, "Fri, 21 Nov 1997 09(comment):   55  :  06 -0600");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(1997, 11, 21));
    QCOMPARE(h->dateTime().time(), QTime(9, 55, 6));
    QCOMPARE(h->dateTime().utcOffset(), -6 * 3600);
    delete h;

    // Make sure uppercase OCT is parsed correctly - bug 150620
    h = new Date(0, "08 OCT 08 16:54:05 +0000");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2008, 10, 8));
    QCOMPARE(h->dateTime().time(), QTime(16, 54, 05));
    QCOMPARE(h->dateTime().utcOffset(), 0);
    delete h;

    // Test for bug 111633, year < 1970
    h = new Date(0, "Mon, 27 Aug 1956 21:31:46 +0200");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(1956, 8, 27));
    QCOMPARE(h->dateTime().time(), QTime(21, 31, 46));
    QCOMPARE(h->dateTime().utcOffset(), +2 * 3600);
    delete h;

    // Test for bug 207766
    h = new Date(0, "Fri, 18 Sep 2009 04:44:55 -0400");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2009, 9, 18));
    QCOMPARE(h->dateTime().time(), QTime(4, 44, 55));
    QCOMPARE(h->dateTime().utcOffset(), -4 * 3600);
    delete h;

    // Test for bug 260761
    h = new Date(0, "Sat, 18 Dec 2010 14:01:21 \"GMT\"");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2010, 12, 18));
    QCOMPARE(h->dateTime().time(), QTime(14, 1, 21));
    QCOMPARE(h->dateTime().utcOffset(), 0);
    delete h;

    // old asctime()-like formatted date; regression to KDE3; see bug 117848
    h = new Date(0, "Thu Mar 30 18:36:28 CEST 2006");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2006, 3, 30));
    QCOMPARE(h->dateTime().time(), QTime(18, 36, 28));
    QCOMPARE(h->dateTime().utcOffset(), 2 * 3600);
    delete h;

    h = new Date(0, "Thu Mar 30 18:36:28 2006");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2006, 3, 30));
    QCOMPARE(h->dateTime().time(), QTime(18, 36, 28));
    QCOMPARE(h->dateTime().utcOffset(), 0);
    delete h;

    // regression to KDE3; see bug 54098
    h = new Date(0, "Tue, Feb 04, 2003 00:01:20 +0000");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2003, 2, 4));
    QCOMPARE(h->dateTime().time(), QTime(0, 1, 20));
    QCOMPARE(h->dateTime().utcOffset(), 0);
    delete h;
}

void HeaderTest::testLinesHeader()
{
    Lines *h;

    // empty header
    h = new Lines();
    QVERIFY(h->isEmpty());
    QVERIFY(h->as7BitString().isEmpty());

    // set some content
    h->setNumberOfLines(5);
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(), QByteArray("Lines: 5"));

    // clear again
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // parse header with comment
    h = new Lines(0, "(this is a comment) 10 (and yet another comment)");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->numberOfLines(), 10);
    delete h;
}

void HeaderTest::testNewsgroupsHeader()
{
    Newsgroups *h;

    // empty header
    h = new Newsgroups();
    QVERIFY(h->isEmpty());
    QVERIFY(h->as7BitString().isEmpty());

    // set newsgroups
    QList<QByteArray> groups;
    groups << "gmane.comp.kde.devel.core" << "gmane.comp.kde.devel.buildsystem";
    h->setGroups(groups);
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(), QByteArray("Newsgroups: gmane.comp.kde.devel.core,gmane.comp.kde.devel.buildsystem"));

    // and clear again
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // parse a header
    h = new Newsgroups(0, "gmane.comp.kde.devel.core,gmane.comp.kde.devel.buildsystem");
    groups = h->groups();
    QCOMPARE(groups.count(), 2);
    QCOMPARE(groups.takeFirst(), QByteArray("gmane.comp.kde.devel.core"));
    QCOMPARE(groups.takeFirst(), QByteArray("gmane.comp.kde.devel.buildsystem"));
    delete h;

    // same again, this time with whitespaces and comments
    h = new Newsgroups();
    h->from7BitString("(comment) gmane.comp.kde.devel.core (second comment),\n gmane.comp.kde.devel.buildsystem (that all)");
    groups = h->groups();
    QCOMPARE(groups.count(), 2);
    QCOMPARE(groups.takeFirst(), QByteArray("gmane.comp.kde.devel.core"));
    QCOMPARE(groups.takeFirst(), QByteArray("gmane.comp.kde.devel.buildsystem"));
    delete h;
}

void HeaderTest::testControlHeader()
{
    Control *h;

    // empty header
    h = new Control();
    QVERIFY(h->isEmpty());
    QVERIFY(h->as7BitString().isEmpty());

    // set some content
    h->setCancel("<foo@bar>");
    QVERIFY(!h->isEmpty());
    QVERIFY(h->isCancel());
    QCOMPARE(h->as7BitString(),  QByteArray("Control: cancel <foo@bar>"));

    // clear again
    h->clear();
    QVERIFY(h->isEmpty());
    delete h;

    // parse a control header
    h = new Control(0, "cancel <foo@bar>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->parameter(), QByteArray("<foo@bar>"));
    QVERIFY(h->isCancel());
    QCOMPARE(h->controlType(), QByteArray("cancel"));
    delete h;
}

void HeaderTest::testReturnPath()
{
    ReturnPath *h;

    h = new ReturnPath();
    QVERIFY(h->isEmpty());
    QVERIFY(h->as7BitString().isEmpty());

    h->from7BitString("<foo@bar>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(true), QByteArray("Return-Path: <foo@bar>"));

    delete h;
}

void HeaderTest::noAbstractHeaders()
{
    From *h2 = new From(); delete h2;
    Sender *h3 = new Sender(); delete h3;
    To *h4 = new To(); delete h4;
    Cc *h5 = new Cc(); delete h5;
    Bcc *h6 = new Bcc(); delete h6;
    ReplyTo *h7 = new ReplyTo(); delete h7;
    Keywords *h8 = new Keywords(); delete h8;
    MIMEVersion *h9 = new MIMEVersion(); delete h9;
    MessageID *h10 = new MessageID(); delete h10;
    ContentID *h11 = new ContentID(); delete h11;
    Supersedes *h12 = new Supersedes(); delete h12;
    InReplyTo *h13 = new InReplyTo(); delete h13;
    References *h14 = new References(); delete h14;
    Generic *h15 = new Generic(); delete h15;
    Subject *h16 = new Subject(); delete h16;
    Organization *h17 = new Organization(); delete h17;
    ContentDescription *h18 = new ContentDescription(); delete h18;
    FollowUpTo *h22 = new FollowUpTo(); delete h22;
    UserAgent *h24 = new UserAgent(); delete h24;
}

void HeaderTest::testInvalidButOkQEncoding()
{
    // A stray '?' should not confuse the parser
    Subject subject;
    subject.from7BitString("=?us-ascii?q?Why?_Why_do_some_clients_violate_the_RFC?" "?=");
    QCOMPARE(subject.as7BitString(false), QByteArray("Why? Why do some clients violate the RFC?"));
}

void HeaderTest::testInvalidQEncoding_data()
{
    QTest::addColumn<QString>("encodedWord");

    // All examples below should not be treated as invalid encoded strings, since the '?=' is missing
    QTest::newRow("") << QString::fromLatin1("=?us-ascii?q?Why?_Why_do_some_clients_violate_the_RFC??");
    QTest::newRow("") << QString::fromLatin1("=?us-ascii?q?Why?_Why_do_some_clients_violate_the_RFC?");
    QTest::newRow("") << QString::fromLatin1("=?us-ascii?q?Why?_Why_do_some_clients_violate_the_RFC");
}

void HeaderTest::testInvalidQEncoding()
{
    using namespace HeaderParsing;
    QFETCH(QString, encodedWord);

    QByteArray tmp = encodedWord.toLatin1();
    const char *data = tmp.data();
    const char *start = data + 1;
    const char *end = data + strlen(data);
    QString result;
    QByteArray language;
    QByteArray usedCS;
    QVERIFY(!parseEncodedWord(start, end, result, language, usedCS));
}

void HeaderTest::testBug271192()
{
    QFETCH(QString, displayName);
    QFETCH(bool, quote);

    const QString addrSpec = QLatin1String("example@example.com");
    const QString mailbox = (quote ? QLatin1String("\"") : QString()) + displayName +
                            (quote ? QLatin1String("\"") : QString()) +
                            QLatin1String(" <") + addrSpec + QLatin1String(">");

    Headers::Generics::SingleMailbox *h = new Headers::Generics::SingleMailbox();
    h->fromUnicodeString(mailbox, "utf-8");
    QCOMPARE(h->displayNames().size(), 1);
    QCOMPARE(h->displayNames().first().toUtf8(), displayName.remove(QLatin1String("\\")).toUtf8());
    delete h;
    h = 0;

    Headers::Generics::MailboxList *h2 = new Headers::Generics::MailboxList();
    h2->fromUnicodeString(mailbox + QLatin1String(",") + mailbox, "utf-8");
    QCOMPARE(h2->displayNames().size(), 2);
    QCOMPARE(h2->displayNames()[0].toUtf8(), displayName.remove(QLatin1String("\\")).toUtf8());
    QCOMPARE(h2->displayNames()[1].toUtf8(), displayName.remove(QLatin1String("\\")).toUtf8());
    delete h2;
    h2 = 0;
}

void HeaderTest::testBug271192_data()
{
    QTest::addColumn<QString>("displayName");
    QTest::addColumn<bool>("quote");

    QTest::newRow("Plain") << QString::fromUtf8("John Doe") << false;
    QTest::newRow("Firstname 1") << QString::fromUtf8("Marc-André Lastname") << false;
    QTest::newRow("Firstname 2") << QString::fromUtf8("Интернет-компания Lastname") << false;
    QTest::newRow("Lastname") << QString::fromUtf8("Tobias König") << false;
    QTest::newRow("Firstname + Lastname") << QString::fromUtf8("Интернет-компания König") << false;
    QTest::newRow("Quotemarks") << QString::fromUtf8("John \\\"Rocky\\\" Doe") << true;
    QTest::newRow("Quotemarks") << QString::fromUtf8("Jöhn \\\"Röcky\\\" Döe") << true;

    QTest::newRow("Plain") << QString::fromUtf8("John Doe") << true;
    QTest::newRow("Firstname 1") << QString::fromUtf8("Marc-André Lastname") << true;
    QTest::newRow("Firstname 2") << QString::fromUtf8("Интернет-компания Lastname") << true;
    QTest::newRow("Lastname") << QString::fromUtf8("Tobias König") << true;
    QTest::newRow("Firstname + Lastname") << QString::fromUtf8("Интернет-компания König") << true;
    QTest::newRow("LastName, Firstname") << QString::fromUtf8("König, Интернет-компания") << true;
}

