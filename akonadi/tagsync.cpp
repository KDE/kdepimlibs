/*
    Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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
namespace Akonadi {
    class Item;
}

unsigned int qHash(const Akonadi::Item &item);

#include "tagsync.h"

#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/tagfetchjob.h>
#include <akonadi/tagcreatejob.h>
#include <akonadi/tagmodifyjob.h>

using namespace Akonadi;

//We want to compare items by remoteId and not by id
uint qHash(const Item &item)
{
    if (item.isValid()) {
        return qHash(item.id());
    }
    Q_ASSERT(!item.remoteId().isEmpty());
    return qHash(item.remoteId());
}

bool operator==(const Item &left, const Item &right)
{
    if (left.isValid() && right.isValid() && (left.id() == right.id())) {
        return true;
    }
    if (!left.remoteId().isEmpty() && !right.remoteId().isEmpty() && (left.remoteId() == right.remoteId())) {
        return true;
    }
    return false;
}

TagSync::TagSync(QObject *parent)
    : Job(parent),
    mDeliveryDone(false),
    mTagMembersDeliveryDone(false),
    mLocalTagsFetched(false)
{

}

TagSync::~TagSync()
{

}

void TagSync::setFullTagList(const Akonadi::Tag::List &tags)
{
    mRemoteTags = tags;
    mDeliveryDone = true;
    diffTags();
}

void TagSync::setTagMembers(const QHash<QString, Akonadi::Item::List> &ridMemberMap)
{
    mRidMemberMap = ridMemberMap;
    mTagMembersDeliveryDone = true;
    diffTags();
}

void TagSync::doStart()
{
    // kDebug();
    //This should include all tags, including the ones that don't have a remote id
    Akonadi::TagFetchJob *fetch = new Akonadi::TagFetchJob(this);
    connect(fetch, SIGNAL(result(KJob*)), this, SLOT(onLocalTagFetchDone(KJob*)));
}

void TagSync::onLocalTagFetchDone(KJob *job)
{
    // kDebug();
    TagFetchJob *fetch = static_cast<TagFetchJob*>(job);
    mLocalTags = fetch->tags();
    mLocalTagsFetched = true;
    diffTags();
}

void TagSync::diffTags()
{
    if (!mDeliveryDone || !mTagMembersDeliveryDone || !mLocalTagsFetched) {
        kDebug() << "waiting for delivery: " << mDeliveryDone << mLocalTagsFetched;
        return;
    }
    // kDebug() << "diffing";
    QHash<QByteArray, Akonadi::Tag> tagByGid;
    QHash<QByteArray, Akonadi::Tag> tagByRid;
    QHash<Akonadi::Tag::Id, Akonadi::Tag> tagById;
    Q_FOREACH (const Akonadi::Tag &localTag, mLocalTags) {
        tagByRid.insert(localTag.remoteId(), localTag);
        tagByGid.insert(localTag.gid(), localTag);
        if (!localTag.remoteId().isEmpty()) {
            tagById.insert(localTag.id(), localTag);
        }
    }
    Q_FOREACH (const Akonadi::Tag &remoteTag, mRemoteTags) {
        if (tagByRid.contains(remoteTag.remoteId())) {
            //Tag still exists, check members
            Tag tag = tagByRid.value(remoteTag.remoteId());
            ItemFetchJob *itemFetch = new ItemFetchJob(tag, this);
            itemFetch->setProperty("tag", QVariant::fromValue(tag));
            itemFetch->setProperty("merge", false);
            connect(itemFetch, SIGNAL(result(KJob*)), this, SLOT(onTagItemsFetchDone(KJob*)));
            connect(itemFetch, SIGNAL(result(KJob*)), this, SLOT(onJobDone(KJob*)));
            tagById.remove(tagByRid.value(remoteTag.remoteId()).id());
        } else if (tagByGid.contains(remoteTag.gid())) {
            //Tag exists but has no rid
            //Merge members and set rid
            Tag tag = tagByGid.value(remoteTag.gid());
            tag.setRemoteId(remoteTag.remoteId());
            ItemFetchJob *itemFetch = new ItemFetchJob(tag, this);
            itemFetch->setProperty("tag", QVariant::fromValue(tag));
            itemFetch->setProperty("merge", true);
            connect(itemFetch, SIGNAL(result(KJob*)), this, SLOT(onTagItemsFetchDone(KJob*)));
            connect(itemFetch, SIGNAL(result(KJob*)), this, SLOT(onJobDone(KJob*)));
            tagById.remove(tagByGid.value(remoteTag.gid()).id());
        } else {
            //New tag, create
            TagCreateJob *createJob = new TagCreateJob(remoteTag, this);
            createJob->setMergeIfExisting(true);
            connect(createJob, SIGNAL(result(KJob*)), this, SLOT(onCreateTagDone(KJob*)));
            connect(createJob, SIGNAL(result(KJob*)), this, SLOT(onJobDone(KJob*)));
            //TODO add tags
        }
    }
    Q_FOREACH (const Akonadi::Tag::Id &removedTag, tagById.keys()) {
        //Removed remotely, unset rid
        Tag tag = tagById.value(removedTag);
        tag.setRemoteId(QByteArray(""));
        TagModifyJob *modJob = new TagModifyJob(tag, this);
        connect(modJob, SIGNAL(result(KJob*)), this, SLOT(onJobDone(KJob*)));
    }
    checkDone();
}

static QSet<QString> ridSet(const Akonadi::Item::List &list)
{
    QSet<QString> set;
    Q_FOREACH (const Akonadi::Item &item, list) {
        set << item.remoteId();
    }
    return set;
}

void TagSync::onCreateTagDone(KJob *job)
{
    if (job->error()) {
        kWarning() << "ItemFetch failed: " << job->errorString();
        // cancelTask(job->errorString());
        return;
    }

    Akonadi::Tag tag = static_cast<Akonadi::TagCreateJob*>(job)->tag();
    const Item::List remoteMembers = mRidMemberMap.value(QString::fromLatin1(tag.remoteId()));
    Q_FOREACH (Item item, remoteMembers) {
        item.setTag(tag);
        ItemModifyJob *modJob = new ItemModifyJob(item, this);
        connect(modJob, SIGNAL(result(KJob*)), this, SLOT(onJobDone(KJob*)));
        kDebug() << "setting tag " << item.remoteId();
    }
}

void TagSync::onTagItemsFetchDone(KJob *job)
{
    if (job->error()) {
        kWarning() << "ItemFetch failed: " << job->errorString();
        // cancelTask(job->errorString());
        return;
    }

    const Akonadi::Item::List items = static_cast<Akonadi::ItemFetchJob*>(job)->items();
    const Akonadi::Tag tag = job->property("tag").value<Akonadi::Tag>();
    const bool merge = job->property("merge").toBool();
    const QSet<Item> localMembers = items.toSet();
    const QSet<Item> remoteMembers = mRidMemberMap.value(QString::fromLatin1(tag.remoteId())).toSet();
    const QSet<Item> toAdd = remoteMembers - localMembers;
    const QSet<Item> toRemove = localMembers - remoteMembers;
    if (!merge) {
        Q_FOREACH (Item item, toRemove) {
            item.clearTag(tag);
            ItemModifyJob *modJob = new ItemModifyJob(item, this);
            connect(modJob, SIGNAL(result(KJob*)), this, SLOT(onJobDone(KJob*)));
            kDebug() << "removing tag " << item.remoteId();
        }
    }
    Q_FOREACH (Item item, toAdd) {
        item.setTag(tag);
        ItemModifyJob *modJob = new ItemModifyJob(item, this);
        connect(modJob, SIGNAL(result(KJob*)), this, SLOT(onJobDone(KJob*)));
        kDebug() << "setting tag " << item.remoteId();
    }
}

void TagSync::onJobDone(KJob *job)
{
    checkDone();
}

void TagSync::slotResult(KJob *job)
{
    if (job->error()) {
        kWarning() << "Error during CollectionSync: " << job->errorString() << job->metaObject()->className();
        // pretent there were no errors
        Akonadi::Job::removeSubjob(job);
    } else {
        Akonadi::Job::slotResult(job);
    }
}

void TagSync::checkDone()
{
    if (hasSubjobs()) {
        kDebug() << "Still going";
        return;
    }
    kDebug() << "done";
    emitResult();
}

#include "tagsync.moc"
