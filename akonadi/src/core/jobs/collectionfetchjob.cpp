/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#include "collectionfetchjob.h"

#include "job_p.h"
#include "protocolhelper_p.h"
#include "entity_p.h"
#include "collectionfetchscope.h"
#include "collectionutils.h"
#include "protocolhelper_p.h"
#include <akonadi/private/protocol_p.h>

#include <qdebug.h>
#include <KLocalizedString>

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

using namespace Akonadi;

class Akonadi::CollectionFetchJobPrivate : public JobPrivate
{
public:
    CollectionFetchJobPrivate(CollectionFetchJob *parent)
        : JobPrivate(parent)
        , mEmitTimer(0)
        , mBasePrefetch(false)
    {

    }

    void init()
    {
        mEmitTimer = new QTimer(q_ptr);
        mEmitTimer->setSingleShot(true);
        mEmitTimer->setInterval(100);
        q_ptr->connect(mEmitTimer, SIGNAL(timeout()), q_ptr, SLOT(timeout()));
    }

    Q_DECLARE_PUBLIC(CollectionFetchJob)

    CollectionFetchJob::Type mType;
    Collection mBase;
    Collection::List mBaseList;
    Collection::List mCollections;
    CollectionFetchScope mScope;
    Collection::List mPendingCollections;
    QTimer *mEmitTimer;
    bool mBasePrefetch;
    Collection::List mPrefetchList;

    void aboutToFinish() Q_DECL_OVERRIDE
    {
      timeout();
    }

    void timeout()
    {
        Q_Q(CollectionFetchJob);

        mEmitTimer->stop(); // in case we are called by result()
        if (!mPendingCollections.isEmpty()) {
            if (!q->error() || mScope.ignoreRetrievalErrors()) {
                emit q->collectionsReceived(mPendingCollections);
            }
            mPendingCollections.clear();
        }
    }

    void subJobCollectionReceived(const Akonadi::Collection::List &collections)
    {
        mPendingCollections += collections;
        if (!mEmitTimer->isActive()) {
            mEmitTimer->start();
        }
    }

    QString jobDebuggingString() const Q_DECL_OVERRIDE
    {
        if (mBase.isValid()) {
            return QStringLiteral("Collection Id %1").arg(mBase.id());
        } else if (CollectionUtils::hasValidHierarchicalRID(mBase)) {
            //return QLatin1String("(") + ProtocolHelper::hierarchicalRidToScope(mBase).hridChain().join(QLatin1String(", ")) + QLatin1String(")");
            return QLatin1String("HRID chain");
        } else {
            return QString::fromLatin1("Collection RemoteId %1").arg(mBase.remoteId());
        }
    }

    bool jobFailed(KJob *job)
    {
        Q_Q(CollectionFetchJob);
        if (mScope.ignoreRetrievalErrors()) {
           int error = job->error();
           if (error && !q->error()) {
               q->setError(error);
               q->setErrorText(job->errorText());
           }

           if (error == Job::ConnectionFailed ||
               error == Job::ProtocolVersionMismatch ||
               error == Job::UserCanceled) {
               return true;
           }
           return false;
        } else {
            return job->error();
        }
    }
};

CollectionFetchJob::CollectionFetchJob(const Collection &collection, Type type, QObject *parent)
    : Job(new CollectionFetchJobPrivate(this), parent)
{
    Q_D(CollectionFetchJob);
    d->init();

    d->mBase = collection;
    d->mType = type;
}

CollectionFetchJob::CollectionFetchJob(const Collection::List &cols, QObject *parent)
    : Job(new CollectionFetchJobPrivate(this), parent)
{
    Q_D(CollectionFetchJob);
    d->init();

    Q_ASSERT(!cols.isEmpty());
    if (cols.size() == 1) {
        d->mBase = cols.first();
    } else {
        d->mBaseList = cols;
    }
    d->mType = CollectionFetchJob::Base;
}

CollectionFetchJob::CollectionFetchJob(const Collection::List &cols, Type type, QObject *parent)
    : Job(new CollectionFetchJobPrivate(this), parent)
{
    Q_D(CollectionFetchJob);
    d->init();

    Q_ASSERT(!cols.isEmpty());
    if (cols.size() == 1) {
        d->mBase = cols.first();
    } else {
        d->mBaseList = cols;
    }
    d->mType = type;
}

