/*
    Copyright 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "messagequeuejobtest.h"

#include <QStringList>

#include <agentinstance.h>
#include <agentmanager.h>
#include <collection.h>
#include <collectionstatistics.h>
#include <collectionstatisticsjob.h>
#include <control.h>
#include <itemfetchjob.h>
#include <itemfetchscope.h>
#include <itemdeletejob.h>
#include <qtest_akonadi.h>
#include <addressattribute.h>
#include <messageflags.h>
#include <specialmailcollections.h>
#include <specialmailcollectionsrequestjob.h>

#include <kmime_message.h>
#include <boost/shared_ptr.hpp>

#include <dispatchmodeattribute.h>
#include <errorattribute.h>
#include <messagequeuejob.h>
#include <sentbehaviourattribute.h>
#include <transport.h>
#include <transportattribute.h>
#include <transportmanager.h>

#define SPAM_ADDRESS ( QStringList() << "idanoka@gmail.com" )

using namespace Akonadi;
using namespace KMime;
using namespace MailTransport;

void MessageQueueJobTest::initTestCase()
{
    Control::start();
    // HACK: Otherwise the MDA is not switched offline soon enough apparently...
    QTest::qWait(1000);

    // Switch MDA offline to avoid spam.
    AgentInstance mda = AgentManager::self()->instance("akonadi_maildispatcher_agent");
    QVERIFY(mda.isValid());
    mda.setIsOnline(false);

    // check that outbox is empty
    SpecialMailCollectionsRequestJob *rjob = new SpecialMailCollectionsRequestJob(this);
    rjob->requestDefaultCollection(SpecialMailCollections::Outbox);
    QTest::kWaitForSignal(rjob, SIGNAL(result(KJob*)));
    verifyOutboxContents(0);
}

void MessageQueueJobTest::testValidMessages()
{
    // check transport
    int tid = TransportManager::self()->defaultTransportId();
    QVERIFY2(tid >= 0, "I need a default transport, but there is none.");

    // send a valid message using the default transport
    MessageQueueJob *qjob = new MessageQueueJob;
    qjob->transportAttribute().setTransportId(tid);
    Message::Ptr msg = Message::Ptr(new Message);
    msg->setContent("\nThis is message #1 from the MessageQueueJobTest unit test.\n");
    qjob->setMessage(msg);
    qjob->addressAttribute().setTo(SPAM_ADDRESS);
    verifyOutboxContents(0);
    AKVERIFYEXEC(qjob);

    // fetch the message and verify it
    QTest::qWait(1000);
    verifyOutboxContents(1);
    ItemFetchJob *fjob = new ItemFetchJob(
        SpecialMailCollections::self()->defaultCollection(SpecialMailCollections::Outbox));
    fjob->fetchScope().fetchFullPayload();
    fjob->fetchScope().fetchAllAttributes();
    AKVERIFYEXEC(fjob);
    QCOMPARE(fjob->items().count(), 1);
    Item item = fjob->items().first();
    QVERIFY(!item.remoteId().isEmpty());   // stored by the resource
    QVERIFY(item.hasPayload<Message::Ptr>());
    AddressAttribute *addrA = item.attribute<AddressAttribute>();
    QVERIFY(addrA);
    QVERIFY(addrA->from().isEmpty());
    QCOMPARE(addrA->to().count(), 1);
    QCOMPARE(addrA->to(), SPAM_ADDRESS);
    QCOMPARE(addrA->cc().count(), 0);
    QCOMPARE(addrA->bcc().count(), 0);
    DispatchModeAttribute *dA = item.attribute<DispatchModeAttribute>();
    QVERIFY(dA);
    QCOMPARE(dA->dispatchMode(), DispatchModeAttribute::Automatic);   // default mode
    SentBehaviourAttribute *sA = item.attribute<SentBehaviourAttribute>();
    QVERIFY(sA);
    // default sent collection
    QCOMPARE(sA->sentBehaviour(), SentBehaviourAttribute::MoveToDefaultSentCollection);
    TransportAttribute *tA = item.attribute<TransportAttribute>();
    QVERIFY(tA);
    QCOMPARE(tA->transportId(), tid);
    ErrorAttribute *eA = item.attribute<ErrorAttribute>();
    QVERIFY(!eA);   // no error
    QCOMPARE(item.flags().count(), 1);
    QVERIFY(item.flags().contains(Akonadi::MessageFlags::Queued));

    // delete message, for further tests
    ItemDeleteJob *djob = new ItemDeleteJob(item);
    AKVERIFYEXEC(djob);
    verifyOutboxContents(0);

    // TODO test with no To: but only BCC:

    // TODO test due-date sending

    // TODO test sending with custom sent-mail collections
}

void MessageQueueJobTest::testInvalidMessages()
{
    MessageQueueJob *job = 0;
    Message::Ptr msg;

    // without message
    job = new MessageQueueJob;
    job->transportAttribute().setTransportId(TransportManager::self()->defaultTransportId());
    job->addressAttribute().setTo(SPAM_ADDRESS);
    QVERIFY(!job->exec());

    // without recipients
    job = new MessageQueueJob;
    msg = Message::Ptr(new Message);
    msg->setContent("\nThis is a message sent from the MessageQueueJobTest unittest."
                    " This shouldn't have been sent.\n");
    job->setMessage(msg);
    job->transportAttribute().setTransportId(TransportManager::self()->defaultTransportId());
    QVERIFY(!job->exec());

    // without transport
    job = new MessageQueueJob;
    msg = Message::Ptr(new Message);
    msg->setContent("\nThis is a message sent from the MessageQueueJobTest unittest."
                    " This shouldn't have been sent.\n");
    job->setMessage(msg);
    job->addressAttribute().setTo(SPAM_ADDRESS);
    QVERIFY(!job->exec());

    // with MoveToCollection and no sent-mail folder
    job = new MessageQueueJob;
    msg = Message::Ptr(new Message);
    msg->setContent("\nThis is a message sent from the MessageQueueJobTest unittest."
                    " This shouldn't have been sent.\n");
    job->setMessage(msg);
    job->addressAttribute().setTo(SPAM_ADDRESS);
    job->sentBehaviourAttribute().setSentBehaviour(SentBehaviourAttribute::MoveToCollection);
    QVERIFY(!job->exec());
}

void MessageQueueJobTest::verifyOutboxContents(qlonglong count)
{
    QVERIFY(SpecialMailCollections::self()->hasDefaultCollection(SpecialMailCollections::Outbox));
    Collection outbox =
        SpecialMailCollections::self()->defaultCollection(SpecialMailCollections::Outbox);
    QVERIFY(outbox.isValid());
    CollectionStatisticsJob *job = new CollectionStatisticsJob(outbox);
    AKVERIFYEXEC(job);
    QCOMPARE(job->statistics().count(), count);
}

QTEST_AKONADIMAIN(MessageQueueJobTest, NoGUI)

