/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>
    Copyright (c) 2012 Laurent Montel <montel@kde.org>

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


#ifndef AKONADI_RECURSIVECOLLECTIONFILTERPROXYMODEL_H
#define AKONADI_RECURSIVECOLLECTIONFILTERPROXYMODEL_H

#include <krecursivefilterproxymodel.h>

#include "akonadi_export.h"

namespace Akonadi
{

class RecursiveCollectionFilterProxyModelPrivate;

/**
 * @short A model to filter out collections of non-matching content types.
 *
 * @author Stephen Kelly <steveire@gmail.com>
 * @since 4.6
 */
class AKONADI_EXPORT RecursiveCollectionFilterProxyModel : public KRecursiveFilterProxyModel
{
  Q_OBJECT

  public:
    /**
     * Creates a new recursive collection filter proxy model.
     *
     * @param parent The parent object.
     */
    RecursiveCollectionFilterProxyModel( QObject *parent = 0 );

    /**
     * Destroys the recursive collection filter proxy model.
     */
    virtual ~RecursiveCollectionFilterProxyModel();

    /**
     * Add content mime type to be shown by the filter.
     *
     * @param mimeType A mime type to be shown.
     */
    void addContentMimeTypeInclusionFilter( const QString &mimeType );

    /**
     * Add content mime types to be shown by the filter.
     *
     * @param mimeTypes A list of content mime types to be included.
     */
    void addContentMimeTypeInclusionFilters( const QStringList &mimeTypes );

    /**
     * Clears the current filters.
     */
    void clearFilters();

    /**
     * Replace the content mime types to be shown by the filter.
     *
     * @param mimeTypes A list of content mime types to be included.
     */
    void setContentMimeTypeInclusionFilters( const QStringList &mimeTypes );

    /**
     * Returns the currently included mimetypes in the filter.
     */
    QStringList contentMimeTypeInclusionFilters() const;

    /**
     * Add search pattern
     * @since 4.8.1
     */
    void setSearchPattern( const QString &pattern );

    /**
     * Show only checked item
     * @since 4.9
     */
    void setIncludeCheckedOnly( bool checked );

  protected:
    /* reimp */ bool acceptRow( int sourceRow, const QModelIndex &sourceParent ) const;
    /* reimp */ int columnCount( const QModelIndex& index ) const;

  protected:
    RecursiveCollectionFilterProxyModelPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(RecursiveCollectionFilterProxyModel)
};

}

#endif