CollectionFetchJob::CollectionFetchJob(const QList<Collection::Id> &cols, Type type, QObject *parent)
    : Job(new CollectionFetchJobPrivate(this), parent)
{
    Q_D(CollectionFetchJob);
    d->init();

    Q_ASSERT(!cols.isEmpty());
    if (cols.size() == 1) {
        d->mBase = Collection(cols.first());
    } else {
        foreach (Collection::Id id, cols) {
            d->mBaseList.append(Collection(id));
        }
    }
    d->mType = type;
}

CollectionFetchJob::~CollectionFetchJob()
{
}

Akonadi::Collection::List CollectionFetchJob::collections() const
{
    Q_D(const CollectionFetchJob);

    return d->mCollections;
}

void CollectionFetchJob::doStart()
{
    Q_D(CollectionFetchJob);

    if (!d->mBaseList.isEmpty()) {
        if (d->mType == Recursive) {
            // Because doStart starts several subjobs and @p cols could contain descendants of
            // other elements in the list, if type is Recusrive, we could end up with duplicates in the result.
            // To fix this we require an initial fetch of @p cols with Base and RetrieveAncestors,
            // Iterate over that result removing intersections and then perform the Recursive fetch on
            // the remainder.
            d->mBasePrefetch = true;
            // No need to connect to the collectionsReceived signal here. This job is internal. The
            // result needs to be filtered through filterDescendants before it is useful.
            new CollectionFetchJob(d->mBaseList, NonOverlappingRoots, this);
        } else if (d->mType == NonOverlappingRoots) {
            foreach (const Collection &col, d->mBaseList) {
                // No need to connect to the collectionsReceived signal here. This job is internal. The (aggregated)
                // result needs to be filtered through filterDescendants before it is useful.
                CollectionFetchJob *subJob = new CollectionFetchJob(col, Base, this);
                subJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
            }
        } else {
            foreach (const Collection &col, d->mBaseList) {
                CollectionFetchJob *subJob = new CollectionFetchJob(col, d->mType, this);
                connect(subJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), SLOT(subJobCollectionReceived(Akonadi::Collection::List)));
                subJob->setFetchScope(fetchScope());
            }
        }
        return;
    }

    if (!d->mBase.isValid() && d->mBase.remoteId().isEmpty()) {
        setError(Unknown);
        setErrorText(i18n("Invalid collection given."));
        emitResult();
        return;
    }

    Protocol::FetchCollectionsCommand cmd(ProtocolHelper::entityToScope(d->mBase));
    switch (d->mType) {
    case Base:
        cmd.setDepth(Protocol::FetchCollectionsCommand::BaseCollection);
        break;
    case Akonadi::CollectionFetchJob::FirstLevel:
        cmd.setDepth(Protocol::FetchCollectionsCommand::ParentCollection);
        break;
    case Akonadi::CollectionFetchJob::Recursive:
        cmd.setDepth(Protocol::FetchCollectionsCommand::AllCollections);
        break;
    default:
        Q_ASSERT(false);
    }
    cmd.setResource(d->mScope.resource());
    cmd.setMimeTypes(d->mScope.contentMimeTypes());

    switch (d->mScope.listFilter()) {
    case CollectionFetchScope::Display:
        cmd.setDisplayPref(true);
        break;
    case CollectionFetchScope::Sync:
        cmd.setSyncPref(true);
        break;
    case CollectionFetchScope::Index:
        cmd.setIndexPref(true);
        break;
    case CollectionFetchScope::Enabled:
        cmd.setEnabled(true);
        break;
    case CollectionFetchScope::NoFilter:
        break;
    default:
        Q_ASSERT(false);
    }

    cmd.setFetchStats(d->mScope.includeStatistics());
    switch (d->mScope.ancestorRetrieval()) {
    case CollectionFetchScope::None:
        cmd.setAncestorsDepth(Protocol::Ancestor::NoAncestor);
        break;
    case CollectionFetchScope::Parent:
        cmd.setAncestorsDepth(Protocol::Ancestor::ParentAncestor);
        break;
    case CollectionFetchScope::All:
        cmd.setAncestorsDepth(Protocol::Ancestor::AllAncestors);
        break;
    }
    if (d->mScope.ancestorRetrieval() != CollectionFetchScope::None) {
        cmd.setAncestorsAttributes(d->mScope.ancestorFetchScope().attributes());
    }

    d->sendCommand(cmd);
}

