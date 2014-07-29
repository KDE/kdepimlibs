/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  Based on KMail code by:
  Copyright (c) 2002 Marc Mutz <mutz@kde.org>
  Copyright (c) 2007 Mathias Soeken <msoeken@tzi.de>

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

#include "transportlistview.h"
#include "transport.h"
#include "transportmanager.h"
#include "transporttype.h"

#include <QHeaderView>
#include <QLineEdit>

#include <QDebug>
#include <KLocalizedString>

using namespace MailTransport;

TransportListView::TransportListView(QWidget *parent)
    : QTreeWidget(parent)
{
    setHeaderLabels(QStringList()
                    << i18nc("@title:column email transport name", "Name")
                    << i18nc("@title:column email transport type", "Type"));
    setRootIsDecorated(false);
    header()->setMovable(false);
    header()->setResizeMode(QHeaderView::ResizeToContents);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
    setSelectionMode(SingleSelection);

    fillTransportList();
    connect(TransportManager::self(), SIGNAL(transportsChanged()),
            this, SLOT(fillTransportList()));
}

void TransportListView::editItem(QTreeWidgetItem *item, int column)
{
    // TODO: is there a nicer way to make only the 'name' column editable?
    if (column == 0 && item) {
        Qt::ItemFlags oldFlags = item->flags();
        item->setFlags(oldFlags | Qt::ItemIsEditable);
        QTreeWidget::editItem(item, 0);
        item->setFlags(oldFlags);
        const int id = item->data(0, Qt::UserRole).toInt();
        Transport *t = TransportManager::self()->transportById(id);
        if (!t) {
            qWarning() << "Transport" << id << "not known by manager.";
            return;
        }
        if (TransportManager::self()->defaultTransportId() == t->id()) {
            item->setText(0, t->name());
        }
    }
}

void TransportListView::commitData(QWidget *editor)
{
    if (selectedItems().isEmpty()) {
        // transport was deleted by someone else???
        qDebug() << "No selected item.";
        return;
    }
    QTreeWidgetItem *item = selectedItems().first();
    QLineEdit *edit = dynamic_cast<QLineEdit *>(editor);  // krazy:exclude=qclasses
    Q_ASSERT(edit);   // original code had if

    const int id = item->data(0, Qt::UserRole).toInt();
    Transport *t = TransportManager::self()->transportById(id);
    if (!t) {
        qWarning() << "Transport" << id << "not known by manager.";
        return;
    }
    qDebug() << "Renaming transport" << id << "to" << edit->text();
    t->setName(edit->text());
    t->forceUniqueName();
    t->save();
}

void TransportListView::fillTransportList()
{
    // try to preserve the selection
    int selected = -1;
    if (currentItem()) {
        selected = currentItem()->data(0, Qt::UserRole).toInt();
    }

    clear();
    foreach (Transport *t, TransportManager::self()->transports()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setData(0, Qt::UserRole, t->id());
        QString name = t->name();
        if (TransportManager::self()->defaultTransportId() == t->id()) {
            name += i18nc("@label the default mail transport", " (Default)");
            QFont font(item->font(0));
            font.setBold(true);
            item->setFont(0, font);
        }
        item->setText(0, name);
        item->setText(1, t->transportType().name());
        if (t->id() == selected) {
            setCurrentItem(item);
        }
    }
}
