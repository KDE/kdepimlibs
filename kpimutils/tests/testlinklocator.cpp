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
#include <kdebug.h>

#include "testlinklocator.h"
#include "testlinklocator.moc"

QTEST_KDEMAIN( LinkLocatorTest, NoGUI )

#include "kpimutils/linklocator.h"
using namespace KPIMUtils;

void LinkLocatorTest::testGetEmailAddress()
{
  // empty input
  const QString emptyQString;
  LinkLocator ll1( emptyQString, 0 );
  QVERIFY( ll1.getEmailAddress().isEmpty() );

  // no '@' at scan position
  LinkLocator ll2( "foo@bar.baz", 0 );
  QVERIFY( ll2.getEmailAddress().isEmpty() );

  // '@' in local part
  LinkLocator ll3( "foo@bar@bar.baz", 7 );
  QVERIFY( ll3.getEmailAddress().isEmpty() );

  // empty local part
  LinkLocator ll4( "@bar.baz", 0 );
  QVERIFY( ll4.getEmailAddress().isEmpty() );
  LinkLocator ll5( ".@bar.baz", 1 );
  QVERIFY( ll5.getEmailAddress().isEmpty() );
  LinkLocator ll6( " @bar.baz", 1 );
  QVERIFY( ll6.getEmailAddress().isEmpty() );
  LinkLocator ll7( ".!#$%&'*+-/=?^_`{|}~@bar.baz",
                   strlen( ".!#$%&'*+-/=?^_`{|}~" ) );
  QVERIFY( ll7.getEmailAddress().isEmpty() );

  // allowed special chars in local part of address
  LinkLocator ll8( "a.!#$%&'*+-/=?^_`{|}~@bar.baz",
                   strlen( "a.!#$%&'*+-/=?^_`{|}~" ) );
  QVERIFY( ll8.getEmailAddress() == "a.!#$%&'*+-/=?^_`{|}~@bar.baz" );

  // '@' in domain part
  LinkLocator ll9 ( "foo@bar@bar.baz", 3 );
  QVERIFY( ll9.getEmailAddress().isEmpty() );

  // domain part without dot
  LinkLocator lla( "foo@bar", 3 );
  QVERIFY( lla.getEmailAddress().isEmpty() );
  LinkLocator llb( "foo@bar.", 3 );
  QVERIFY( llb.getEmailAddress().isEmpty() );
  LinkLocator llc( ".foo@bar", 4 );
  QVERIFY( llc.getEmailAddress().isEmpty() );
  LinkLocator lld( "foo@bar ", 3 );
  QVERIFY( lld.getEmailAddress().isEmpty() );
  LinkLocator lle( " foo@bar", 4 );
  QVERIFY( lle.getEmailAddress().isEmpty() );
  LinkLocator llf( "foo@bar-bar", 3 );
  QVERIFY( llf.getEmailAddress().isEmpty() );

  // empty domain part
  LinkLocator llg( "foo@", 3 );
  QVERIFY( llg.getEmailAddress().isEmpty() );
  LinkLocator llh( "foo@.", 3 );
  QVERIFY( llh.getEmailAddress().isEmpty() );
  LinkLocator lli( "foo@-", 3 );
  QVERIFY( lli.getEmailAddress().isEmpty() );

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

void LinkLocatorTest::testGetUrl()
{
  QStringList brackets;
  brackets << "" << "";   // no brackets
  brackets << "(" << ")";
  brackets << "<" << ">";
  brackets << "[" << "]";
  brackets << "<link>" << "</link>";

  for (int i = 0; i < brackets.count(); i += 2)
    testGetUrl2(brackets[i], brackets[i+1]);
}

void LinkLocatorTest::testGetUrl2(const QString &left, const QString &right)
{
  QStringList schemas;
  schemas << "http://";
  schemas << "https://";
  schemas << "vnc://";
  schemas << "fish://";
  schemas << "ftp://";
  schemas << "ftps://";
  schemas << "sftp://";
  schemas << "smb://";
  schemas << "file://";

  QStringList urls;
  urls << "www.kde.org";
  urls << "user@www.kde.org";
  urls << "user:pass@www.kde.org";
  urls << "user:pass@www.kde.org:1234";
  urls << "user:pass@www.kde.org:1234/sub/path";
  urls << "user:pass@www.kde.org:1234/sub/path?a=1";
  urls << "user:pass@www.kde.org:1234/sub/path?a=1#anchor";
  urls << "user:pass@www.kde.org:1234/sub/path/special(123)?a=1#anchor";
  urls << "user:pass@www.kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor";

  foreach (QString schema, schemas)
  {
    foreach (QString url, urls)
    {
      QString test(left + schema + url + right);
      LinkLocator ll(test, left.length());
      QString gotUrl = ll.getUrl();

      bool ok = ( gotUrl == (schema + url) );
      //qDebug() << "check:" << (ok ? "OK" : "NOK") << test << "=>" << (schema + url);
      QVERIFY2( ok, qPrintable(test) );
    }
  }

  QStringList urlsWithoutSchema;
  urlsWithoutSchema << ".kde.org";
  urlsWithoutSchema << ".kde.org:1234/sub/path";
  urlsWithoutSchema << ".kde.org:1234/sub/path?a=1";
  urlsWithoutSchema << ".kde.org:1234/sub/path?a=1#anchor";
  urlsWithoutSchema << ".kde.org:1234/sub/path/special(123)?a=1#anchor";
  urlsWithoutSchema << ".kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor";

  QStringList starts;
  starts << "www" << "ftp" << "news:www";

  foreach (QString start, starts)
  {
    foreach (QString url, urlsWithoutSchema)
    {
      QString test(left + start + url + right);
      LinkLocator ll(test, left.length());
      QString gotUrl = ll.getUrl();

      bool ok = ( gotUrl == (start + url) );
      //qDebug() << "check:" << (ok ? "OK" : "NOK") << test << "=>" << (start + url);
      QVERIFY2( ok, qPrintable(test) );
    }
  }

  // mailto
  {
    QString addr = "mailto:test@kde.org";
    QString test(left + addr + right);
    LinkLocator ll(test, left.length());

    QString gotUrl = ll.getUrl();

    bool ok = ( gotUrl == addr );
    //qDebug() << "check:" << (ok ? "OK" : "NOK") << test << "=>" << addr;
    QVERIFY2( ok, qPrintable(test) );
  }
}

void LinkLocatorTest::testHtmlConvert_data()
{
  QTest::addColumn<QString>("plainText");
  QTest::addColumn<int>("flags");
  QTest::addColumn<QString>("htmlText");

  //QTest::newRow( "" ) << "foo" << 0 << "foo";
  //QTest::newRow( "" ) << "  foo " << 0 << "  foo ";
  // Linker error when using PreserveSpaces, therefore the hardcoded 0x01
  QTest::newRow( "" ) << " foo" << 0x01 << "&nbsp;foo";
  QTest::newRow( "" ) << "  foo" << 0x01 << "&nbsp;&nbsp;foo";
  QTest::newRow( "" ) << "  foo  " << 0x01 << "&nbsp;&nbsp;foo&nbsp;&nbsp;";
  QTest::newRow( "" ) << "  foo " << 0x01 << "&nbsp;&nbsp;foo&nbsp;";
  QTest::newRow( "" ) << "bla bla bla bla bla" << 0x01 << "bla bla bla bla bla";
  QTest::newRow( "" ) << "bla bla bla \n  bla bla bla " << 0x01
                      << "bla bla bla&nbsp;<br />\n&nbsp;&nbsp;bla bla bla&nbsp;";
  QTest::newRow( "" ) << "bla bla  bla" << 0x01
                      << "bla bla&nbsp;&nbsp;bla";
  QTest::newRow( "" ) << " bla bla \n bla bla a\n  bla bla " << 0x01
                      << "&nbsp;bla bla&nbsp;<br />\n&nbsp;bla bla a<br />\n&nbsp;&nbsp;bla bla&nbsp;";
}

void LinkLocatorTest::testHtmlConvert()
{
  QFETCH(QString, plainText);
  QFETCH(int, flags);
  QFETCH(QString, htmlText);

  QString actualHtml = LinkLocator::convertToHtml( plainText, flags );
  QCOMPARE( actualHtml, htmlText );
}


