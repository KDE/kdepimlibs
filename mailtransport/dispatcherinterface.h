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

#ifndef MAILTRANSPORT_DISPATCHERINTERFACE_H
#define MAILTRANSPORT_DISPATCHERINTERFACE_H

#include <mailtransport/mailtransport_export.h>

#include <akonadi/agentinstance.h>

namespace MailTransport {

/**
  @short An interface for applications to interact with the dispatcher agent.

  This class provides methods such as send queued messages (@see
  dispatchManually) and retry sending (@see retryDispatching).

  This class also takes care of registering the attributes that the mail
  dispatcher agent and MailTransport use.

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class MAILTRANSPORT_EXPORT DispatcherInterface
{
  public:

    /**
      Creates a new dispatcher interface.
    */
    DispatcherInterface();

    /**
      Returns the current instance of the mail dispatcher agent. May return an invalid
      AgentInstance in case it cannot find the mail dispatcher agent.
    */
    Akonadi::AgentInstance dispatcherInstance() const;

    /**
      Looks for messages in the outbox with DispatchMode::Manual and marks them
      DispatchMode::Automatic for sending.
    */
    void dispatchManually();

    /**
      Looks for messages in the outbox with ErrorAttribute, and clears them and
      queues them again for sending.
    */
    void retryDispatching();
};

} // namespace MailTransport

#endif // MAILTRANSPORT_DISPATCHERINTERFACE_H
