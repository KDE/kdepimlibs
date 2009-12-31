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

#include "dispatcherinterface.h"
#include "dispatcherinterface_p.h"

#include "outboxactions.h"

#include <KDebug>
#include <KGlobal>
#include <KLocalizedString>

#include <akonadi/agentmanager.h>
#include <akonadi/collection.h>
#include "filteractionjob.h"
#include <akonadi/kmime/specialmailcollections.h>

using namespace Akonadi;
using namespace MailTransport;

K_GLOBAL_STATIC( DispatcherInterfacePrivate, sInstance )

DispatcherInterfacePrivate::DispatcherInterfacePrivate()
  : instance( new DispatcherInterface() )
{
}

DispatcherInterfacePrivate::~DispatcherInterfacePrivate()
{
  delete instance;
}

void DispatcherInterfacePrivate::massModifyResult( KJob *job )
{
  // Nothing to do here, really.  If the job fails, the user can retry it.
  if( job->error() ) {
    kDebug() << "failed" << job->errorString();
  } else {
    kDebug() << "succeeded.";
  }
}

DispatcherInterface::DispatcherInterface()
{
}

DispatcherInterface *DispatcherInterface::self()
{
  return sInstance->instance;
}

AgentInstance DispatcherInterface::dispatcherInstance() const
{
  AgentInstance a =
    AgentManager::self()->instance( QLatin1String( "akonadi_maildispatcher_agent" ) );
  if( !a.isValid() ) {
    kWarning() << "Could not get MDA instance.";
  }
  return a;
}

void DispatcherInterface::dispatchManually()
{
  Collection outbox = SpecialMailCollections::self()->defaultCollection( SpecialMailCollections::Outbox );
  if( !outbox.isValid() ) {
    kError() << "Could not access Outbox.";
    return;
  }

  FilterActionJob *mjob = new FilterActionJob( outbox, new SendQueuedAction, sInstance );
  QObject::connect( mjob, SIGNAL(result(KJob*)), sInstance, SLOT(massModifyResult(KJob*)) );
}

void DispatcherInterface::retryDispatching()
{
  Collection outbox = SpecialMailCollections::self()->defaultCollection( SpecialMailCollections::Outbox );
  if( !outbox.isValid() ) {
    kError() << "Could not access Outbox.";
    return;
  }

  FilterActionJob *mjob = new FilterActionJob( outbox, new ClearErrorAction, sInstance );
  QObject::connect( mjob, SIGNAL(result(KJob*)), sInstance, SLOT(massModifyResult(KJob*)) );
}

#include "dispatcherinterface_p.moc"
