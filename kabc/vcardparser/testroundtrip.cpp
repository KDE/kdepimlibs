/*
    This file is part of libkabc.
    Copyright (c) 2012 Kevin Krammer <krammer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include "addressee.h"
#include "vcardconverter.h"

#include <qtest_kde.h>

#include <QObject>
#include <QString>
#include <QByteArray>

using namespace KABC;

class RoundtripTest : public QObject
{
  Q_OBJECT

  private:
    QString mOutFilePattern;

    QDir mInputDir;
    QDir mOutput2_1Dir;
    QDir mOutput3_0Dir;
    QDir mOutput4_0Dir;

    QStringList mInputFiles;

  private Q_SLOTS:
    void initTestCase();
    void testVCardRoundtrip_data();
    void testVCardRoundtrip();

	private:
		void validate( VCardConverter::Version version,
                   const QDir& outputDir,
                   const QString& outputFileName,
                   const Addressee::List list );
};

// check the validity of our test data set
void RoundtripTest::initTestCase()
{
  mOutFilePattern = QLatin1String( "%1.ref" );

  // check that all resource prefixes exist

  mInputDir = QDir( QLatin1String( ":/input" ) );
  QVERIFY( mInputDir.exists() );
  QVERIFY( mInputDir.cd( QLatin1String( "tests" ) ) );

  mOutput2_1Dir = QDir( QLatin1String( ":/output2.1" ) );
  QVERIFY( mOutput2_1Dir.exists() );
  QVERIFY( mOutput2_1Dir.cd( QLatin1String( "tests" ) ) );

  mOutput3_0Dir = QDir( QLatin1String( ":/output3.0" ) );
  QVERIFY( mOutput3_0Dir.exists() );
  QVERIFY( mOutput3_0Dir.cd( QLatin1String( "tests" ) ) );

  mOutput4_0Dir = QDir( QLatin1String( ":/output4.0" ) );
  QVERIFY( mOutput4_0Dir.exists() );
  QVERIFY( mOutput4_0Dir.cd( QLatin1String( "tests" ) ) );

  // check that there are input files

  mInputFiles = mInputDir.entryList();
  QVERIFY( !mInputFiles.isEmpty() );
}

void RoundtripTest::testVCardRoundtrip_data()
{
  QTest::addColumn<QString>( "inputFile" );
  QTest::addColumn<QString>( "output2_1File" );
  QTest::addColumn<QString>( "output3_0File" );
  QTest::addColumn<QString>( "output4_0File" );

  Q_FOREACH ( const QString &inputFile, mInputFiles ) {
    const QString outFile = mOutFilePattern.arg( inputFile );

    QTest::newRow( QFile::encodeName( inputFile ) )
      << inputFile
      << ( mOutput2_1Dir.exists( outFile ) ? outFile : QString() )
      << ( mOutput3_0Dir.exists( outFile ) ? outFile : QString() )
      << ( mOutput4_0Dir.exists( outFile ) ? outFile : QString() );
  }
}

void RoundtripTest::testVCardRoundtrip()
{
  QFETCH( QString, inputFile );
  QFETCH( QString, output2_1File );
  QFETCH( QString, output3_0File );
  QFETCH( QString, output4_0File );

  QVERIFY2( !output2_1File.isEmpty()
            || !output3_0File.isEmpty()
            || !output4_0File.isEmpty(),
            "No reference output file for either format version" );

  QFile input( QFileInfo( mInputDir, inputFile ).absoluteFilePath() );
  QVERIFY( input.open( QIODevice::ReadOnly ) );

  const QByteArray inputData = input.readAll();
  QVERIFY( !inputData.isEmpty() );

  VCardConverter converter;
  const Addressee::List list = converter.parseVCards( inputData );
  QVERIFY( !list.isEmpty() );

  validate( VCardConverter::v2_1, mOutput2_1Dir, output2_1File, list );
  validate( VCardConverter::v3_0, mOutput3_0Dir, output3_0File, list );
  validate( VCardConverter::v4_0, mOutput4_0Dir, output4_0File, list );
}

void RoundtripTest::validate( VCardConverter::Version version,
                              const QDir& outputDir,
                              const QString& outputFileName,
                              const Addressee::List list )
{
  if ( !outputFileName.isEmpty() ) {
    VCardConverter converter;
    const QByteArray outputData = converter.createVCards( list, version );

    /* FIX
     * Plain text files *.ref in the git repository must have Unix line endings. (CR)
     * However, the parser generates DOS line endings. (CR+LF) (according to RFC).
     * So we remove all '\r's from the generated output.  */
    QByteArray processedOutputData = outputData;
    int index = outputData.indexOf( '\r' );
    while ( index != -1 ) {
      processedOutputData.remove( index, 1 );
      index = processedOutputData.indexOf( '\r', index );
    }

    QCOMPARE( processedOutputData.size(), outputRefData.size() );

    const QList<QByteArray> outputLines = processedOutputData.split( '\n' );
    const QList<QByteArray> outputRefLines = outputRefData.split( '\n' );
    QCOMPARE( outputLines.count(), outputRefLines.count() );

    const QByteArray versionString( ( version == VCardConverter::v2_1 ) ? "2.1"
                                    : ( version == VCardConverter::v3_0 ) ? "3.0"
                                    : "4.0" );

    for ( int i = 0; i < outputLines.count(); ++i ) {
      const QByteArray actual = outputLines[ i ];
      const QByteArray expect = outputRefLines[ i ];

      if ( actual != expect ) {
        qCritical() << "Mismatch in v" << versionString << " output line" << ( i + 1 );
        qCritical() << "\nActual:" << actual << "\nExpect:" << expect;
        QCOMPARE( actual.count(), expect.count() );
        QCOMPARE( actual, expect );
      }
    }

    // Second line is VERSION:<version n°>
    QByteArray versionExpected = "VERSION:";
    versionExpected.append( versionString );
    QCOMPARE( outputLines[ 1 ], versionExpected );
  }

  if ( !output4_0File.isEmpty() ) {
    const QByteArray outputData = converter.createVCards( list, VCardConverter::v4_0 );
    QFile outputFile( QFileInfo( mOutput4_0Dir, output4_0File ).absoluteFilePath() );
    QVERIFY( outputFile.open( QIODevice::ReadOnly ) );

    const QByteArray outputRefData = outputFile.readAll();

    const QList<QByteArray> outputLines = processedOutputData.split( '\n' );
    const QList<QByteArray> outputRefLines = outputRefData.split( '\n' );
    QCOMPARE( outputLines.count(), outputRefLines.count() );

    const QByteArray versionString( ( version == VCardConverter::v2_1 ) ? "2.1"
                                    : ( version == VCardConverter::v3_0 ) ? "3.0"
                                    : "4.0" );

    for ( int i = 0; i < outputLines.count(); ++i ) {
      const QByteArray actual = outputLines[ i ];
      const QByteArray expect = outputRefLines[ i ];

      if ( actual != expect ) {
        qCritical() << "Mismatch in v" << versionString << " output line" << ( i + 1 );
        qCritical() << "\nActual:" << actual << "\nExpect:" << expect;
        QCOMPARE( actual.count(), expect.count() );
        QCOMPARE( actual, expect );
      }
    }

    // Second line is VERSION:<version n°>
    QByteArray versionExpected = "VERSION:";
    versionExpected.append( versionString );
    QCOMPARE( outputLines[ 1 ], versionExpected );
  }
}

QTEST_KDEMAIN( RoundtripTest, NoGUI )

#include "testroundtrip.moc"
