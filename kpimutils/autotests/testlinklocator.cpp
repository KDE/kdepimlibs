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

#include "testlinklocator.h"


#include <qtest.h>
#include <qdebug.h>

// GUI test, since the smileys use GUI stuff
QTEST_MAIN( LinkLocatorTest )

#include "kpimutils/linklocator.h"
using namespace KPIMUtils;

void LinkLocatorTest::testGetEmailAddress()
{
  // empty input
  const QString emptyQString;
  LinkLocator ll1( emptyQString, 0 );
  QVERIFY( ll1.getEmailAddress().isEmpty() );

  // no '@' at scan position
  LinkLocator ll2( QLatin1String("foo@bar.baz"), 0 );
  QVERIFY( ll2.getEmailAddress().isEmpty() );

  // '@' in local part
  LinkLocator ll3( QLatin1String("foo@bar@bar.baz"), 7 );
  QVERIFY( ll3.getEmailAddress().isEmpty() );

  // empty local part
  LinkLocator ll4( QLatin1String("@bar.baz"), 0 );
  QVERIFY( ll4.getEmailAddress().isEmpty() );
  LinkLocator ll5( QLatin1String(".@bar.baz"), 1 );
  QVERIFY( ll5.getEmailAddress().isEmpty() );
  LinkLocator ll6( QLatin1String(" @bar.baz"), 1 );
  QVERIFY( ll6.getEmailAddress().isEmpty() );
  LinkLocator ll7( QLatin1String(".!#$%&'*+-/=?^_`{|}~@bar.baz"),
                   strlen( ".!#$%&'*+-/=?^_`{|}~" ) );
  QVERIFY( ll7.getEmailAddress().isEmpty() );

  // allowed special chars in local part of address
  LinkLocator ll8( QLatin1String("a.!#$%&'*+-/=?^_`{|}~@bar.baz"),
                   strlen( "a.!#$%&'*+-/=?^_`{|}~" ) );
  QVERIFY( ll8.getEmailAddress() == QLatin1String("a.!#$%&'*+-/=?^_`{|}~@bar.baz") );

  // '@' in domain part
  LinkLocator ll9 ( QLatin1String("foo@bar@bar.baz"), 3 );
  QVERIFY( ll9.getEmailAddress().isEmpty() );

  // domain part without dot
  LinkLocator lla( QLatin1String("foo@bar"), 3 );
  QVERIFY( lla.getEmailAddress().isEmpty() );
  LinkLocator llb( QLatin1String("foo@bar."), 3 );
  QVERIFY( llb.getEmailAddress().isEmpty() );
  LinkLocator llc( QLatin1String(".foo@bar"), 4 );
  QVERIFY( llc.getEmailAddress().isEmpty() );
  LinkLocator lld( QLatin1String("foo@bar "), 3 );
  QVERIFY( lld.getEmailAddress().isEmpty() );
  LinkLocator lle( QLatin1String(" foo@bar"), 4 );
  QVERIFY( lle.getEmailAddress().isEmpty() );
  LinkLocator llf( QLatin1String("foo@bar-bar"), 3 );
  QVERIFY( llf.getEmailAddress().isEmpty() );

  // empty domain part
  LinkLocator llg( QLatin1String("foo@"), 3 );
  QVERIFY( llg.getEmailAddress().isEmpty() );
  LinkLocator llh( QLatin1String("foo@."), 3 );
  QVERIFY( llh.getEmailAddress().isEmpty() );
  LinkLocator lli( QLatin1String("foo@-"), 3 );
  QVERIFY( lli.getEmailAddress().isEmpty() );

  // simple address
  LinkLocator llj( QLatin1String("foo@bar.baz"), 3 );
  QVERIFY( llj.getEmailAddress() == QLatin1String("foo@bar.baz") );
  LinkLocator llk( QLatin1String("foo@bar.baz."), 3 );
  QVERIFY( llk.getEmailAddress() == QLatin1String("foo@bar.baz") );
  LinkLocator lll( QLatin1String(".foo@bar.baz"), 4 );
  QVERIFY( lll.getEmailAddress() == QLatin1String("foo@bar.baz") );
  LinkLocator llm( QLatin1String("foo@bar.baz-"), 3 );
  QVERIFY( llm.getEmailAddress() == QLatin1String("foo@bar.baz") );
  LinkLocator lln( QLatin1String("-foo@bar.baz"), 4 );
  QVERIFY( lln.getEmailAddress() == QLatin1String("foo@bar.baz") );
  LinkLocator llo( QLatin1String("foo@bar.baz "), 3 );
  QVERIFY( llo.getEmailAddress() == QLatin1String("foo@bar.baz") );
  LinkLocator llp( QLatin1String(" foo@bar.baz"), 4 );
  QVERIFY( llp.getEmailAddress() == QLatin1String("foo@bar.baz") );
  LinkLocator llq( QLatin1String("foo@bar-bar.baz"), 3 );
  QVERIFY( llq.getEmailAddress() == QLatin1String("foo@bar-bar.baz") );
}

