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

#ifndef OUTBOXINTERFACE_DISPATCHERINTERFACE_H
#define OUTBOXINTERFACE_DISPATCHERINTERFACE_H

#include <outboxinterface/outboxinterface_export.h>

#include <QtCore/QObject>

#include <akonadi/agentinstance.h>

namespace OutboxInterface {

class DispatcherInterfacePrivate;

/**
  An interface for apps to interact with the MDA.
  It connects to the MDA via D-Bus and provides info about queued messages
  and progress, and methods such as abort or retry sending.

  TODO should this class provide a progress widget?

  TODO dispatchManually and retryDispatching functions should be offered on a
  per-item basis as well, I imagine when the user will have a messageView of
  the outbox.  Then do we need global ones here (i.e. for all items in the
  outbox)?
*/
class OUTBOXINTERFACE_EXPORT DispatcherInterface : public QObject
{
  Q_OBJECT

  public:
    /**
      Returns the DispatcherInterface instance.
    */
    static DispatcherInterface *self();

    bool isReady() const;
    bool dispatcherOnline() const;
    Akonadi::AgentInstance::Status dispatcherStatus() const;
    int dispatcherProgress() const;

    /**
      Aborts sending the current message, and marks all messages in the queue
      as DispatchMode::Never.
    */
    void abortDispatching();

    /**
      Looks for messages in the outbox with DispatchMode::Never and marks them
      DispatchMode::Immediately for sending.
    */
    void dispatchManually();

    /**
      Looks for messages in the outbox with ErrorAttribute, and clears them and
      queues them again for sending.
    */
    void retryDispatching();


  private:
    friend class DispatcherInterfacePrivate;
    DispatcherInterfacePrivate *const d;

    // singleton class; the only instance resides in sInstance->instance
    DispatcherInterface( DispatcherInterfacePrivate *dd );

    Q_PRIVATE_SLOT( d, void connectToAgent() )
    Q_PRIVATE_SLOT( d, void dbusServiceOwnerChanged( const QString&, const QString&, const QString& ) )
    Q_PRIVATE_SLOT( d, void agentInstanceRemoved( const Akonadi::AgentInstance& ) )
    Q_PRIVATE_SLOT( d, void agentInstanceChanged( const Akonadi::AgentInstance& ) )

};


}


#endif
