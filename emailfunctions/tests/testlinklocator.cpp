/*
  This file is part of the kpimutils library.

  Copyright (C) 2005 Ingo Kloecker <kloecker@kde.org>
  Copyright (C) 2007 Allen Winter <winter@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

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

#include "testlinklocator.h"
#include "testlinklocator.moc"

QTEST_KDEMAIN( LinkLocatorTest, NoGUI )

#include "emailfunctions/linklocator.h"
using namespace KPIMUtils;

void LinkLocatorTest::testGetEmailAddress()
{
  // empty input
  LinkLocator ll1( QString(), 0 );
  QVERIFY( ll1.getEmailAddress() == QString() );

  // no '@' at scan position
  LinkLocator ll2( "foo@bar.baz", 0 );
  QVERIFY( ll2.getEmailAddress() == QString() );

  // '@' in local part
  LinkLocator ll3( "foo@bar@bar.baz", 7 );
  QVERIFY( ll3.getEmailAddress() == QString() );

  // empty local part
  LinkLocator ll4( "@bar.baz", 0 );
  QVERIFY( ll4.getEmailAddress() == QString() );
  LinkLocator ll5( ".@bar.baz", 1 );
  QVERIFY( ll5.getEmailAddress() == QString() );
  LinkLocator ll6( " @bar.baz", 1 );
  QVERIFY( ll6.getEmailAddress() == QString() );
  LinkLocator ll7( ".!#$%&'*+-/=?^_`{|}~@bar.baz",
                   strlen(".!#$%&'*+-/=?^_`{|}~") );
  QVERIFY( ll7.getEmailAddress() == QString() );

  // allowed special chars in local part of address
  LinkLocator ll8( "a.!#$%&'*+-/=?^_`{|}~@bar.baz",
                   strlen("a.!#$%&'*+-/=?^_`{|}~") );
  QVERIFY( ll8.getEmailAddress() == "a.!#$%&'*+-/=?^_`{|}~@bar.baz" );

  // '@' in domain part
  LinkLocator ll9 ( "foo@bar@bar.baz", 3 );
  QVERIFY( ll9.getEmailAddress() == QString() );

  // domain part without dot
  LinkLocator lla( "foo@bar", 3 );
  QVERIFY( lla.getEmailAddress() == QString() );
  LinkLocator llb( "foo@bar.", 3 );
  QVERIFY( llb.getEmailAddress() == QString() );
  LinkLocator llc( ".foo@bar", 4 );
  QVERIFY( llc.getEmailAddress() == QString() );
  LinkLocator lld( "foo@bar ", 3 );
  QVERIFY( lld.getEmailAddress() == QString() );
  LinkLocator lle( " foo@bar", 4 );
  QVERIFY( lle.getEmailAddress() == QString() );
  LinkLocator llf( "foo@bar-bar", 3 );
  QVERIFY( llf.getEmailAddress() == QString() );

  // empty domain part
  LinkLocator llg( "foo@", 3 );
  QVERIFY( llg.getEmailAddress() == QString() );
  LinkLocator llh( "foo@.", 3 );
  QVERIFY( llh.getEmailAddress() == QString() );
  LinkLocator lli( "foo@-", 3 );
  QVERIFY( lli.getEmailAddress() == QString() );

  // simple address
  LinkLocator llj( "foo@bar.baz", 3 );
  QVERIFY( llj.getEmailAddress() == "foo@bar.baz" );
  LinkLocator llk( "foo@bar.baz.", 3 );
  QVERIFY( llk.getEmailAddress() == "foo@bar.baz" );
  LinkLocator lll( ".foo@bar.baz", 4 );
  QVERIFY( lll.getEmailAddress() == "foo@bar.baz" );
  LinkLocator llm( "foo@bar.baz-", 3 );
  QVERIFY( llm.getEmailAddress() == "foo@bar.baz" );
  LinkLocator lln( "-foo@bar.baz", 4 );
  QVERIFY( lln.getEmailAddress() == "foo@bar.baz" );
  LinkLocator llo( "foo@bar.baz ", 3 );
  QVERIFY( llo.getEmailAddress() == "foo@bar.baz" );
  LinkLocator llp( " foo@bar.baz", 4 );
  QVERIFY( llp.getEmailAddress() == "foo@bar.baz" );
  LinkLocator llq( "foo@bar-bar.baz", 3 );
  QVERIFY( llq.getEmailAddress() == "foo@bar-bar.baz" );
}

