/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

    Based on code from Kopete (addaccountwizard)

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

#ifndef MAILTRANSPORT_ADDTRANSPORTASSISTANT_H
#define MAILTRANSPORT_ADDTRANSPORTASSISTANT_H

#include <mailtransport/mailtransport_export.h>

#include <KDE/KAssistantDialog>


namespace MailTransport
{


/**
  Assistant to help the user set up a new transport.
*/
class MAILTRANSPORT_EXPORT AddTransportAssistant : public KAssistantDialog
{
  Q_OBJECT

public:
  // TODO docu
  explicit AddTransportAssistant( QWidget *parent = 0 );
  ~AddTransportAssistant();

private slots:
  void typeListClicked();
  void typeListDoubleClicked();

protected slots:
  virtual void accept();
  virtual void next();
  virtual void reject();

private:
  class Private;
  Private * const d;

};


}


#endif

