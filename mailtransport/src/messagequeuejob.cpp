/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "messagequeuejob.h"

#include "transport.h"
#include "transportattribute.h"
#include "transportmanager.h"

#include <QDebug>
#include <KLocalizedString>

#include <collection.h>
#include <item.h>
#include <itemcreatejob.h>
#include <addressattribute.h>
#include <messageflags.h>
#include <specialmailcollections.h>
#include <specialmailcollectionsrequestjob.h>

using namespace Akonadi;
using namespace KMime;
using namespace MailTransport;

/**
  @internal
*/
class MailTransport::MessageQueueJob::Private
{
public:
    Private(MessageQueueJob *qq)
        : q(qq)
    {
        started = false;
    }

    MessageQueueJob *const q;

    Message::Ptr message;
    TransportAttribute transportAttribute;
    DispatchModeAttribute dispatchModeAttribute;
    SentBehaviourAttribute sentBehaviourAttribute;
    SentActionAttribute sentActionAttribute;
    AddressAttribute addressAttribute;
    bool started;

    /**
      Returns true if this message has everything it needs and is ready to be
      sent.
    */
    bool validate();

    // slot
    void outboxRequestResult(KJob *job);

};

bool MessageQueueJob::Private::validate()
{
    if (!message) {
        q->setError(UserDefinedError);
        q->setErrorText(i18n("Empty message."));
        q->emitResult();
        return false;
    }

    if (addressAttribute.to().count() + addressAttribute.cc().count() +
            addressAttribute.bcc().count() == 0) {
        q->setError(UserDefinedError);
        q->setErrorText(i18n("Message has no recipients."));
        q->emitResult();
        return false;
    }

    const int transport = transportAttribute.transportId();
    if (TransportManager::self()->transportById(transport, false) == 0) {
        q->setError(UserDefinedError);
        q->setErrorText(i18n("Message has invalid transport."));
        q->emitResult();
        return false;
    }

    if (sentBehaviourAttribute.sentBehaviour() == SentBehaviourAttribute::MoveToCollection &&
            !(sentBehaviourAttribute.moveToCollection().isValid())) {
        q->setError(UserDefinedError);
        q->setErrorText(i18n("Message has invalid sent-mail folder."));
        q->emitResult();
        return false;
    } else if (sentBehaviourAttribute.sentBehaviour() ==
               SentBehaviourAttribute::MoveToDefaultSentCollection) {
        // TODO require SpecialMailCollections::SentMail here?
    }

    return true; // all ok
}

void MessageQueueJob::Private::outboxRequestResult(KJob *job)
{
    Q_ASSERT(!started);
    started = true;

    if (job->error()) {
        qCritical() << "Failed to get the Outbox folder:" << job->error() << job->errorString();
        q->setError(job->error());
        q->emitResult();
        return;
    }

    if (!validate()) {
        // The error has been set; the result has been emitted.
        return;
    }

    SpecialMailCollectionsRequestJob *requestJob =
        qobject_cast<SpecialMailCollectionsRequestJob *>(job);
    if (!requestJob) {
        return;
    }

    // Create item.
    Item item;
    item.setMimeType(QStringLiteral("message/rfc822"));
    item.setPayload<Message::Ptr>(message);

    // Set attributes.
    item.addAttribute(addressAttribute.clone());
    item.addAttribute(dispatchModeAttribute.clone());
    item.addAttribute(sentBehaviourAttribute.clone());
    item.addAttribute(sentActionAttribute.clone());
    item.addAttribute(transportAttribute.clone());

    // Update status flags
    if (KMime::isSigned(message.get())) {
        item.setFlag(Akonadi::MessageFlags::Signed);
    }

    if (KMime::isEncrypted(message.get())) {
        item.setFlag(Akonadi::MessageFlags::Encrypted);
    }

    if (KMime::isInvitation(message.get())) {
        item.setFlag(Akonadi::MessageFlags::HasInvitation);
    }

    if (KMime::hasAttachment(message.get())) {
        item.setFlag(Akonadi::MessageFlags::HasAttachment);
    }

    // Set flags.
    item.setFlag(Akonadi::MessageFlags::Queued);

    // Store the item in the outbox.
    const Collection collection = requestJob->collection();
    Q_ASSERT(collection.isValid());
    ItemCreateJob *cjob = new ItemCreateJob(item, collection);   // job autostarts
    q->addSubjob(cjob);
}

MessageQueueJob::MessageQueueJob(QObject *parent)
    : KCompositeJob(parent), d(new Private(this))
{
}

MessageQueueJob::~MessageQueueJob()
{
    delete d;
}

Message::Ptr MessageQueueJob::message() const
{
    return d->message;
}

DispatchModeAttribute &MessageQueueJob::dispatchModeAttribute()
{
    return d->dispatchModeAttribute;
}

AddressAttribute &MessageQueueJob::addressAttribute()
{
    return d->addressAttribute;
}

TransportAttribute &MessageQueueJob::transportAttribute()
{
    return d->transportAttribute;
}

SentBehaviourAttribute &MessageQueueJob::sentBehaviourAttribute()
{
    return d->sentBehaviourAttribute;
}

SentActionAttribute &MessageQueueJob::sentActionAttribute()
{
    return d->sentActionAttribute;
}

void MessageQueueJob::setMessage(Message::Ptr message)
{
    d->message = message;
}

void MessageQueueJob::start()
{
    SpecialMailCollectionsRequestJob *rjob = new SpecialMailCollectionsRequestJob(this);
    rjob->requestDefaultCollection(SpecialMailCollections::Outbox);
    connect(rjob, SIGNAL(result(KJob*)), this, SLOT(outboxRequestResult(KJob*)));
    rjob->start();
}

void MessageQueueJob::slotResult(KJob *job)
{
    // error handling
    KCompositeJob::slotResult(job);

    if (!error()) {
        emitResult();
    }
}

#include "moc_messagequeuejob.cpp"
