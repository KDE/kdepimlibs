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

// the following test cases are taken from KDE mailinglists, bug reports and
// RFC 2822, Appendix A

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
}

#include "headertest.moc"
