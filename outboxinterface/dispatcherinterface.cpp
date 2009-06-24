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

//#include "mdainterface.h"

#include <QTimer>

#include <KDebug>
#include <KGlobal>
#include <KLocalizedString>

#include <akonadi/agentmanager.h>

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

    bool connected;
    //org::kde::Akonadi::MailDispatcher *iface;
    AgentInstance agent;

    // slots
    void connectToAgent();
    //void dbusServiceOwnerChanged( const QString &name, const QString &oldOwner, const QString &newOwner );
    void agentInstanceRemoved( const AgentInstance &a );
    void agentInstanceChanged( const AgentInstance &a );

};

K_GLOBAL_STATIC( DispatcherInterfacePrivate, sInstance )

DispatcherInterfacePrivate::DispatcherInterfacePrivate()
  : instance( new DispatcherInterface( this ) )
  //, iface( 0 )
{
  // QDBusConnection bus = QDBusConnection::sessionBus();
  // QObject::connect( bus.interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
  //     instance, SLOT(dbusServiceOwnerChanged(QString,QString,QString)) );

  // AgentInstance objects are not updated automatically, so we need to watch
  // for AgentManager's signals:
  QObject::connect( AgentManager::self(), SIGNAL(instanceOnline(Akonadi::AgentInstance,bool)),
      instance, SLOT(agentInstanceChanged(Akonadi::AgentInstance)) );
  QObject::connect( AgentManager::self(), SIGNAL(instanceProgressChanged(Akonadi::AgentInstance)),
      instance, SLOT(agentInstanceChanged(Akonadi::AgentInstance)) );
  QObject::connect( AgentManager::self(), SIGNAL(instanceStatusChanged(Akonadi::AgentInstance)),
      instance, SLOT(agentInstanceChanged(Akonadi::AgentInstance)) );
  QObject::connect( AgentManager::self(), SIGNAL(instanceRemoved(Akonadi::AgentInstance)),
      instance, SLOT(agentInstanceRemoved(Akonadi::AgentInstance)) );
  connected = false;
  connectToAgent();
}

DispatcherInterfacePrivate::~DispatcherInterfacePrivate()
{
  delete instance;
}

void DispatcherInterfacePrivate::connectToAgent()
{
  if( connected ) {
    kDebug() << "Already connected to MDA.";
    return;
  }

#if 0
  delete iface;
  iface = new org::kde::Akonadi::MailDispatcher( QLatin1String( "org.freedesktop.Akonadi.Agent.akonadi_maildispatcher_agent" ),
      QLatin1String( "/" ), QDBusConnection::sessionBus(), instance );
  if( !iface->isValid() ) {
    kDebug() << "Couldn't get D-Bus interface of MDA. Retrying in 1s.";
    QTimer::singleShot( 1000, instance, SLOT(connectToAgent()) );
    return;
  }
#endif

  agent = AgentManager::self()->instance( QLatin1String( "akonadi_maildispatcher_agent" ) );
  if( !agent.isValid() ) {
    kDebug() << "Could not get agent instance of MDA. Retrying in 1s.";
    QTimer::singleShot( 1000, instance, SLOT(connectToAgent()) );
    return;
  }

  kDebug() << "Connected to the MDA.";
  connected = true;
}

#if 0
void DispatcherInterfacePrivate::dbusServiceOwnerChanged( const QString &name, const QString &oldOwner, const QString &newOwner )
{
  Q_UNUSED( oldOwner );
  if( name == QLatin1String( "org.freedesktop.Akonadi.Agent.akonad_maildispatcher_agent" ) ) {
    if( newOwner.isEmpty() ) {
      kDebug() << "MDA disappeared from D-Bus.";
      connected = false;
      QTimer::singleShot( 0, instance, SLOT(connectToAgent()) );
    }
  }
}
#endif

void DispatcherInterfacePrivate::agentInstanceRemoved( const AgentInstance &a )
{
  if( agent == a ) {
    kDebug() << "MDA agent disappeared.";
    connected = false;
    QTimer::singleShot( 0, instance, SLOT(connectToAgent()) );
  }
}

void DispatcherInterfacePrivate::agentInstanceChanged( const AgentInstance &a )
{
  if( agent == a ) {
    kDebug() << "Updating instance.";
    agent = a;
    // This is not as weird as it looks :) operator== checks the id only, but
    // operator= copies everything (like status, progress etc.)
  }
}



DispatcherInterface::DispatcherInterface( DispatcherInterfacePrivate *dd )
  : QObject()
  , d( dd )
{
}

DispatcherInterface *DispatcherInterface::self()
{
  return sInstance->instance;
}

bool DispatcherInterface::isReady() const
{
  return d->connected;
}

bool DispatcherInterface::dispatcherOnline() const
{
  if( !d->connected ) {
    kWarning() << "Not connected to the MDA.";
    return false;
  }
 
  return d->agent.isOnline();
}

AgentInstance::Status DispatcherInterface::dispatcherStatus() const
{
  if( !d->connected ) {
    kWarning() << "Not connected to the MDA.";
    return AgentInstance::Broken;
  }

  return d->agent.status();
}

int DispatcherInterface::dispatcherProgress() const
{
  if( !d->connected ) {
    kWarning() << "Not connected to the MDA.";
    return -1;
  }

  return d->agent.progress();
}

void DispatcherInterface::abortDispatching()
{
  if( !d->connected ) {
    kWarning() << "Not connected to the MDA.";
    return;
  }

  d->agent.abort();
}

void DispatcherInterface::dispatchManually()
{
  if( !d->connected ) {
    kWarning() << "Not connected to the MDA.";
    return;
  }

  kDebug() << "implement me"; //TODO
}

void DispatcherInterface::retryDispatching()
{
  if( !d->connected ) {
    kWarning() << "Not connected to the MDA.";
    return;
  }

  kDebug() << "implement me"; //TODO
}



#include "dispatcherinterface.moc"
