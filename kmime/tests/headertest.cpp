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
  Headers::Generics::Ident* h = new Headers::Generics::Ident();
  h->from7BitString( QByteArray( "<1162746587.784559.5038.nullmailer@svn.kde.org>" ) );
  QCOMPARE( h->identifiers().count(), 1 );
  QCOMPARE( h->identifiers().first(), QString( "1162746587.784559.5038.nullmailer@svn.kde.org" ) );
}

#include "headertest.moc"
