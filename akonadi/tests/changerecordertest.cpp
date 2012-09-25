/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include <akonadi/changerecorder.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/agentmanager.h>

#include <QtCore/QObject>
#include <QtCore/QSettings>

#include <qtest_akonadi.h>

using namespace Akonadi;

Q_DECLARE_METATYPE(QSet<QByteArray>)

class ChangeRecorderTest : public QObject
{
  Q_OBJECT
  private:
    void triggerChange( int uid )
    {
      Item item( uid );
      item.setFlag( "random_flag" );
      ItemModifyJob *job = new ItemModifyJob( item );
      job->disableRevisionCheck();
      AKVERIFYEXEC( job );
      item.clearFlag( "random_flag" );
      job = new ItemModifyJob( item );
      job->disableRevisionCheck();
      AKVERIFYEXEC( job );
    }

  private slots:
    void initTestCase()
    {
      qRegisterMetaType<Akonadi::Item>();
      qRegisterMetaType<QSet<QByteArray> >();

      // switch all resources offline to reduce interference from them
      foreach ( Akonadi::AgentInstance agent, Akonadi::AgentManager::self()->instances() ) //krazy:exclude=foreach
        agent.setIsOnline( false );
    }

    void testChangeRecorder()
    {
      QSettings *settings = new QSettings( "kde.org", "akonadi-changerecordertest", this );
      settings->clear();

      ChangeRecorder *rec = new ChangeRecorder();
      rec->setConfig( settings );
      rec->setAllMonitored();

      QSignalSpy spy( rec, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)) );
      QVERIFY( spy.isValid() );
      QSignalSpy cspy( rec, SIGNAL(changesAdded()) );
      QVERIFY( cspy.isValid() );

      triggerChange( 1 );
      triggerChange( 1 );
      triggerChange( 3 );
      QTest::qWait( 500 ); // enter event loop and wait for change notifications from the server

      QCOMPARE( spy.count(), 0 );
      QVERIFY( !cspy.isEmpty() );
      delete rec;

      rec = new ChangeRecorder();
      rec->setConfig( settings );
      rec->setAllMonitored();
      rec->itemFetchScope().fetchFullPayload();
      rec->itemFetchScope().fetchAllAttributes();
      QVERIFY( !rec->isEmpty() );

      QSignalSpy spy2( rec, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)) );
      QVERIFY( spy2.isValid() );
      rec->replayNext();
      QTest::kWaitForSignal( rec, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), 1000 );
      QCOMPARE( spy2.count(), 1 );
      rec->changeProcessed();
      QVERIFY( !rec->isEmpty() );
      rec->replayNext();
      QTest::kWaitForSignal( rec, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), 1000 );
      QCOMPARE( spy2.count(), 2 );
      rec->changeProcessed();
      QVERIFY( rec->isEmpty() );

      // nothing changes here
      rec->replayNext();
      QTest::kWaitForSignal( rec, SIGNAL(nothingToReplay()), 1000 );
      rec->changeProcessed();
      QVERIFY( rec->isEmpty() );
      QCOMPARE( spy2.count(), 2 );
      delete rec;
    }

    void testEmptyChangeReplay()
    {
      ChangeRecorder recorder;
      recorder.setAllMonitored();
      recorder.itemFetchScope().fetchFullPayload();
      recorder.itemFetchScope().fetchAllAttributes();
      QSignalSpy nothingSpy( &recorder, SIGNAL(nothingToReplay()) );
      QSignalSpy changedSpy( &recorder, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)) );
      QVERIFY( nothingSpy.isValid() );
      QVERIFY( changedSpy.isValid() );

      // Nothing to replay, should emit that signal then.
      recorder.replayNext();
      QTest::kWaitForSignal( &recorder, SIGNAL(nothingToReplay()), 1000 );
      QCOMPARE( nothingSpy.count(), 1 );
      QCOMPARE( changedSpy.count(), 0 );

      // Give it something to replay
      triggerChange( 2 );
      QTest::kWaitForSignal( &recorder, SIGNAL(changesAdded()), 1000 );
      recorder.replayNext();
      QTest::kWaitForSignal( &recorder, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), 1000 );
      QCOMPARE( nothingSpy.count(), 1 );
      QCOMPARE( changedSpy.count(), 1 );

      // Nothing else to replay now
      recorder.changeProcessed();
      recorder.replayNext();
      QTest::kWaitForSignal( &recorder, SIGNAL(nothingToReplay()), 1000 );
      QCOMPARE( nothingSpy.count(), 2 );
      QCOMPARE( changedSpy.count(), 1 );
    };
};

QTEST_AKONADIMAIN( ChangeRecorderTest, NoGUI )

#include "changerecordertest.moc"
