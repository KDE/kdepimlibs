/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef MAILTRANSPORT_TRANSPORTLISTVIEW_H
#define MAILTRANSPORT_TRANSPORTLISTVIEW_H

#include <QTreeWidget>

namespace MailTransport
{

/**
  @internal
  A QTreeWidget for transports.
*/
class TransportListView : public QTreeWidget
{
    Q_OBJECT

public:
    explicit TransportListView(QWidget *parent = 0);
    //virtual ~TransportListView() {}

    // overloaded from QTreeWidget
    void editItem(QTreeWidgetItem *item, int column = 0);

protected Q_SLOTS:
    void commitData(QWidget *editor);

private Q_SLOTS:
    void fillTransportList();

};

} // namespace MailTransport

#endif // MAILTRANSPORT_TRANSPORTLISTVIEW_H
