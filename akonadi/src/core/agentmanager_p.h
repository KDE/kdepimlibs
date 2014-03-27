/*
    Copyright (c) 2006-2008 Tobias Koenig <tokoe@kde.org>

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

#ifndef AKONADI_AGENTMANAGER_P_H
#define AKONADI_AGENTMANAGER_P_H

#include "agentmanagerinterface.h"

#include "agenttype.h"
#include "agentinstance.h"

#include <QtCore/QHash>

namespace Akonadi {

class AgentManager;

/**
 * @internal
 */
class AgentManagerPrivate
{
    friend class AgentManager;

public:
    AgentManagerPrivate(AgentManager *parent)
        : mParent(parent)
        , mManager(0)
    {
    }

    /*
     * Used by AgentInstanceCreateJob
     */
    AgentInstance createInstance(const AgentType &type);

    void agentTypeAdded(const QString &);
    void agentTypeRemoved(const QString &);
    void agentInstanceAdded(const QString &);
    void agentInstanceRemoved(const QString &);
    void agentInstanceStatusChanged(const QString &, int, const QString &);
    void agentInstanceProgressChanged(const QString &, uint, const QString &);
    void agentInstanceNameChanged(const QString &, const QString &);
    void agentInstanceWarning(const QString &, const QString &);
    void agentInstanceError(const QString &, const QString &);
    void agentInstanceOnlineChanged(const QString &, bool);

    /**
     * Reads the information about all known agent types from the serverside
     * agent manager and updates mTypes, like agentTypeAdded() does.
     *
     * This will not remove agents from the internal map that are no longer on
     * the server.
     */
    void readAgentTypes();

    /**
     * Reads the information about all known agent instances from the server. If AgentManager
     * is created before the Akonadi.Control interface is registered, the agent
     * instances aren't immediately found then.
     */
    void readAgentInstances();

    void setName(const AgentInstance &, const QString &);
    void setOnline(const AgentInstance &, bool);
    void configure(const AgentInstance &, QWidget *);
    void synchronize(const AgentInstance &);
    void synchronizeCollectionTree(const AgentInstance &);

    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void createDBusInterface();

    AgentType fillAgentType(const QString &identifier) const;
    AgentInstance fillAgentInstance(const QString &identifier) const;
    AgentInstance fillAgentInstanceLight(const QString &identifier) const;

    static AgentManager *mSelf;

    AgentManager *mParent;
    org::freedesktop::Akonadi::AgentManager *mManager;

    QHash<QString, AgentType> mTypes;
    QHash<QString, AgentInstance> mInstances;
};

}

#endif
