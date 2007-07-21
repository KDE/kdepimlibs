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
#include <qtest_kde.h>

#include <kmime_headers.h>

using namespace KMime;
using namespace KMime::Headers;
using namespace KMime::Headers::Generics;

// the following test cases are taken from KDE mailinglists, bug reports, RFC 2045,
// RFC 2183 and RFC 2822, Appendix A

QTEST_KDEMAIN( HeaderTest, NoGUI )

void HeaderTest::testIdentHeader()
{
  // empty header
  Headers::Generics::Ident* h = new Headers::Generics::Ident();
  QVERIFY( h->isEmpty() );

  // parse single identifier
  h->from7BitString( QByteArray( "<1162746587.784559.5038.nullmailer@svn.kde.org>" ) );
  QCOMPARE( h->identifiers().count(), 1 );
  QCOMPARE( h->identifiers().first(), QByteArray( "1162746587.784559.5038.nullmailer@svn.kde.org" ) );
  QCOMPARE( h->asUnicodeString(), QString("<1162746587.784559.5038.nullmailer@svn.kde.org>") );
  QVERIFY( !h->isEmpty() );

  // clearing a header
  h->clear();
  QVERIFY( h->isEmpty() );
  QVERIFY( h->identifiers().isEmpty() );
  delete h;

  // parse multiple identifiers
  h = new Headers::Generics::Ident();
  h->from7BitString( QByteArray( "<1234@local.machine.example> <3456@example.net>" ) );
  QCOMPARE( h->identifiers().count(), 2 );
  QList<QByteArray> ids = h->identifiers();
  QCOMPARE( ids.takeFirst(), QByteArray( "1234@local.machine.example" ) );
  QCOMPARE( ids.first(), QByteArray( "3456@example.net" ) );
  delete h;

  // parse multiple identifiers with folded headers
  h = new Headers::Generics::Ident();
  h->from7BitString( QByteArray( "<1234@local.machine.example>\n  <3456@example.net>" ) );
  QCOMPARE( h->identifiers().count(), 2 );
  ids = h->identifiers();
  QCOMPARE( ids.takeFirst(), QByteArray( "1234@local.machine.example" ) );
  QCOMPARE( ids.first(), QByteArray( "3456@example.net" ) );

  // appending of new identifiers (with and without angle-brackets)
  h->appendIdentifier( "<abcd.1234@local.machine.tld>" );
  h->appendIdentifier( "78910@example.net" );
  QCOMPARE( h->identifiers().count(), 4 );

  // assemble the final header
  QCOMPARE( h->as7BitString( false ), QByteArray("<1234@local.machine.example> <3456@example.net> <abcd.1234@local.machine.tld> <78910@example.net>") );
}

