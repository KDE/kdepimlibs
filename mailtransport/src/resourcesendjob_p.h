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

#ifndef MAILTRANSPORT_RESOURCESENDJOB_P_H
#define MAILTRANSPORT_RESOURCESENDJOB_P_H

#include <transportjob.h>

#include <item.h>

namespace MailTransport
{

class ResourceSendJobPrivate;

/**
  Mail transport job for an Akonadi resource-based transport.

  This is a wrapper job that makes old applications work with resource-based
  transports.  It calls the appropriate methods in MessageQueueJob, and emits
  result() as soon as the item is placed in the outbox, since there is no way
  of monitoring the progress from here.

  @deprecated Use MessageQueueJob for placing messages in the outbox.

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class MAILTRANSPORT_DEPRECATED ResourceSendJob : public TransportJob
{
    Q_OBJECT
public:
    /**
      Creates an ResourceSendJob.
      @param transport The transport object to use.
      @param parent The parent object.
    */
    explicit ResourceSendJob(Transport *transport, QObject *parent = 0);

    /**
      Destroys this job.
    */
    virtual ~ResourceSendJob();

protected:
    /** reimpl */
    virtual void doStart();

private:
    friend class ResourceSendJobPrivate;
    ResourceSendJobPrivate *const d;

    Q_PRIVATE_SLOT(d, void slotEmitResult())

};

} // namespace MailTransport

#endif // MAILTRANSPORT_RESOURCESENDJOB_H
