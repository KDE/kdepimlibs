/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_MESSAGEMODEL_H
#define AKONADI_MESSAGEMODEL_H

#include "akonadi-mime_export.h"

#include <itemmodel.h>
#include <job.h>

#if defined(MAKE_AKONADI_KMIME_LIB)
#  define AKONADI_IGNORE_DEPRECATED_WARNINGS
#endif

namespace Akonadi
{

/**
 * A flat self-updating message model.
 * @deprecated Subclass Akonadi::EntityTreeModel instead. An example can be seen in AkonadiConsole,
 *             have a look at AkonadiBrowserModel there.
 */
class AKONADI_MIME_DEPRECATED_EXPORT MessageModel : public Akonadi::ItemModel
{
    Q_OBJECT

public:
    /**
      Column types.
    */
    enum Column {
        Subject, /**< Subject column. */
        Sender, /**< Sender column. */
        Receiver, /**< Receiver column. */
        Date, /**< Date column. */
        Size /**< Size column. */
    };

    /**
      Creates a new message model.

      @param parent The parent object.
    */
    explicit MessageModel(QObject *parent = Q_NULLPTR);

    /**
      Deletes the message model.
    */
    virtual ~MessageModel();

    /**
      Reimplemented from QAbstractItemModel.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    /**
      Reimplemented from QAbstractItemModel.
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    /**
      Reimplemented from QAbstractItemModel.
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    /**
      Reimplemented from QAbstractItemModel.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    /**
      Reimplemented from QAbstractItemModel.
     */
    QStringList mimeTypes() const Q_DECL_OVERRIDE;
private:
    class Private;
    Private *const d;
};

}

#endif
