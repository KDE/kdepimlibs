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

#ifndef MAILTRANSPORT_AKONADICONFIGWIDGET_H
#define MAILTRANSPORT_AKONADICONFIGWIDGET_H

#include "transportconfigwidget.h"

namespace MailTransport {

class Transport;

/**
  @internal
*/
class AkonadiConfigWidgetPrivate;

/**
  @internal
  Configuration widget for an Akonadi transport.
*/
class AkonadiConfigWidget : public TransportConfigWidget
{
  Q_OBJECT

  public:
    explicit AkonadiConfigWidget( Transport *transport, QWidget *parent = 0 );
    //virtual ~AkonadiConfigWidget();

  public Q_SLOTS:
    /** reimpl */
    virtual void apply();

  protected:
    AkonadiConfigWidget( AkonadiConfigWidgetPrivate &dd, Transport *transport, QWidget *parent );

  private:
    Q_DECLARE_PRIVATE( AkonadiConfigWidget )

    void init();

};

}

#endif
