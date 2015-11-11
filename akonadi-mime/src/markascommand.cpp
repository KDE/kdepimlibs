/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "markascommand.h"
#include "util_p.h"
#include "akonadi_mime_debug.h"
#include <itemfetchjob.h>
#include <itemfetchscope.h>
#include <itemmodifyjob.h>
using namespace Akonadi;

class Akonadi::MarkAsCommandPrivate
{
public:
    MarkAsCommandPrivate()
        : mMarkJobCount(0),
          mFolderListJobCount(0),
          mInvertMark(0)
    {

    }

    Akonadi::Collection::List mFolders;
    Akonadi::Item::List mMessages;
    Akonadi::MessageStatus mTargetStatus;
    int mMarkJobCount;
    int mFolderListJobCount;
    int mInvertMark;
};

MarkAsCommand::MarkAsCommand(const Akonadi::MessageStatus &targetStatus, const Akonadi::Item::List &msgList, bool invert, QObject *parent)
    : CommandBase(parent),
      d(new Akonadi::MarkAsCommandPrivate())
{
    d->mInvertMark = invert;
    d->mMessages = msgList;
    d->mTargetStatus = targetStatus;
    d->mFolderListJobCount = 0;
    d->mMarkJobCount = 0;
}

MarkAsCommand::MarkAsCommand(const Akonadi::MessageStatus &targetStatus, const Akonadi::Collection::List &folders, bool invert, QObject *parent)
    : CommandBase(parent),
      d(new Akonadi::MarkAsCommandPrivate())
{
    d->mInvertMark = invert;
    d->mFolders = folders;
    d->mTargetStatus = targetStatus;
    d->mFolderListJobCount = d->mFolders.size();
    d->mMarkJobCount = 0;
}

MarkAsCommand::~MarkAsCommand()
{
    delete d;
}

void MarkAsCommand::slotFetchDone(KJob *job)
{
    d->mFolderListJobCount--;

    if (job->error()) {
        // handle errors
        Util::showJobError(job);
        emitResult(Failed);
        return;
    }

    Akonadi::ItemFetchJob *fjob = static_cast<Akonadi::ItemFetchJob *>(job);
    d->mMessages.clear();
    foreach (const Akonadi::Item &item, fjob->items()) {
        Akonadi::MessageStatus status;
        status.setStatusFromFlags(item.flags());
        if (d->mInvertMark) {
            if (status & d->mTargetStatus) {
                d->mMessages.append(item);
            }
        } else if (!(status & d->mTargetStatus)) {
            d->mMessages.append(item);
        }
    }
    if (d->mMessages.empty()) {
        if (d->mFolderListJobCount == 0) {
            emitResult(OK);
            return;
        }
    } else {
        markMessages();
    }
    if (d->mFolderListJobCount > 0) {
        Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(d->mFolders[d->mFolderListJobCount - 1], parent());
        job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
        connect(job, &Akonadi::ItemFetchJob::result, this, &MarkAsCommand::slotFetchDone);
    }
}

void MarkAsCommand::execute()
{
    if (!d->mFolders.isEmpty()) {
        //yes, we go backwards, shouldn't matter
        Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(d->mFolders[d->mFolderListJobCount - 1], parent());
        job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
        connect(job, &Akonadi::ItemFetchJob::result, this, &MarkAsCommand::slotFetchDone);
    } else if (!d->mMessages.isEmpty()) {
        d->mFolders << d->mMessages.first().parentCollection();
        markMessages();
    } else {
        emitResult(OK);
    }
}

void MarkAsCommand::markMessages()
{
    d->mMarkJobCount = 0;

    QSet<QByteArray> flags = d->mTargetStatus.statusFlags();
    Q_ASSERT(flags.size() == 1);
    Akonadi::Item::Flag flag;
    if (!flags.isEmpty()) {
        flag = *(flags.begin());
    }

    Akonadi::Item::List itemsToModify;
    foreach (const Akonadi::Item &it, d->mMessages) {
        Akonadi::Item item(it);

        // be careful to only change the flags we want to change, not to overwrite them
        // otherwise ItemModifyJob will not do what we expect
        if (d->mInvertMark) {
            if (item.hasFlag(flag)) {
                item.clearFlag(flag);
                itemsToModify.push_back(item);
            }
        } else {
            if (!item.hasFlag(flag)) {
                item.setFlag(flag);
                itemsToModify.push_back(item);
            }
        }
    }

    d->mMarkJobCount++;
    if (itemsToModify.isEmpty()) {
        slotModifyItemDone(0);   // pretend we did something
    } else {
        Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(itemsToModify, this);
        modifyJob->setIgnorePayload(true);
        modifyJob->disableRevisionCheck();
        connect(modifyJob, &Akonadi::ItemModifyJob::result, this, &MarkAsCommand::slotModifyItemDone);
    }
}

void MarkAsCommand::slotModifyItemDone(KJob *job)
{
    d->mMarkJobCount--;
    //NOTE(Andras): from kmail/kmmcommands, KMSetStatusCommand
    if (job && job->error()) {
        qCDebug(AKONADIMIME_LOG) << " Error trying to set item status:" << job->errorText();
        emitResult(Failed);
    }
    if (d->mMarkJobCount == 0 && d->mFolderListJobCount == 0) {
        emitResult(OK);
    }
}

