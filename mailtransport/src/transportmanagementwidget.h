/*
  Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

  Based on KMail code by:
  Copyright (C) 2001-2003 Marc Mutz <mutz@kde.org>

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

#ifndef MAILTRANSPORT_TRANSPORTMANAGEMENTWIDGET_H
#define MAILTRANSPORT_TRANSPORTMANAGEMENTWIDGET_H

#include <mailtransport_export.h>

#include <QWidget>

namespace MailTransport
{

/**
  A widget to manage mail transports.
*/
class MAILTRANSPORT_EXPORT TransportManagementWidget : public QWidget
{
    Q_OBJECT

public:
    /**
      Creates a new TransportManagementWidget.
      @param parent The parent widget.
    */
    TransportManagementWidget(QWidget *parent = 0);

    /**
      Destroys the widget.
    */
    virtual ~TransportManagementWidget();

private:
    class Private;
    Private *const d;
    Q_PRIVATE_SLOT(d, void defaultClicked())
    Q_PRIVATE_SLOT(d, void removeClicked())
    Q_PRIVATE_SLOT(d, void renameClicked())
    Q_PRIVATE_SLOT(d, void editClicked())
    Q_PRIVATE_SLOT(d, void addClicked())
    Q_PRIVATE_SLOT(d, void updateButtonState())
    Q_PRIVATE_SLOT(d, void slotCustomContextMenuRequested(const QPoint &))
};

} // namespace MailTransport

#endif // MAILTRANSPORT_TRANSPORTMANAGEMENTWIDGET_H
