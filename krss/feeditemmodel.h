/*
 * This file is part of the krss library
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef KRSS_FEEDITEMMODEL_H
#define KRSS_FEEDITEMMODEL_H

#include <krss/krss_export.h>

#include <akonadi/entitytreemodel.h>

class KJob;

namespace KRss {

class FeedItemModelPrivate;

class KRSS_EXPORT FeedItemModel : public Akonadi::EntityTreeModel
{
    Q_OBJECT

public:

    enum ItemColumn {
        ItemTitleColumn = 0,
        AuthorsColumn,
        DateColumn,
        FeedTitleForItemColumn,
        ItemColumnCount
    };

    enum FeedColumn {
        FeedTitleColumn=0,
        UnreadCountColumn,
        TotalCountColumn,
        FeedColumnCount
    };

    enum FeedRoles {
        HasFetchErrorRole=EntityTreeModel::UserRole,
        FetchErrorStringRole
    };

    enum ItemRoles {
        SortRole=FetchErrorStringRole + 1,
        IsUnreadRole,
        IsReadRole,
        IsDeletedRole,
        IsImportantRole,
        LinkRole,
        IsFolderRole
    };

public:

    explicit FeedItemModel( Akonadi::ChangeRecorder* monitor, QObject* parent = 0 );
    ~FeedItemModel();

    /* reimp */ QVariant entityData( const Akonadi::Item& item, int column, int role=Qt::DisplayRole ) const;

    /* reimp */ QVariant entityData( const Akonadi::Collection& collection, int column, int role=Qt::DisplayRole ) const;

    /* reimp */ int entityColumnCount( EntityTreeModel::HeaderGroup headerSet ) const;

    /* reimp */ QVariant entityHeaderData( int section, Qt::Orientation orientation, int role, EntityTreeModel::HeaderGroup headerSet ) const;

private:
    friend class ::KRss::FeedItemModelPrivate;
    FeedItemModelPrivate* const d;
};

} // namespace KRss

#endif // KRSS_FEEDITEMMODEL_H
