/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "contactcompletionmodel_p.h"

#include <changerecorder.h>
#include <entitymimetypefiltermodel.h>
#include <itemfetchscope.h>
#include <session.h>

#include <kcontacts/addressee.h>

using namespace Akonadi;

QAbstractItemModel *ContactCompletionModel::mSelf = Q_NULLPTR;

QAbstractItemModel *ContactCompletionModel::self()
{
    if (mSelf) {
        return mSelf;
    }

    ChangeRecorder *monitor = new ChangeRecorder;
    monitor->fetchCollection(true);
    monitor->itemFetchScope().fetchFullPayload();
    monitor->setCollectionMonitored(Akonadi::Collection::root());
    monitor->setMimeTypeMonitored(KContacts::Addressee::mimeType());

    ContactCompletionModel *model = new ContactCompletionModel(monitor);

    EntityMimeTypeFilterModel *filter = new Akonadi::EntityMimeTypeFilterModel(model);
    filter->setSourceModel(model);
    filter->addMimeTypeExclusionFilter(Akonadi::Collection::mimeType());
    filter->addMimeTypeExclusionFilter(Akonadi::Collection::virtualMimeType());
    filter->setHeaderGroup(Akonadi::EntityTreeModel::ItemListHeaders);

    mSelf = filter;

    return mSelf;
}

ContactCompletionModel::ContactCompletionModel(ChangeRecorder *monitor, QObject *parent)
    : EntityTreeModel(monitor, parent)
{
    setCollectionFetchStrategy(InvisibleCollectionFetch);
}

ContactCompletionModel::~ContactCompletionModel()
{
}

QVariant ContactCompletionModel::entityData(const Item &item, int column, int role) const
{
    if (!item.hasPayload<KContacts::Addressee>()) {
        // Pass modeltest
        if (role == Qt::DisplayRole) {
            return item.remoteId();
        }

        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const KContacts::Addressee contact = item.payload<KContacts::Addressee>();

        switch (column) {
        case NameColumn:
            if (!contact.formattedName().isEmpty()) {
                return contact.formattedName();
            } else {
                return contact.assembledName();
            }
            break;
        case NameAndEmailColumn: {
            QString name = QStringLiteral("%1 %2").arg(contact.givenName())
                           .arg(contact.familyName()).simplified();
            if (name.isEmpty()) {
                name = contact.organization().simplified();
            }
            if (name.isEmpty()) {
                return QString();
            }

            const QString email = contact.preferredEmail().simplified();
            if (email.isEmpty()) {
                return QString();
            }

            return QStringLiteral("%1 <%2>").arg(name).arg(email);
        }
        break;
        case EmailColumn:
            return contact.preferredEmail();
            break;
        }
    }

    return EntityTreeModel::entityData(item, column, role);
}

QVariant ContactCompletionModel::entityData(const Collection &collection, int column, int role) const
{
    return EntityTreeModel::entityData(collection, column, role);
}

int ContactCompletionModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return 3;
    } else {
        return 0;
    }
}

int ContactCompletionModel::entityColumnCount(HeaderGroup) const
{
    return 3;
}

#include "moc_contactcompletionmodel_p.cpp"
