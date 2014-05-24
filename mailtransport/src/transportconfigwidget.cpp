/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

    Based on MailTransport code by:
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2007 KovoKs <kovoks@kovoks.nl>

    Based on KMail code by:
    Copyright (c) 2001-2002 Michael Haeckel <haeckel@kde.org>

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

#include "transportconfigwidget.h"
#include "transportconfigwidget_p.h"
#include "transport.h"
#include "transportmanager.h"

#include <KConfigDialogManager>
#include <QDebug>

using namespace MailTransport;

TransportConfigWidget::TransportConfigWidget( Transport *transport, QWidget *parent )
  : QWidget( parent ), d_ptr( new TransportConfigWidgetPrivate )
{
  init( transport );
}

TransportConfigWidget::TransportConfigWidget( TransportConfigWidgetPrivate &dd,
                                              Transport *transport, QWidget *parent )
  : QWidget( parent ), d_ptr( &dd )
{
  init( transport );
}

TransportConfigWidget::~ TransportConfigWidget()
{
  delete d_ptr;
}

void TransportConfigWidget::init( Transport *transport )
{
  Q_D( TransportConfigWidget );
  qDebug() << "this" << this << "d" << d;
  Q_ASSERT( transport );
  d->transport = transport;

  d->manager = new KConfigDialogManager( this, transport );
  //d->manager->updateWidgets(); // no-op; ui is set up in subclasses.
}

KConfigDialogManager *TransportConfigWidget::configManager() const
{
  Q_D( const TransportConfigWidget );
  Q_ASSERT( d->manager );
  return d->manager;
}

void TransportConfigWidget::apply()
{
  Q_D( TransportConfigWidget );
  d->manager->updateSettings();
  d->transport->forceUniqueName();
  d->transport->save();
  qDebug() << "Config written.";
}

