/*
    Copyright (c) 2010 Thomas McGuire <mcguire@kde.org>

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
#include "codectest.h"

#include <qtest_kde.h>

#include <kmime_charfreq.h>
#include <kmime_codecs.h>

#include <QDir>
using namespace KMime;

QTEST_KDEMAIN( CodecTest, NoGUI )

enum Mode { Decode, Encode };
Q_DECLARE_METATYPE( Mode );

void CodecTest::testCodecs_data()
{
  QTest::addColumn<QByteArray>( "input" );
  QTest::addColumn<QByteArray>( "expResult" );
  QTest::addColumn<QByteArray>( "codecName" );
  QTest::addColumn<QString>( "tag" );
  QTest::addColumn<Mode>( "mode" );

  QDir codecBaseDir( CODEC_DIR );
  foreach( const QString &dir, codecBaseDir.entryList( QStringList(), QDir::Dirs | QDir::NoDotAndDotDot,
                                                       QDir::NoSort ) ) {
    if ( dir.toLower().startsWith( "codec_" ) ) {
      const QString codecName = dir.right( dir.size() - 6 );
      QDir codecDir( CODEC_DIR"/" + dir );
      foreach( const QString &file, codecDir.entryList( QStringList(), QDir::Files, QDir::NoSort ) ) {
        if ( file.toLower().endsWith( ".expected" ) ) {
          const QString dataFileNameBase = file.left( file.size() - 9 );
          QFile dataFile( codecDir.path() + '/' + dataFileNameBase );
          QFile expectedFile( codecDir.path() + '/' + file );
          QVERIFY( dataFile.open( QIODevice::ReadOnly ) );
          QVERIFY( expectedFile.open( QIODevice::ReadOnly ) );

          Mode mode;
          if ( file.contains( "encode") )
            mode = Encode;
          else
            mode = Decode;

          const QByteArray data = dataFile.readAll();
          const QByteArray expected = expectedFile.readAll();

          const QString tag = codecName + '/' + dataFileNameBase;
          if ( tag != "x-uuencode/basic-decode.x-uuencode" ) // this one crashes
            QTest::newRow( tag.toAscii() ) << data << expected << codecName.toAscii() << tag  << mode;

          dataFile.close();
          expectedFile.close();
        }
      }
    }
  }
}

void CodecTest::testCodecs()
{
  QFETCH( QByteArray, input );
  QFETCH( QByteArray, expResult );
  QFETCH( QByteArray, codecName );
  QFETCH( QString, tag );
  QFETCH( Mode, mode );

  Codec * codec = Codec::codecForName( codecName );
  QVERIFY( codec );

  QByteArray result;
  if ( mode == Decode )
    result = codec->decode( input, false );
  else
    result = codec->encode( input, false );

  QStringList blacklistedTags;
  blacklistedTags << "x-uuencode/basic-decode.x-uuencode"
                  << "b/padding0"
                  << "b/padding1"
                  << "b/padding2"
                  << "base64/very_small"
                  << "q/all-encoded.q"
                  << "q/nothing-encoded.q"
                  << "quoted-printable/wrap"
                  << "x-kmime-rfc2231/all-encoded.x-kmime-rfc2231"
                  << "x-kmime-rfc2231/nothing-encoded.x-kmime-rfc2231";
  if ( blacklistedTags.contains( tag ) )
    QEXPECT_FAIL( tag.toAscii(), "Codec broken", Continue );

  QCOMPARE( result.data(), expResult.data() );
}

#include "codectest.moc"
