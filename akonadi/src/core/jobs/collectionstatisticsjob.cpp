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

#include "collectionstatisticsjob.h"

#include "collection.h"
#include "collectionstatistics.h"
#include "job_p.h"
#include "protocolhelper_p.h"

#include <akonadi/private/protocol_p.h>

using namespace Akonadi;

class Akonadi::CollectionStatisticsJobPrivate : public JobPrivate
{
public:
    CollectionStatisticsJobPrivate(CollectionStatisticsJob *parent)
        : JobPrivate(parent)
    {
    }

    QString jobDebuggingString() const Q_DECL_OVERRIDE { /*Q_DECL_OVERRIDE*/
        return QStringLiteral("Collection Id %1").arg(mCollection.id());
    }

    Collection mCollection;
    CollectionStatistics mStatistics;
};

CollectionStatisticsJob::CollectionStatisticsJob(const Collection &collection, QObject *parent)
    : Job(new CollectionStatisticsJobPrivate(this), parent)
{
    Q_D(CollectionStatisticsJob);

    d->mCollection = collection;
}

CollectionStatisticsJob::~CollectionStatisticsJob()
{
}

void CollectionStatisticsJob::doStart()
{
    Q_D(CollectionStatisticsJob);

    try {
        d->sendCommand(Protocol::FetchCollectionStatsCommand(ProtocolHelper::entityToScope(d->mCollection)));
    } catch (const std::exception &e) {
        setError(Unknown);
        setErrorText(QString::fromUtf8(e.what()));
        emitResult();
        return;
    }
}

void CollectionStatisticsJob::doHandleResponse(qint64 tag, const Protocol::Command &response)
{
    Q_D(CollectionStatisticsJob);

    if (!response.isResponse() || response.type() != Protocol::Command::FetchCollectionStats) {
        Job::doHandleResponse(tag, response);
        return;
    }

    d->mStatistics = ProtocolHelper::parseCollectionStatistics(response);
    emitResult();
}

Collection CollectionStatisticsJob::collection() const
{
    Q_D(const CollectionStatisticsJob);

    return d->mCollection;
}

CollectionStatistics Akonadi::CollectionStatisticsJob::statistics() const
{
    Q_D(const CollectionStatisticsJob);

    return d->mStatistics;
}
