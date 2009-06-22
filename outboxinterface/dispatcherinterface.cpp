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

#include "addressattribute.h"
#include "dispatchmodeattribute.h"
#include "errorattribute.h"
#include "outboxactions.h"
#include "sentbehaviourattribute.h"
#include "transportattribute.h"

#include <QTimer>

#include <KDebug>
#include <KGlobal>
#include <KLocalizedString>

#include <akonadi/agentmanager.h>
#include <akonadi/attributefactory.h>
#include <akonadi/collection.h>
#include <akonadi/filteractionjob.h>
#include <akonadi/kmime/localfolders.h>

using namespace Akonadi;
using namespace OutboxInterface;


/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class OutboxInterface::DispatcherInterfacePrivate
{
  public:
    DispatcherInterfacePrivate();
    ~DispatcherInterfacePrivate();

    DispatcherInterface *instance;

    // slots
    void massModifyResult( KJob *job );

};

K_GLOBAL_STATIC( DispatcherInterfacePrivate, sInstance )

DispatcherInterfacePrivate::DispatcherInterfacePrivate()
  : instance( new DispatcherInterface( this ) )
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



DispatcherInterface::DispatcherInterface( DispatcherInterfacePrivate *dd )
  : QObject()
  , d( dd )
{
  // register attributes
  AttributeFactory::registerAttribute<AddressAttribute>();
  AttributeFactory::registerAttribute<DispatchModeAttribute>();
  AttributeFactory::registerAttribute<ErrorAttribute>();
  AttributeFactory::registerAttribute<SentBehaviourAttribute>();
  AttributeFactory::registerAttribute<TransportAttribute>();
}

DispatcherInterface *DispatcherInterface::self()
{
  return sInstance->instance;
}

AgentInstance DispatcherInterface::dispatcherInstance() const
{
  AgentInstance a = AgentManager::self()->instance( QLatin1String( "akonadi_maildispatcher_agent" ) );
  if( !a.isValid() ) {
    kWarning() << "Could not get MDA instance.";
  }
  return a;
}

void DispatcherInterface::dispatchManually()
{
  if( !LocalFolders::self()->isReady() ) {
    kWarning() << "LocalFolders not ready.";
    return;
  }

  FilterActionJob *mjob = new FilterActionJob( LocalFolders::self()->outbox(), new SendQueuedAction, this );
  connect( mjob, SIGNAL(result(KJob*)), this, SLOT(massModifyResult(KJob*)) );
}

void DispatcherInterface::retryDispatching()
{
  if( !LocalFolders::self()->isReady() ) {
    kWarning() << "LocalFolders not ready.";
    return;
  }

  FilterActionJob *mjob = new FilterActionJob( LocalFolders::self()->outbox(), new ClearErrorAction, this );
  connect( mjob, SIGNAL(result(KJob*)), this, SLOT(massModifyResult(KJob*)) );
}



#include "dispatcherinterface.moc"