void HeaderTest::testAddressListHeader()
{
  // empty header
  Headers::Generics::AddressList *h = new Headers::Generics::AddressList();
  QVERIFY( h->isEmpty() );

  // parse single simple address
  h->from7BitString( "joe@where.test" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->addresses().count(), 1 );
  QCOMPARE( h->addresses().first(), QByteArray("joe@where.test") );
  QCOMPARE( h->displayNames().count(), 1 );
  QCOMPARE( h->displayNames().first(), QString() );
  QCOMPARE( h->prettyAddresses().count(), 1 );
  QCOMPARE( h->prettyAddresses().first(), QString("joe@where.test") );

  // clearing a header
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // parsing and re-assembling a single address with display name
  h = new Headers::Generics::AddressList();
  h->from7BitString( "Pete <pete@silly.example>" );
  QCOMPARE( h->addresses().count(), 1 );
  QCOMPARE( h->addresses().first(), QByteArray( "pete@silly.example" ) );
  QCOMPARE( h->displayNames().first(), QString("Pete") );
  QCOMPARE( h->prettyAddresses().first(), QString("Pete <pete@silly.example>") );
  QCOMPARE( h->as7BitString( false ), QByteArray("Pete <pete@silly.example>") );
  delete h;

  // parsing a single address with legacy comment style display name
  h = new Headers::Generics::AddressList();
  h->from7BitString( "jdoe@machine.example (John Doe)" );
  QCOMPARE( h->addresses().count(), 1 );
  QCOMPARE( h->addresses().first(), QByteArray( "jdoe@machine.example" ) );
  QCOMPARE( h->displayNames().first(), QString("John Doe") );
  QCOMPARE( h->prettyAddresses().first(), QString("John Doe <jdoe@machine.example>") );
  delete h;

  // parsing and re-assembling list of diffrent addresses
  h = new Headers::Generics::AddressList();
  h->from7BitString( "Mary Smith <mary@x.test>, jdoe@example.org, Who? <one@y.test>" );
  QCOMPARE( h->addresses().count(), 3 );
  QStringList names = h->displayNames();
  QCOMPARE( names.takeFirst(), QString("Mary Smith") );
  QCOMPARE( names.takeFirst(), QString() );
  QCOMPARE( names.takeFirst(), QString("Who?") );
  QCOMPARE( h->as7BitString( false ), QByteArray("Mary Smith <mary@x.test>, jdoe@example.org, Who? <one@y.test>") );
  delete h;

  // same again with some interessting quoting
  h = new Headers::Generics::AddressList();
  h->from7BitString( "\"Joe Q. Public\" <john.q.public@example.com>, <boss@nil.test>, \"Giant; \\\"Big\\\" Box\" <sysservices@example.net>" );
  QCOMPARE( h->addresses().count(), 3 );
  names = h->displayNames();
  QCOMPARE( names.takeFirst(), QString("Joe Q. Public") );
  QCOMPARE( names.takeFirst(), QString() );
  QCOMPARE( names.takeFirst(), QString("Giant; \"Big\" Box") );
  QCOMPARE( h->as7BitString( false ), QByteArray("\"Joe Q. Public\" <john.q.public@example.com>, boss@nil.test, \"Giant; \\\"Big\\\" Box\" <sysservices@example.net>") );
  delete h;

  // a display name with non-latin1 content
  h = new Headers::Generics::AddressList();
  h->from7BitString( "Ingo =?iso-8859-15?q?Kl=F6cker?= <kloecker@kde.org>" );
  QCOMPARE( h->addresses().count(), 1 );
  QCOMPARE( h->addresses().first(), QByteArray( "kloecker@kde.org" ) );
  QCOMPARE( h->displayNames().first(), QString::fromUtf8("Ingo Klöcker") );
  QCOMPARE( h->asUnicodeString(), QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>") );
  QCOMPARE( h->as7BitString( false ), QByteArray("Ingo =?ISO-8859-1?Q?Kl=F6cker?= <kloecker@kde.org>") );
  delete h;

  // again, this time legacy style
  h = new Headers::Generics::AddressList();
  h->from7BitString( "kloecker@kde.org (Ingo =?iso-8859-15?q?Kl=F6cker?=)" );
  QCOMPARE( h->addresses().count(), 1 );
  QCOMPARE( h->addresses().first(), QByteArray( "kloecker@kde.org" ) );
  QCOMPARE( h->displayNames().first(), QString::fromUtf8("Ingo Klöcker") );
  delete h;

  // parsing a empty group
  h = new Headers::Generics::AddressList();
  h->from7BitString( "Undisclosed recipients:;" );
  QCOMPARE( h->addresses().count(), 0 );
  delete h;

  // parsing and re-assembling a address list with a group
  h = new Headers::Generics::AddressList();
  h->from7BitString( "A Group:Chris Jones <c@a.test>,joe@where.test,John <jdoe@one.test>;" );
  QCOMPARE( h->addresses().count(), 3 );
  names = h->displayNames();
  QCOMPARE( names.takeFirst(), QString("Chris Jones") );
  QCOMPARE( names.takeFirst(), QString() );
  QCOMPARE( names.takeFirst(), QString("John") );
  QCOMPARE( h->as7BitString( false ), QByteArray("Chris Jones <c@a.test>, joe@where.test, John <jdoe@one.test>") );
  delete h;

  // modifying a header
  h = new Headers::Generics::AddressList();
  h->from7BitString( "John <jdoe@one.test>" );
  h->addAddress( "<kloecker@kde.org>", QString::fromUtf8("Ingo Klöcker") );
  h->addAddress( "c@a.test" );
  QCOMPARE( h->addresses().count(), 3 );
  QCOMPARE( h->asUnicodeString(), QString::fromUtf8("John <jdoe@one.test>, Ingo Klöcker <kloecker@kde.org>, c@a.test") );
  QCOMPARE( h->as7BitString( false ), QByteArray("John <jdoe@one.test>, Ingo =?ISO-8859-1?Q?Kl=F6cker?= <kloecker@kde.org>, c@a.test") );
  delete h;

  // parsing from utf-8
  h = new Headers::Generics::AddressList();
  h->fromUnicodeString( QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"), "utf-8" );
  QCOMPARE( h->addresses().count(), 1 );
  QCOMPARE( h->addresses().first(), QByteArray( "kloecker@kde.org" ) );
  QCOMPARE( h->displayNames().first(), QString::fromUtf8("Ingo Klöcker") );
  delete h;

  // based on bug #137033, a header broken in various ways: ';' as list separator,
  // unquoted '.' in display name
  h = new Headers::Generics::AddressList();
  h->from7BitString( "Vice@censored.serverkompetenz.net,\n    President@mail2.censored.net;\"Int\\\\\\\\\\\\\\\\\\\\'l\" Lotto Commission. <censored@yahoo.fr>" );
  QCOMPARE( h->addresses().count(), 3 );
  names = h->displayNames();
  QCOMPARE( names.takeFirst(), QString() );
  QCOMPARE( names.takeFirst(), QString() );
  // there is an wrong ' ' after the name, but since the header is completely
  // broken we can be happy it parses at all...
  QCOMPARE( names.takeFirst(), QString("Int\\\\\\\\\\'l Lotto Commission. ") );
  QList<QByteArray> addrs = h->addresses();
  QCOMPARE( addrs.takeFirst(), QByteArray("Vice@censored.serverkompetenz.net") );
  QCOMPARE( addrs.takeFirst(), QByteArray("President@mail2.censored.net") );
  QCOMPARE( addrs.takeFirst(), QByteArray("censored@yahoo.fr") );
  delete h;

  // based on bug #102010, a display name containing '<'
  h = new Headers::Generics::AddressList( 0, QByteArray("\"|<onrad\" <censored@censored.dy>") );
  QCOMPARE( h->addresses().count(), 1 );
  QCOMPARE( h->addresses().first(), QByteArray("censored@censored.dy") );
  QCOMPARE( h->displayNames().first(), QString("|<onrad") );
  QCOMPARE( h->as7BitString( false ), QByteArray("\"|<onrad\" <censored@censored.dy>") );

  // based on bug #93790 (legacy display name with nested comments)
  h = new Headers::Generics::AddressList( 0, QByteArray("first.name@domain.tld (first name (nickname))") );
  QCOMPARE( h->displayNames().count(), 1 );
  QCOMPARE( h->displayNames().first(), QString("first name (nickname)") );
  QCOMPARE( h->as7BitString( false ), QByteArray("\"first name (nickname)\" <first.name@domain.tld>") );
  delete h;

  // rfc 2047 encoding in quoted name (which is not allowed there)
  h = new Headers::Generics::AddressList();
  h->from7BitString( QByteArray( "\"Ingo =?iso-8859-15?q?Kl=F6cker?=\" <kloecker@kde.org>" ) );
  QCOMPARE( h->mailboxes().count(), 1 );
  QCOMPARE( h->asUnicodeString(), QString::fromUtf8( "Ingo =?iso-8859-15?q?Kl=F6cker?= <kloecker@kde.org>" ) );
  delete h;
}

void HeaderTest::testMailCopiesToHeader()
{
  Headers::MailCopiesTo *h;

  // empty header
  h = new Headers::MailCopiesTo();
  QVERIFY( h->isEmpty() );
  QVERIFY( !h->alwaysCopy() );
  QVERIFY( !h->neverCopy() );

  // set to always copy to poster
  h->setAlwaysCopy();
  QVERIFY( !h->isEmpty() );
  QVERIFY( h->alwaysCopy() );
  QVERIFY( !h->neverCopy() );
  QCOMPARE( h->as7BitString(), QByteArray( "Mail-Copies-To: poster" ) );

  // set to never copy
  h->setNeverCopy();
  QVERIFY( !h->isEmpty() );
  QVERIFY( !h->alwaysCopy() );
  QVERIFY( h->neverCopy() );
  QCOMPARE( h->as7BitString(), QByteArray( "Mail-Copies-To: nobody" ) );

  // clear header
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // parse copy to poster
  h = new MailCopiesTo( 0, "always" );
  QVERIFY( h->addresses().isEmpty() );
  QVERIFY( !h->isEmpty() );
  QVERIFY( h->alwaysCopy() );
  delete h;

  // parse never copy
  h = new MailCopiesTo( 0, "never" );
  QVERIFY( h->addresses().isEmpty() );
  QVERIFY( !h->isEmpty() );
  QVERIFY( h->neverCopy() );
  delete h;

  // parse address
  h = new MailCopiesTo( 0, "vkrause@kde.org" );
  QVERIFY( !h->addresses().isEmpty() );
  QVERIFY( h->alwaysCopy() );
  QVERIFY( !h->neverCopy() );
  QCOMPARE( h->as7BitString(), QByteArray( "Mail-Copies-To: vkrause@kde.org" ) );
  delete h;
}

void HeaderTest::testParametrizedHeader()
{
  Parametrized *h;

  // empty header
  h = new Parametrized();
  QVERIFY( h->isEmpty() );

  // add a parameter
  h->setParameter( "filename", "bla.jpg" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->parameter( "filename" ), QString( "bla.jpg" ) );
  QCOMPARE( h->as7BitString( false ), QByteArray( "filename=\"bla.jpg\"" ) );

  // clear again
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // parse a parameter list
  h = new Parametrized( 0, "filename=genome.jpeg;\n modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"" );
  QCOMPARE( h->parameter( "filename" ), QString( "genome.jpeg" ) );
  QCOMPARE( h->parameter( "modification-date" ), QString( "Wed, 12 Feb 1997 16:29:51 -0500" ) );
  QCOMPARE( h->as7BitString( false ), QByteArray( "filename=\"genome.jpeg\"; modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"" ) );
  delete h;

  // quoting of whitespaces in parameter value
  h = new Parametrized();
  h->setParameter( "boundary", "simple boundary" );
  QCOMPARE( h->as7BitString( false ), QByteArray( "boundary=\"simple boundary\"" ) );
  delete h;

  // TODO: test RFC 2047 encoded values
  // TODO: test case-insensitive key-names
}

void HeaderTest::testContentDispositionHeader()
{
  ContentDisposition *h;

  // empty header
  h = new ContentDisposition();
  QVERIFY( h->isEmpty() );

  // set some values
  h->setFilename( "test.jpg" );
  QVERIFY( h->isEmpty() );
  QVERIFY( h->as7BitString( false ).isEmpty() );
  h->setDisposition( CDattachment );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->as7BitString( false ), QByteArray( "attachment; filename=\"test.jpg\"" ) );
  delete h;

  // parse parameter-less header
  h = new ContentDisposition( 0, "inline" );
  QCOMPARE( h->disposition(), CDinline );
  QVERIFY( h->filename().isEmpty() );
  QCOMPARE( h->as7BitString( true ), QByteArray( "Content-Disposition: inline" ) );
  delete h;

  // parse header with parameter
  h = new ContentDisposition( 0, "attachment; filename=genome.jpeg;\n modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\";");
  QCOMPARE( h->disposition(), CDattachment );
  QCOMPARE( h->filename(), QString( "genome.jpeg" ) );
  delete h;

  // TODO: test for case-insensitive disposition value
}

void HeaderTest::testContentTypeHeader()
{
  ContentType* h;

  // empty header
  h = new ContentType();
  QVERIFY( h->isEmpty() );

  // set a mimetype
  h->setMimeType( "text/plain" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->mimeType(), QByteArray( "text/plain" ) );
  QCOMPARE( h->mediaType(), QByteArray("text") );
  QCOMPARE( h->subType(), QByteArray("plain") );
  QVERIFY( h->isText() );
  QVERIFY( h->isPlainText() );
  QVERIFY( !h->isMultipart() );
  QVERIFY( !h->isPartial() );
  QVERIFY( h->isMediatype( "text" ) );
  QVERIFY( h->isSubtype( "plain" ) );
  QCOMPARE( h->as7BitString( true ), QByteArray( "Content-Type: text/plain" ) );

  // add some parameters
  h->setId( "bla" );
  h->setCharset( "us-ascii" );
  QCOMPARE( h->as7BitString( false ), QByteArray( "text/plain; charset=\"us-ascii\"; id=\"bla\"" ) );

  // clear header
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // parse a complete header
  h = new ContentType( 0, "text/plain; charset=us-ascii (Plain text)" );
  QVERIFY( h->isPlainText() );
  QCOMPARE( h->charset(), QByteArray( "us-ascii" ) );
  delete h;

  // bug #136631 (name with rfc 2231 style parameter wrapping)
  h = new ContentType( 0, "text/plain;\n name*0=\"PIN_Brief_box1@xx.xxx.censored_Konfigkarte.confi\";\n name*1=\"guration.txt\"" );
  QVERIFY( h->isPlainText() );
  QCOMPARE( h->name(), QString( "PIN_Brief_box1@xx.xxx.censored_Konfigkarte.configuration.txt" ) );
  delete h;
}

void HeaderTest::testTokenHeader()
{
  Token *h;

  // empty header
  h = new Token();
  QVERIFY( h->isEmpty() );

  // set a token
  h->setToken( "bla" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->as7BitString( false ), QByteArray( "bla" ) );

  // clear it again
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // parse a header
  h = new Token( 0, "value (comment)" );
  QCOMPARE( h->token(), QByteArray("value") );
  QCOMPARE( h->as7BitString( false ), QByteArray("value") );
  delete h;
}

void HeaderTest::testContentTransferEncoding()
{
  ContentTransferEncoding *h;

  // empty header
  h = new ContentTransferEncoding();
  QVERIFY( h->isEmpty() );

  // set an encoding
  h->setEncoding( CEbinary );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->as7BitString( true ), QByteArray("Content-Transfer-Encoding: binary") );

  // clear again
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // parse a header
  h = new ContentTransferEncoding( 0, "(comment) base64" );
  QCOMPARE( h->encoding(), CEbase64 );
  QCOMPARE( h->as7BitString( false ), QByteArray("base64") );
  delete h;
}

void HeaderTest::testPhraseListHeader()
{
  PhraseList *h;

  // empty header
  h = new PhraseList();
  QVERIFY( h->isEmpty() );
  delete h;

  // parse a simple phrase list
  h = new PhraseList( 0, "foo,\n bar" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->phrases().count(), 2 );
  QStringList phrases = h->phrases();
  QCOMPARE( phrases.takeFirst(), QString( "foo" ) );
  QCOMPARE( phrases.takeFirst(), QString( "bar" ) );
  QCOMPARE( h->as7BitString( false ), QByteArray("foo, bar") );

  // clear header
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // TODO: encoded/quoted phrases
}

void HeaderTest::testDotAtomHeader()
{
  DotAtom *h;

  // empty header
  h = new DotAtom;
  QVERIFY( h->isEmpty() );

  // parse a simple dot atom
  h->from7BitString( "1.0 (mime version)" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->asUnicodeString(), QString( "1.0" ) );

  // clear again
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // TODO: more complex atoms
}

void HeaderTest::testDateHeader()
{
  Date *h;

  // empty header
  h = new Date();
  QVERIFY( h->isEmpty() );

  // parse a simple date
  h->from7BitString( "Fri, 21 Nov 1997 09:55:06 -0600" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->dateTime().date(), QDate( 1997, 11, 21 ) );
  QCOMPARE( h->dateTime().time(), QTime( 9, 55, 6 ) );
  QCOMPARE( h->dateTime().utcOffset(), -6 * 3600 );
  QCOMPARE( h->as7BitString(), QByteArray( "Date: Fri, 21 Nov 1997 09:55:06 -0600" ) );

  // clear it again
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // white spaces and comment (from RFC 2822, Appendix A.5)
  h = new Date( 0, "Thu,\n  13\n    Feb\n  1969\n  23:32\n  -0330 (Newfoundland Time)" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->dateTime().date(), QDate( 1969, 2, 13 ) );
  QCOMPARE( h->dateTime().time(), QTime( 23, 32 ) );
  QCOMPARE( h->dateTime().utcOffset(), -12600 );
  QCOMPARE( h->as7BitString( false ), QByteArray( "Thu, 13 Feb 1969 23:32 -0330" ) );
  delete h;

  // obsolete date format (from RFC 2822, Appendix A.6.2)
  h = new Date( 0, "21 Nov 97 09:55:06 GMT" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->dateTime().date(), QDate( 1997, 11, 21 ) );
  QCOMPARE( h->dateTime().time(), QTime( 9, 55, 6 ) );
  QCOMPARE( h->dateTime().utcOffset(), 0 );
  delete h;

  // obsolete whitespaces and commnets (from RFC 2822, Appendix A.6.3)
  h = new Date( 0, "Fri, 21 Nov 1997 09(comment):   55  :  06 -0600" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->dateTime().date(), QDate( 1997, 11, 21 ) );
  QCOMPARE( h->dateTime().time(), QTime( 9, 55, 6 ) );
  QCOMPARE( h->dateTime().utcOffset(), -6 * 3600 );
  delete h;
}

void HeaderTest::testLinesHeader()
{
  Lines *h;

  // empty header
  h = new Lines();
  QVERIFY( h->isEmpty() );
  QVERIFY( h->as7BitString().isEmpty() );

  // set some content
  h->setNumberOfLines( 5 );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->as7BitString(), QByteArray( "Lines: 5" ) );

  // clear again
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // parse header with comment
  h = new Lines( 0, "(this is a comment) 10 (and yet another comment)" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->numberOfLines(), 10 );
  delete h;
}

void HeaderTest::testNewsgroupsHeader()
{
  Newsgroups *h;

  // empty header
  h = new Newsgroups();
  QVERIFY( h->isEmpty() );
  QVERIFY( h->as7BitString().isEmpty() );

  // set newsgroups
  QList<QByteArray> groups;
  groups << "gmane.comp.kde.devel.core" << "gmane.comp.kde.devel.buildsystem";
  h->setGroups( groups );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->as7BitString(), QByteArray( "Newsgroups: gmane.comp.kde.devel.core,gmane.comp.kde.devel.buildsystem" ) );

  // and clear again
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // parse a header
  h = new Newsgroups( 0, "gmane.comp.kde.devel.core,gmane.comp.kde.devel.buildsystem" );
  groups = h->groups();
  QCOMPARE( groups.count(), 2 );
  QCOMPARE( groups.takeFirst(), QByteArray("gmane.comp.kde.devel.core") );
  QCOMPARE( groups.takeFirst(), QByteArray("gmane.comp.kde.devel.buildsystem") );
  delete h;

  // same again, this time with whitespaces and comments
  h = new Newsgroups();
  h->from7BitString( "(comment) gmane.comp.kde.devel.core (second comment),\n gmane.comp.kde.devel.buildsystem (that all)" );
  groups = h->groups();
  QCOMPARE( groups.count(), 2 );
  QCOMPARE( groups.takeFirst(), QByteArray("gmane.comp.kde.devel.core") );
  QCOMPARE( groups.takeFirst(), QByteArray("gmane.comp.kde.devel.buildsystem") );
  delete h;
}

void HeaderTest::testControlHeader()
{
  Control *h;

  // empty header
  h = new Control();
  QVERIFY( h->isEmpty() );
  QVERIFY( h->as7BitString().isEmpty() );

  // set some content
  h->setCancel( "<foo@bar>" );
  QVERIFY( !h->isEmpty() );
  QVERIFY( h->isCancel() );
  QCOMPARE( h->as7BitString(),  QByteArray( "Control: cancel <foo@bar>" ) );

  // clear again
  h->clear();
  QVERIFY( h->isEmpty() );
  delete h;

  // parse a control header
  h = new Control( 0, "cancel <foo@bar>" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->parameter(), QByteArray("<foo@bar>") );
  QVERIFY( h->isCancel() );
  QCOMPARE( h->controlType(), QByteArray("cancel") );
  delete h;
}

void HeaderTest::testReturnPath()
{
  ReturnPath *h;

  h = new ReturnPath();
  QVERIFY( h->isEmpty() );
  QVERIFY( h->as7BitString().isEmpty() );

  h->from7BitString( "<foo@bar>" );
  QVERIFY( !h->isEmpty() );
  QCOMPARE( h->as7BitString( true ), QByteArray( "Return-Path: <foo@bar>" ) );

  delete h;
}

void HeaderTest::noAbstractHeaders()
{
  From* h2 = new From(); delete h2;
  Sender* h3 = new Sender(); delete h3;
  To* h4 = new To(); delete h4;
  Cc* h5 = new Cc(); delete h5;
  Bcc* h6 = new Bcc(); delete h6;
  ReplyTo* h7 = new ReplyTo(); delete h7;
  Keywords* h8 = new Keywords(); delete h8;
  MIMEVersion* h9 = new MIMEVersion(); delete h9;
  MessageID* h10 = new MessageID(); delete h10;
  ContentID* h11 = new ContentID(); delete h11;
  Supersedes* h12 = new Supersedes(); delete h12;
  InReplyTo* h13 = new InReplyTo(); delete h13;
  References* h14 = new References(); delete h14;
  Generic* h15 = new Generic(); delete h15;
  Subject* h16 = new Subject(); delete h16;
  Organization* h17 = new Organization(); delete h17;
  ContentDescription* h18 = new ContentDescription(); delete h18;
  FollowUpTo* h22 = new FollowUpTo(); delete h22;
  UserAgent* h24 = new UserAgent(); delete h24;
}

#include "headertest.moc"