void LinkLocatorTest::testGetUrl()
{
  QStringList brackets;
  brackets << QLatin1String("") << QLatin1String("");   // no brackets
  brackets << QLatin1String("<") <<QLatin1String( ">");
  brackets << QLatin1String("[" )<< QLatin1String("]");
  brackets << QLatin1String("\"") << QLatin1String("\"");
  brackets << QLatin1String("<link>") << QLatin1String("</link>");

  for ( int i = 0; i < brackets.count(); i += 2 ) {
    testGetUrl2( brackets[ i ], brackets[ i + 1 ] );
  }
}

void LinkLocatorTest::testGetUrl2( const QString &left, const QString &right )
{
  QStringList schemas;
  schemas << QLatin1String("http://");
  schemas << QLatin1String("https://");
  schemas << QLatin1String("vnc://");
  schemas << QLatin1String("fish://");
  schemas << QLatin1String("ftp://");
  schemas << QLatin1String("ftps://");
  schemas << QLatin1String("sftp://");
  schemas << QLatin1String("smb://");
  schemas << QLatin1String("file://");

  QStringList urls;
  urls << QLatin1String("www.kde.org");
  urls << QLatin1String("user@www.kde.org");
  urls << QLatin1String("user:pass@www.kde.org");
  urls << QLatin1String("user:pass@www.kde.org:1234");
  urls << QLatin1String("user:pass@www.kde.org:1234/sub/path");
  urls << QLatin1String("user:pass@www.kde.org:1234/sub/path?a=1");
  urls << QLatin1String("user:pass@www.kde.org:1234/sub/path?a=1#anchor");
  urls << QLatin1String("user:pass@www.kde.org:1234/sub/\npath  \n /long/  path \t  ?a=1#anchor");
  urls << QLatin1String("user:pass@www.kde.org:1234/sub/path/special(123)?a=1#anchor");
  urls << QLatin1String("user:pass@www.kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor");
  urls << QLatin1String("user:pass@www.kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla");
  urls << QLatin1String("user:pass@www.kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla]");
  urls << QLatin1String("user:pass@www.kde.org:1234/\nsub/path:with:colon/\nspecial(123)?\na=1#anchor[bla]");
  urls << QLatin1String("user:pass@www.kde.org:1234/  \n  sub/path:with:colon/  \n\t   \t   special(123)?"
          "\n\t  \n\t   a=1#anchor[bla]");

  foreach ( const QString &schema, schemas ) {
    foreach ( QString url, urls ) { //krazy:exclude=foreach
      // by definition: if the URL is enclosed in brackets, the URL itself is not allowed
      // to contain the closing bracket, as this would be detected as the end of the URL
      if ( ( left.length() == 1 ) && ( url.contains( right[ 0 ] ) ) ) {
        continue;
      }

      // if the url contains a whitespace, it must be enclosed with brackets
      if ( ( url.contains( QLatin1Char('\n') ) || url.contains( QLatin1Char('\t') ) || url.contains( QLatin1Char(' ') ) ) &&
           left.isEmpty() ) {
        continue;
      }

      QString test( left + schema + url + right );
      LinkLocator ll( test, left.length() );
      QString gotUrl = ll.getUrl();

      // we want to have the url without whitespace
      url.remove( QLatin1Char(' ') );
      url.remove( QLatin1Char('\n') );
      url.remove( QLatin1Char('\t') );

      bool ok = ( gotUrl == ( schema + url ) );
      //qDebug() << "check:" << (ok ? "OK" : "NOK") << test << "=>" << (schema + url);
      if ( !ok ) {
        qDebug() << "got:" << gotUrl;
      }
      QVERIFY2( ok, qPrintable( test ) );
    }
  }

  QStringList urlsWithoutSchema;
  urlsWithoutSchema << QLatin1String(".kde.org");
  urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path");
  urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path?a=1");
  urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path?a=1#anchor");
  urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path/special(123)?a=1#anchor");
  urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor");
  urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla");
  urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla]");
  urlsWithoutSchema << QLatin1String(".kde.org:1234/\nsub/path:with:colon/\nspecial(123)?\na=1#anchor[bla]");
  urlsWithoutSchema << QLatin1String(".kde.org:1234/  \n  sub/path:with:colon/  \n\t   \t   special(123)?"
                       "\n\t  \n\t   a=1#anchor[bla]");

  QStringList starts;
  starts << QLatin1String("www") << QLatin1String("ftp") << QLatin1String("news:www");

  foreach ( const QString &start, starts ) {
    foreach ( QString url, urlsWithoutSchema ) { //krazy:exclude=foreach
      // by definition: if the URL is enclosed in brackets, the URL itself is not allowed
      // to contain the closing bracket, as this would be detected as the end of the URL
      if ( ( left.length() == 1 ) && ( url.contains( right[ 0 ] ) ) ) {
        continue;
      }

      // if the url contains a whitespace, it must be enclosed with brackets
      if ( ( url.contains( QLatin1Char('\n') ) || url.contains( QLatin1Char('\t') ) || url.contains( QLatin1Char(' ') ) ) &&
           left.isEmpty() ) {
        continue;
      }

      QString test( left + start + url + right );
      LinkLocator ll( test, left.length() );
      QString gotUrl = ll.getUrl();

      // we want to have the url without whitespace
      url.remove( QLatin1Char(' ') );
      url.remove( QLatin1Char('\n') );
      url.remove( QLatin1Char('\t') );

      bool ok = ( gotUrl == ( start + url ) );
      //qDebug() << "check:" << (ok ? "OK" : "NOK") << test << "=>" << (start + url);
      if ( !ok ) {
        qDebug() << "got:" << gotUrl;
      }
      QVERIFY2( ok, qPrintable( gotUrl ) );
    }
  }

  // test max url length
  QString url = QLatin1String("http://www.kde.org/this/is/a_very_loooooong_url/test/test/test");
  {
    LinkLocator ll( url );
    ll.setMaxUrlLen( 10 );
    QVERIFY( ll.getUrl().isEmpty() );  // url too long
  }
  {
    LinkLocator ll( url );
    ll.setMaxUrlLen( url.length() - 1 );
    QVERIFY( ll.getUrl().isEmpty() );  // url too long
  }
  {
    LinkLocator ll( url );
    ll.setMaxUrlLen( url.length() );
    QVERIFY( ll.getUrl() == url );
  }
  {
    LinkLocator ll( url );
    ll.setMaxUrlLen( url.length() + 1 );
    QVERIFY( ll.getUrl() == url );
  }

  // mailto
  {
    QString addr = QLatin1String("mailto:test@kde.org");
    QString test( left + addr + right );
    LinkLocator ll( test, left.length() );

    QString gotUrl = ll.getUrl();

    bool ok = ( gotUrl == addr );
    //qDebug() << "check:" << (ok ? "OK" : "NOK") << test << "=>" << addr;
    if ( !ok ) {
      qDebug() << "got:" << gotUrl;
    }
    QVERIFY2( ok, qPrintable( gotUrl ) );
  }
}

