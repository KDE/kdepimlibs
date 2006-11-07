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

QTEST_KDEMAIN( HeaderTest, NoGUI )

void HeaderTest::testIdentHeader()
{
  // parse single identifier
  Headers::Generics::Ident* h = new Headers::Generics::Ident();
  h->from7BitString( QByteArray( "<1162746587.784559.5038.nullmailer@svn.kde.org>" ) );
  QCOMPARE( h->identifiers().count(), 1 );
  QCOMPARE( h->identifiers().first(), QByteArray( "1162746587.784559.5038.nullmailer@svn.kde.org" ) );
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

#include "headertest.moc"
