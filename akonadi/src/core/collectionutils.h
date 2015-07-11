/*
    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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

#ifndef AKONADI_COLLECTIONUTILS_P_H
#define AKONADI_COLLECTIONUTILS_P_H

#include <QtCore/QStringList>

#include "entitydisplayattribute.h"
#include "collectionstatistics.h"
#include "item.h"

namespace Akonadi
{

/**
 * @internal
 */
namespace CollectionUtils
{
inline bool isVirtualParent(const Collection &collection)
{
    return (collection.parentCollection() == Collection::root() && collection.isVirtual());
}

inline bool isReadOnly(const Collection &collection)
{
    return !(collection.rights() &Collection::CanCreateItem);
}

inline bool isRoot(const Collection &collection)
{
    return (collection == Collection::root());
}

inline bool isResource(const Collection &collection)
{
    return (collection.parentCollection() == Collection::root());
}

inline bool isStructural(const Collection &collection)
{
    return collection.contentMimeTypes().isEmpty();
}

inline bool isFolder(const Collection &collection)
{
    return (!isRoot(collection) &&
            !isResource(collection) &&
            !isStructural(collection) &&
            collection.resource() != QLatin1String("akonadi_search_resource") &&
            collection.resource() != QLatin1String("akonadi_nepomuktag_resource"));
}

inline QString defaultIconName(const Collection &col)
{
    if (CollectionUtils::isVirtualParent(col)) {
        return QStringLiteral("edit-find");
    }
    if (col.isVirtual()) {
        return QStringLiteral("document-preview");
    }
    if (CollectionUtils::isResource(col)) {
        return QStringLiteral("network-server");
    }
    if (CollectionUtils::isStructural(col)) {
        return QStringLiteral("folder-grey");
    }
    if (CollectionUtils::isReadOnly(col)) {
        return QStringLiteral("folder-grey");
    }

    const QStringList content = col.contentMimeTypes();
    if ((content.size() == 1) ||
            (content.size() == 2 && content.contains(Collection::mimeType()))) {
        if (content.contains(QStringLiteral("text/x-vcard")) ||
                content.contains(QStringLiteral("text/directory")) ||
                content.contains(QStringLiteral("text/vcard"))) {
            return QStringLiteral("x-office-address-book");
        }
        // TODO: add all other content types and/or fix their mimetypes
        if (content.contains(QStringLiteral("akonadi/event")) || content.contains(QStringLiteral("text/ical"))) {
            return QStringLiteral("view-pim-calendar");
        }
        if (content.contains(QStringLiteral("akonadi/task"))) {
            return QStringLiteral("view-pim-tasks");
        }
    } else if (content.isEmpty()) {
        return QStringLiteral("folder-grey");
    }
    return QStringLiteral("folder");
}
inline QString displayIconName(const Collection &col)
{
    QString iconName = defaultIconName(col);
    if (col.hasAttribute<EntityDisplayAttribute>() &&
            !col.attribute<EntityDisplayAttribute>()->iconName().isEmpty()) {
        if (!col.attribute<EntityDisplayAttribute>()->activeIconName().isEmpty() && col.statistics().unreadCount() > 0) {
            iconName = col.attribute<EntityDisplayAttribute>()->activeIconName();
        } else {
            iconName = col.attribute<EntityDisplayAttribute>()->iconName();
        }
    }
    return iconName;

}
inline bool hasValidHierarchicalRID(const Collection &col)
{
    if (col == Collection::root()) {
        return true;
    }
    if (col.remoteId().isEmpty()) {
        return false;
    }
    return hasValidHierarchicalRID(col.parentCollection());
}
inline bool hasValidHierarchicalRID(const Item &item)
{
    return !item.remoteId().isEmpty() && hasValidHierarchicalRID(item.parentCollection());
}
}

}

#endif
