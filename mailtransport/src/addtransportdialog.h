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

#ifndef MAILTRANSPORT_ADDTRANSPORTDIALOG_H
#define MAILTRANSPORT_ADDTRANSPORTDIALOG_H

#include <QDialog>

namespace MailTransport
{

/**
  @internal

  A dialog for creating a new transport.  It asks the user for the transport
  type and name, and then proceeds to configure the new transport.

  To create a new transport from applications, use
  TransportManager::showNewTransportDialog().

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class AddTransportDialog : public QDialog
{
    Q_OBJECT

public:
    /**
      Creates a new AddTransportDialog.
    */
    explicit AddTransportDialog(QWidget *parent = 0);

    /**
      Destroys the AddTransportDialog.
    */
    virtual ~AddTransportDialog();

    /* reimpl */
    void accept() Q_DECL_OVERRIDE;

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void updateOkButton())
    Q_PRIVATE_SLOT(d, void doubleClicked())
};

} // namespace MailTransport

#endif // MAILTRANSPORT_ADDTRANSPORTDIALOG_H