void CollectionFetchJob::doHandleResponse(qint64 tag, const Protocol::Command &response)
{
    Q_D(CollectionFetchJob);

    if (d->mBasePrefetch || d->mType == NonOverlappingRoots) {
        return;
    }

    if (!response.isResponse() || response.type() != Protocol::Command::FetchCollections) {
        Job::doHandleResponse(tag, response);
        return;
    }

    Protocol::FetchCollectionsResponse resp(response);
    // Invalid response (no ID) means this was the last response
    if (resp.id() == -1) {
        emitResult();
        return;
    }

    Collection collection = ProtocolHelper::parseCollection(resp, true);
    if (!collection.isValid()) {
        return;
    }

    collection.d_ptr->resetChangeLog();
    d->mCollections.append(collection);
    d->mPendingCollections.append(collection);
    if (!d->mEmitTimer->isActive()) {
        d->mEmitTimer->start();
    }
}

static Collection::List filterDescendants(const Collection::List &list)
{
    Collection::List result;

    QVector<QList<Collection::Id> > ids;
    ids.reserve(list.count());
    foreach (const Collection &collection, list) {
        QList<Collection::Id> ancestors;
        Collection parent = collection.parentCollection();
        ancestors << parent.id();
        if (parent != Collection::root()) {
            while (parent.parentCollection() != Collection::root()) {
                parent = parent.parentCollection();
                QList<Collection::Id>::iterator i = qLowerBound(ancestors.begin(), ancestors.end(), parent.id());
                ancestors.insert(i, parent.id());
            }
        }
        ids << ancestors;
    }

    QSet<Collection::Id> excludeList;
    foreach (const Collection &collection, list) {
        int i = 0;
        foreach (const QList<Collection::Id> &ancestors, ids) {
            if (qBinaryFind(ancestors, collection.id()) != ancestors.end()) {
                excludeList.insert(list.at(i).id());
            }
            ++i;
        }
    }

    foreach (const Collection &collection, list) {
        if (!excludeList.contains(collection.id())) {
            result.append(collection);
        }
    }

    return result;
}

void CollectionFetchJob::slotResult(KJob *job)
{
    Q_D(CollectionFetchJob);

    CollectionFetchJob *list = qobject_cast<CollectionFetchJob *>(job);
    Q_ASSERT(job);

    if (d->mType == NonOverlappingRoots) {
        d->mPrefetchList += list->collections();
    } else if (!d->mBasePrefetch) {
        d->mCollections += list->collections();
    }

    if (d_ptr->mCurrentSubJob == job && !d->jobFailed(job)) {
        if (job->error()) {
            qWarning() << "Error during CollectionFetchJob: " << job->errorString();
        }
        d_ptr->mCurrentSubJob = 0;
        removeSubjob(job);
        QTimer::singleShot(0, this, SLOT(startNext()));
    } else {
        Job::slotResult(job);
    }

    if (d->mBasePrefetch) {
        d->mBasePrefetch = false;
        const Collection::List roots = list->collections();
        Q_ASSERT(!hasSubjobs());
        if (!job->error()) {
            foreach (const Collection &col, roots) {
                CollectionFetchJob *subJob = new CollectionFetchJob(col, d->mType, this);
                connect(subJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), SLOT(subJobCollectionReceived(Akonadi::Collection::List)));
                subJob->setFetchScope(fetchScope());
            }
        }
        // No result yet.
    } else if (d->mType == NonOverlappingRoots) {
        if (!d->jobFailed(job) && !hasSubjobs()) {
            const Collection::List result = filterDescendants(d->mPrefetchList);
            d->mPendingCollections += result;
            d->mCollections = result;
            d->delayedEmitResult();
        }
    } else {
        if (!d->jobFailed(job) && !hasSubjobs()) {
            d->delayedEmitResult();
        }
    }
}

void CollectionFetchJob::setFetchScope(const CollectionFetchScope &scope)
{
    Q_D(CollectionFetchJob);
    d->mScope = scope;
}

CollectionFetchScope &CollectionFetchJob::fetchScope()
{
    Q_D(CollectionFetchJob);
    return d->mScope;
}

#include "moc_collectionfetchjob.cpp"
