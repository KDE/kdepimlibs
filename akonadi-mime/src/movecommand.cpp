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

#include "movecommand_p.h"
#include "util_p.h"

#include <itemmovejob.h>
#include <itemdeletejob.h>

MoveCommand::MoveCommand(const Akonadi::Collection &destFolder,
                         const QList<Akonadi::Item> &msgList,
                         QObject *parent)
    : CommandBase(parent)
{
    mDestFolder = destFolder;
    mMessages = msgList;
}

void MoveCommand::execute()
{
    if (mMessages.isEmpty()) {
        emitResult(OK);
        return;
    }
    if (mDestFolder.isValid()) {
        Akonadi::ItemMoveJob *job = new Akonadi::ItemMoveJob(mMessages, mDestFolder, this);
        connect(job, &Akonadi::ItemMoveJob::result, this, &MoveCommand::slotMoveResult);
    } else {
        Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob(mMessages, this);
        connect(job, &Akonadi::ItemDeleteJob::result, this, &MoveCommand::slotMoveResult);
    }
}

void MoveCommand::slotMoveResult(KJob *job)
{
    if (job->error()) {
        // handle errors
        Util::showJobError(job);
        emitResult(Failed);
    } else {
        emitResult(OK);
    }
}

#include "moc_movecommand_p.cpp"