void LinkLocatorTest::testHtmlConvert_data()
{
  QTest::addColumn<QString>( "plainText" );
  QTest::addColumn<int>( "flags" );
  QTest::addColumn<QString>( "htmlText" );

  // Linker error when using PreserveSpaces, therefore the hardcoded 0x01 or 0x09

  // Test preserving whitespace correctly
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
                      << "&nbsp;bla bla&nbsp;<br />\n&nbsp;bla bla a<br />\n"
                         "&nbsp;&nbsp;bla bla&nbsp;";

  // Test highlighting with *, / and _
  QTest::newRow( "" ) << "Ce paragraphe _contient_ des mots ou des _groupes de mots_ à mettre en"
                         " forme…" << 0x09 << "Ce paragraphe <u>_contient_</u> des mots ou des"
                         " <u>_groupes de mots_</u> à mettre en forme…";
  QTest::newRow( "punctation-bug" ) << "Ce texte *a l'air* de _fonctionner_, à condition"
                                       " d’utiliser le guillemet ASCII." << 0x09
                                       << "Ce texte <b>*a l'air*</b> de <u>_fonctionner_</u>, à"
                                          " condition d’utiliser le guillemet ASCII.";
  QTest::newRow( "punctation-bug" ) << "Un répertoire /est/ un *dossier* où on peut mettre des"
                                       " *fichiers*." << 0x09 << "Un répertoire <i>/est/</i> un"
                                       " <b>*dossier*</b> où on peut mettre des <b>*fichiers*</b>.";
  QTest::newRow( "punctation-bug" ) << "*BLA BLA BLA BLA*." << 0x09 << "<b>BLA BLA BLA BLA</b>.";
  QTest::newRow( "" ) << "Je vais tenter de repérer des faux positif*" << 0x09
                      << "Je vais tenter de repérer des faux positif*";
  QTest::newRow( "" ) << "*Ouais !* *Yes!*" << 0x09 << "<b>*Ouais !*</b> <b>*Yes!*</b>";
  QTest::newRow( "" ) << "the /etc/{rsyslog.d,syslog-ng.d}/package.rpmnew file" << 0x09
                      << "the /etc/{rsyslog.d,syslog-ng.d}/package.rpmnew file";

  // This test has problems with the encoding, apparently.
  //QTest::newRow( "" ) << "*Ça fait plaisir de pouvoir utiliser des lettres accentuées dans du"
  //                       " texte mis en forme*." << 0x09 << "<b>Ça fait plaisir de pouvoir"
  //                       " utiliser des lettres accentuées dans du texte mis en forme</b>.";

  // Bug reported by dfaure, the <hostname> would get lost
  QTest::newRow( "" ) << "KUrl url(\"http://strange<hostname>/\");" << ( 0x08 | 0x02 )
                      << "KUrl url(&quot;<a href=\"http://strange<hostname>/\">"
                         "http://strange&lt;hostname&gt;/</a>&quot;);";

  // Bug: 211128 - plain text emails should not replace ampersand & with &amp;
  QTest::newRow( "bug211128" ) << "https://green-site/?Ticket=85&Page=next" << 0x01
    << "<a href=\"https://green-site/?Ticket=85&Page=next\">"
       "https://green-site/?Ticket=85&amp;Page=next</a>";

  QTest::newRow( "dotBeforeEnd" ) << "Look at this file: www.example.com/example.h" << 0x01
                                  << "Look at this file: <a href=\"http://www.example.com/example.h\">"
                                     "www.example.com/example.h</a>";
  QTest::newRow( "dotInMiddle" ) << "Look at this file: www.example.com/.bashrc" << 0x01
                                 << "Look at this file: <a href=\"http://www.example.com/.bashrc\">"
                                     "www.example.com/.bashrc</a>";

  // A dot at the end of an URL is explicitly ignored
  QTest::newRow( "dotAtEnd" ) << "Look at this file: www.example.com/test.cpp." << 0x01
                              << "Look at this file: <a href=\"http://www.example.com/test.cpp\">"
                                 "www.example.com/test.cpp</a>.";

  // Bug 313719 - URL in parenthesis
  QTest::newRow("url-in-parenthesis-1") << "KDE (website http://www.kde.org)" << 0x01
                                        << "KDE (website <a href=\"http://www.kde.org\">http://www.kde.org</a>)";
  QTest::newRow("url-in-parenthesis-2") << "KDE website (http://www.kde.org)" << 0x01
                                        << "KDE website (<a href=\"http://www.kde.org\">http://www.kde.org</a>)";
  QTest::newRow("url-in-parenthesis-3") << "bla (http://www.kde.org - section 5.2)" << 0x01
                                        << "bla (<a href=\"http://www.kde.org\">http://www.kde.org</a> - section 5.2)";
}

void LinkLocatorTest::testHtmlConvert()
{
  QFETCH( QString, plainText );
  QFETCH( int, flags );
  QFETCH( QString, htmlText );

  QEXPECT_FAIL( "punctation-bug", "Linklocator does not properly detect punctation as boundaries",
                Continue );

  QString actualHtml = LinkLocator::convertToHtml( plainText, flags );
  QCOMPARE( actualHtml, htmlText );
}
