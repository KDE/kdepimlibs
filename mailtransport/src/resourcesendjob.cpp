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

#include "resourcesendjob_p.h"
#include "messagequeuejob.h"
#include "transport.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <kmime/kmime_message.h>

#include <collection.h>
#include <itemcreatejob.h>
#include <itemfetchjob.h>
#include <itemfetchscope.h>
#include <addressattribute.h>

using namespace Akonadi;
using namespace KMime;
using namespace MailTransport;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class MailTransport::ResourceSendJobPrivate
{
public:
    ResourceSendJobPrivate(ResourceSendJob *qq)
        : q(qq)
    {
    }

    void slotEmitResult(); // slot

    ResourceSendJob *const q;
};

void ResourceSendJobPrivate::slotEmitResult()
{
    // KCompositeJob took care of the error.
    q->emitResult();
}

ResourceSendJob::ResourceSendJob(Transport *transport, QObject *parent)
    : TransportJob(transport, parent), d(new ResourceSendJobPrivate(this))
{
}

ResourceSendJob::~ResourceSendJob()
{
    delete d;
}

void ResourceSendJob::doStart()
{
    Message::Ptr msg = Message::Ptr(new Message);
    msg->setContent(data());
    MessageQueueJob *job = new MessageQueueJob;
    job->setMessage(msg);
    job->transportAttribute().setTransportId(transport()->id());
    // Default dispatch mode (send now).
    // Move to default sent-mail collection.
    job->addressAttribute().setFrom(sender());
    job->addressAttribute().setTo(to());
    job->addressAttribute().setCc(cc());
    job->addressAttribute().setBcc(bcc());
    addSubjob(job);
    // Once the item is in the outbox, there is nothing more we can do.
    connect(job, SIGNAL(result(KJob *)), this, SLOT(slotEmitResult()));
    job->start();
}

#include "moc_resourcesendjob_p.cpp"
