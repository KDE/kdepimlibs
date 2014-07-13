/*
  Copyright (C) 2009 Bertjan Broeksema <broeksema@kde.org>

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


#include "mboxtest.h"

#include <QtCore/QDir>
#include <QtCore/QFile>

#include <qtest.h>
#include <QStandardPaths>
#include <QTemporaryDir>

QTEST_MAIN( MboxTest )

#include "test-entries.h"

using namespace KMBox;

static const char * testDir = "libmbox-unit-test";
static const char * testFile = "test-mbox-file";
static const char * testLockFile = "test-mbox-lock-file";

QString MboxTest::fileName()
{
  return mTempDir->path() + QLatin1Char('/') + QLatin1String( testFile );
}

QString MboxTest::lockFileName()
{
  return mTempDir->path() + QLatin1Char('/') + QLatin1String( testLockFile );
}

void MboxTest::removeTestFile()
{
  QFile file( fileName() );
  file.remove();
  QVERIFY( !file.exists() );
}

void MboxTest::initTestCase()
{
  mTempDir = new QTemporaryDir( QDir::tempPath() + QLatin1Char('/') + QLatin1String( testDir ) );

  QDir temp( mTempDir->path() );
  QVERIFY( temp.exists() );

  QFile mboxfile( fileName() );
  mboxfile.open( QIODevice::WriteOnly );
  mboxfile.close();
  QVERIFY( mboxfile.exists() );

  mMail1 = KMime::Message::Ptr( new KMime::Message );
  mMail1->setContent( KMime::CRLFtoLF( sEntry1 ) );
  mMail1->parse();

  mMail2 = KMime::Message::Ptr( new KMime::Message );
  mMail2->setContent( KMime::CRLFtoLF( sEntry2 ) );
  mMail2->parse();
}

void MboxTest::testSetLockMethod()
{
  MBox mbox1;

  if ( !QStandardPaths::findExecutable( QLatin1String( "lockfile" ) ).isEmpty() ) {
    QVERIFY( mbox1.setLockType( MBox::ProcmailLockfile ) );
  } else {
    QVERIFY( !mbox1.setLockType( MBox::ProcmailLockfile ) );
  }

  if ( !QStandardPaths::findExecutable( QLatin1String( "mutt_dotlock" ) ).isEmpty() ) {
    QVERIFY( mbox1.setLockType( MBox::MuttDotlock ) );
    QVERIFY( mbox1.setLockType( MBox::MuttDotlockPrivileged ) );
  } else {
    QVERIFY( !mbox1.setLockType( MBox::MuttDotlock ) );
    QVERIFY( !mbox1.setLockType( MBox::MuttDotlockPrivileged ) );
  }

  QVERIFY( mbox1.setLockType( MBox::None ) );
}

void MboxTest::testLockBeforeLoad()
{
  // Should fail because it's not known which file to lock.
  MBox mbox;

  if ( !QStandardPaths::findExecutable( QLatin1String( "lockfile" ) ).isEmpty() ) {
    QVERIFY( mbox.setLockType( MBox::ProcmailLockfile ) );
    QVERIFY( !mbox.lock() );
  }

  if ( !QStandardPaths::findExecutable( QLatin1String( "mutt_dotlock" ) ).isEmpty() ) {
    QVERIFY( mbox.setLockType( MBox::MuttDotlock ) );
    QVERIFY( !mbox.lock() );
    QVERIFY( mbox.setLockType( MBox::MuttDotlockPrivileged ) );
    QVERIFY( !mbox.lock() );
  }

  QVERIFY( mbox.setLockType( MBox::None ) );
  QVERIFY( !mbox.lock() );
}

void MboxTest::testProcMailLock()
{
  // It really only makes sense to test this if the lockfile executable can be
  // found.

  MBox mbox;
  if ( !mbox.setLockType( MBox::ProcmailLockfile ) ) {
    QEXPECT_FAIL( "", "This test only works when procmail is installed.", Abort );
    QVERIFY( false );
  }

  QVERIFY( mbox.load( fileName() ) );

  // By default the filename is used as part of the lockfile filename.
  QVERIFY( !QFile( fileName() + QLatin1String( ".lock" ) ).exists() );
  QVERIFY( mbox.lock() );
  QVERIFY( QFile( fileName() + QLatin1String( ".lock" ) ).exists() );
  QVERIFY( mbox.unlock() );
  QVERIFY( !QFile( fileName() + QLatin1String( ".lock" ) ).exists() );

  mbox.setLockFile( lockFileName() );
  QVERIFY( !QFile( lockFileName() ).exists() );
  QVERIFY( mbox.lock() );
  QVERIFY( QFile( lockFileName() ).exists() );
  QVERIFY( mbox.unlock() );
  QVERIFY( !QFile( lockFileName() ).exists() );
}

void MboxTest::testConcurrentAccess()
{
  // tests if mbox works correctly when another program locks the file
  // and appends new messages

  MBox mbox;

  if ( !mbox.setLockType( MBox::ProcmailLockfile ) ) {
    QEXPECT_FAIL( "", "This test only works when procmail is installed.", Abort );
    QVERIFY( false );
  }

  ThreadFillsMBox thread( fileName() );  // locks the mbox file and adds a new message
  thread.start();

  QVERIFY( mbox.load( fileName() ) );

  MBoxEntry entry = mbox.appendMessage( mMail1 );

  // as the thread appended sEntry1, the offset for the now appended message
  // must be greater
  QVERIFY( static_cast<int>(entry.messageOffset()) > sEntry1.length() );

  thread.wait();
}

void MboxTest::testAppend()
{
  removeTestFile();

  QFileInfo info( fileName() );
  QCOMPARE( info.size(), static_cast<qint64>( 0 ) );

  MBox mbox;
  mbox.setLockType( MBox::None );

  QVERIFY( mbox.load( fileName() ) );

  // First message added to an emtpy file should be at offset 0
  QCOMPARE( mbox.entries().size(), 0 );
  QCOMPARE( mbox.appendMessage( mMail1 ).messageOffset(), static_cast<quint64>( 0 ) );
  QCOMPARE( mbox.entries().size(), 1 );
  QVERIFY( mbox.entries().first().separatorSize() > 0 );
  QCOMPARE( mbox.entries().first().messageSize(), static_cast<quint64>( sEntry1.size() ) );

  const MBoxEntry offsetMail2 = mbox.appendMessage( mMail2 );
  QVERIFY( offsetMail2.messageOffset() > static_cast<quint64>( sEntry1.size() ) );
  QCOMPARE( mbox.entries().size(), 2 );
  QVERIFY( mbox.entries().last().separatorSize() > 0 );
  QCOMPARE( mbox.entries().last().messageSize(), static_cast<quint64>( sEntry2.size() ) );

  // check if appended entries can be read
  MBoxEntry::List list = mbox.entries();
  foreach ( const MBoxEntry &msgInfo, list ) {
    const QByteArray header = mbox.readMessageHeaders( msgInfo );
    QVERIFY( !header.isEmpty() );

    KMime::Message *message = mbox.readMessage( msgInfo );
    QVERIFY( message != 0 );

    KMime::Message *headers = new KMime::Message();
    headers->setHead( KMime::CRLFtoLF( header ) );
    headers->parse();

    QCOMPARE( message->messageID()->identifier(), headers->messageID()->identifier() );
    QCOMPARE( message->subject()->as7BitString(), headers->subject()->as7BitString() );
    QCOMPARE( message->to()->as7BitString(), headers->to()->as7BitString() );
    QCOMPARE( message->from()->as7BitString(), headers->from()->as7BitString() );

    if ( msgInfo.messageOffset() == 0 ) {
      QCOMPARE( message->messageID()->identifier(), mMail1->messageID()->identifier() );
      QCOMPARE( message->subject()->as7BitString(), mMail1->subject()->as7BitString() );
      QCOMPARE( message->to()->as7BitString(), mMail1->to()->as7BitString() );
      QCOMPARE( message->from()->as7BitString(), mMail1->from()->as7BitString() );
    } else if ( msgInfo.messageOffset() == offsetMail2.messageOffset() ) {
      QCOMPARE( message->messageID()->identifier(), mMail2->messageID()->identifier() );
      QCOMPARE( message->subject()->as7BitString(), mMail2->subject()->as7BitString() );
      QCOMPARE( message->to()->as7BitString(), mMail2->to()->as7BitString() );
      QCOMPARE( message->from()->as7BitString(), mMail2->from()->as7BitString() );
    }

    delete message;
    delete headers;
  }
}

void MboxTest::testSaveAndLoad()
{
  removeTestFile();

  MBox mbox;
  QVERIFY( mbox.setLockType( MBox::None ) );
  QVERIFY( mbox.load( fileName() ) );
  QVERIFY( mbox.entries().isEmpty() );
  mbox.appendMessage( mMail1 );
  mbox.appendMessage( mMail2 );

  MBoxEntry::List infos1 = mbox.entries();
  QCOMPARE( infos1.size(), 2 );

  QVERIFY( mbox.save() );
  QVERIFY( QFileInfo( fileName() ).exists() );

  MBoxEntry::List infos2 = mbox.entries();
  QCOMPARE( infos2.size(), 2 );

  for ( int i = 0; i < 2; ++i ) {
    QCOMPARE( infos1.at( i ).messageOffset(), infos2.at( i ).messageOffset() );
    QCOMPARE( infos1.at( i ).separatorSize(), infos2.at( i ).separatorSize() );
    QCOMPARE( infos1.at( i ).messageSize(), infos2.at( i ).messageSize() );
  }

  MBox mbox2;
  QVERIFY( mbox2.setLockType( MBox::None ) );
  QVERIFY( mbox2.load( fileName() ) );

  MBoxEntry::List infos3 = mbox2.entries();
  QCOMPARE( infos3.size(), 2 );

  for ( int i = 0; i < 2; ++i ) {
    QCOMPARE( infos3.at( i ), infos2.at( i ) );

    QCOMPARE( infos3.at( i ).messageOffset(), infos1.at( i ).messageOffset() );
    QCOMPARE( infos3.at( i ).separatorSize(), infos1.at( i ).separatorSize() );
    QCOMPARE( infos3.at( i ).messageSize(), infos1.at( i ).messageSize() );

    quint64 minSize = infos2.at( i ).messageSize();
    quint64 maxSize = infos2.at( i ).messageSize() + 1;
    QVERIFY( infos3.at( i ).messageSize() >= minSize  );
    QVERIFY( infos3.at( i ).messageSize() <= maxSize  );
  }
}

void MboxTest::testBlankLines()
{
  for ( int i = 0; i < 5; ++i ) {
    removeTestFile();

    KMime::Message::Ptr mail = KMime::Message::Ptr( new KMime::Message );
    mail->setContent( KMime::CRLFtoLF( sEntry1 + QByteArray( i, '\n' ) ) );
    mail->parse();

    MBox mbox1;
    QVERIFY( mbox1.setLockType( MBox::None ) );
    QVERIFY( mbox1.load( fileName() ) );
    mbox1.appendMessage( mail );
    mbox1.appendMessage( mail );
    mbox1.appendMessage( mail );
    mbox1.save();

    MBox mbox2;
    QVERIFY( mbox1.setLockType( MBox::None ) );
    QVERIFY( mbox1.load( fileName() ) );
    QCOMPARE( mbox1.entries().size(), 3 );

    quint64 minSize = sEntry1.size() + i - 1; // Possibly on '\n' falls off.
    quint64 maxSize = sEntry1.size() + i;
    for ( int i = 0; i < 3; ++i ) {
      QVERIFY( mbox1.entries().at( i ).messageSize() >= minSize  );
      QVERIFY( mbox1.entries().at( i ).messageSize() <= maxSize  );
    }
  }
}

void MboxTest::testEntries()
{
  removeTestFile();

  MBox mbox1;
  QVERIFY( mbox1.setLockType( MBox::None ) );
  QVERIFY( mbox1.load( fileName() ) );
  mbox1.appendMessage( mMail1 );
  mbox1.appendMessage( mMail2 );
  mbox1.appendMessage( mMail1 );

  MBoxEntry::List infos = mbox1.entries();
  QCOMPARE( infos.size() , 3 );

  MBoxEntry::List deletedEntries;
  deletedEntries << infos.at( 0 );

  MBoxEntry::List infos2 = mbox1.entries( deletedEntries );
  QCOMPARE( infos2.size() , 2 );
  QVERIFY( infos2.first() != infos.first() );
  QVERIFY( infos2.last() != infos.first() );

  deletedEntries << infos.at( 1 );
  infos2 = mbox1.entries( deletedEntries );

  QCOMPARE( infos2.size() , 1 );
  QVERIFY( infos2.first() != infos.at( 0 ) );
  QVERIFY( infos2.first() != infos.at( 1 ) );

  deletedEntries << infos.at( 2 );
  infos2 = mbox1.entries( deletedEntries );
  QCOMPARE( infos2.size() , 0 );

  QVERIFY( !deletedEntries.contains( MBoxEntry( 10 ) ) ); // some random offset
  infos2 = mbox1.entries( MBoxEntry::List() << MBoxEntry( 10 ) );
  QCOMPARE( infos2.size() , 3 );
  QCOMPARE( infos2.at( 0 ), infos.at( 0 ) );
  QCOMPARE( infos2.at( 1 ), infos.at( 1 ) );
  QCOMPARE( infos2.at( 2 ), infos.at( 2 ) );
}

void MboxTest::testPurge()
{
  MBox mbox1;
  QVERIFY( mbox1.setLockType( MBox::None ) );
  QVERIFY( mbox1.load( fileName() ) );
  mbox1.appendMessage( mMail1 );
  mbox1.appendMessage( mMail1 );
  mbox1.appendMessage( mMail1 );
  QVERIFY( mbox1.save() );

  MBoxEntry::List list = mbox1.entries();

  // First test: Delete only the first (all messages afterwards have to be moved).
  mbox1.purge( MBoxEntry::List() << list.first() );

  MBox mbox2;
  QVERIFY( mbox2.load( fileName() ) );
  MBoxEntry::List list2 = mbox2.entries();
  QCOMPARE( list2.size(), 2 ); // Is a message actually gone?

  quint64 newOffsetSecondMessage = list.last().messageOffset() - list.at( 1 ).messageOffset();

  QCOMPARE( list2.first().messageOffset(), static_cast<quint64>( 0 ) );
  QCOMPARE( list2.last().messageOffset(), newOffsetSecondMessage );

  // Second test: Delete the first two (the last message have to be moved).
  removeTestFile();

  QVERIFY( mbox1.load( fileName() ) );
  mbox1.appendMessage( mMail1 );
  mbox1.appendMessage( mMail1 );
  mbox1.appendMessage( mMail1 );
  QVERIFY( mbox1.save() );

  list = mbox1.entries();

  mbox1.purge( MBoxEntry::List() << list.at( 0 ) << list.at( 1 ) );
  QVERIFY( mbox2.load( fileName() ) );
  list2 = mbox2.entries();
  QCOMPARE( list2.size(), 1 ); // Are the messages actually gone?
  QCOMPARE( list2.first().messageOffset(), static_cast<quint64>( 0 ) );

  // Third test: Delete all messages.
  removeTestFile();

  QVERIFY( mbox1.load( fileName() ) );
  mbox1.appendMessage( mMail1 );
  mbox1.appendMessage( mMail1 );
  mbox1.appendMessage( mMail1 );
  QVERIFY( mbox1.save() );

  list = mbox1.entries();

  mbox1.purge( MBoxEntry::List() << list.at( 0 ) << list.at( 1 ) << list.at( 2 ) );
  QVERIFY( mbox2.load( fileName() ) );
  list2 = mbox2.entries();
  QCOMPARE( list2.size(), 0 ); // Are the messages actually gone?
}

void MboxTest::testLockTimeout()
{
  MBox mbox;
  mbox.load( fileName() );
  mbox.setLockType( MBox::None );
  mbox.setUnlockTimeout( 1000 );

  QVERIFY( !mbox.locked() );
  mbox.lock();
  QVERIFY( mbox.locked() );

  QTest::qWait( 1010 );
  QVERIFY( !mbox.locked() );
}

void MboxTest::testHeaders()
{
  MBox mbox;
  QVERIFY( mbox.setLockType( MBox::None ) );
  QVERIFY( mbox.load( fileName() ) );
  mbox.appendMessage( mMail1 );
  mbox.appendMessage( mMail2 );
  QVERIFY( mbox.save() );

  const MBoxEntry::List list = mbox.entries();

  foreach ( const MBoxEntry &msgInfo, list ) {
    const QByteArray header = mbox.readMessageHeaders( msgInfo );
    QVERIFY( !header.isEmpty() );

    KMime::Message *message = mbox.readMessage( msgInfo );
    QVERIFY( message != 0 );

    KMime::Message *headers = new KMime::Message();
    headers->setHead( KMime::CRLFtoLF( header ) );
    headers->parse();

    QCOMPARE( message->messageID()->identifier(), headers->messageID()->identifier() );
    QCOMPARE( message->subject()->as7BitString(), headers->subject()->as7BitString() );
    QCOMPARE( message->to()->as7BitString(), headers->to()->as7BitString() );
    QCOMPARE( message->from()->as7BitString(), headers->from()->as7BitString() );

    delete message;
    delete headers;
  }
}

void MboxTest::cleanupTestCase()
{
  mTempDir->remove();
}

//---------------------------------------------------------------------

ThreadFillsMBox::ThreadFillsMBox( const QString &fileName )
{
  mbox = new MBox;
  QVERIFY( mbox->load( fileName ) );
  mbox->setLockType( MBox::ProcmailLockfile );
  mbox->lock();
}

void ThreadFillsMBox::run()
{
  QTest::qSleep( 2000 );

  QFile file( mbox->fileName() );
  file.open( QIODevice::WriteOnly | QIODevice::Append );

  QByteArray message = KMime::CRLFtoLF( sEntry1 );
  file.write(QByteArray("From test@local.local ") +
                QDateTime::currentDateTime().toString(Qt::ISODate).toUtf8() + "\n");
  file.write( message );
  file.write( "\n\n" );
  file.close();

  mbox->unlock();
  delete mbox;
}
