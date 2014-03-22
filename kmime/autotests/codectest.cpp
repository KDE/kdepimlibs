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
Q_DECLARE_METATYPE( Mode )

void CodecTest::testCodecs_data()
{
  QTest::addColumn<QByteArray>( "input" );
  QTest::addColumn<QByteArray>( "expResult" );
  QTest::addColumn<QByteArray>( "codecName" );
  QTest::addColumn<QString>( "tag" );
  QTest::addColumn<Mode>( "mode" );

  QDir codecBaseDir( TEST_DATA_DIR );
  foreach ( const QString &dir, codecBaseDir.entryList( QStringList(), QDir::Dirs | QDir::NoDotAndDotDot,
                                                        QDir::NoSort ) ) {
    if ( dir.toLower().startsWith( "codec_" ) ) {
      const QString codecName = dir.right( dir.size() - 6 );
      QDir codecDir( TEST_DATA_DIR"/" + dir );
      foreach ( const QString &file, codecDir.entryList( QStringList(), QDir::Files, QDir::NoSort ) ) {
        if ( file.toLower().endsWith( ".expected" ) ) {
          const QString dataFileNameBase = file.left( file.size() - 9 );
          QFile dataFile( codecDir.path() + '/' + dataFileNameBase );
          QFile expectedFile( codecDir.path() + '/' + file );
          QVERIFY( dataFile.open( QIODevice::ReadOnly ) );
          QVERIFY( expectedFile.open( QIODevice::ReadOnly ) );

          Mode mode = Decode;
          if ( file.contains( "-decode" ) ) {
            mode = Decode;
          } else if ( file.contains( "-encode" ) ) {
            mode = Encode;
          }

          const QByteArray data = dataFile.readAll();
          const QByteArray expected = expectedFile.readAll();

          const QString tag = codecName + '/' + dataFileNameBase;
          QTest::newRow( tag.toLatin1() ) << data << expected << codecName.toAscii() << tag  << mode;

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

  QStringList blacklistedTags;
  if ( blacklistedTags.contains( tag ) ) {
    QEXPECT_FAIL( tag.toLatin1(), "Codec broken", Continue );
  }

  QByteArray result;
  if ( mode == Decode ) {
    result = codec->decode( input, false );
  }
  else
    result = codec->encode( input, false );

  QCOMPARE( result, expResult );
}

