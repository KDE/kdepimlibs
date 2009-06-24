/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2007 KovoKs <kovoks@kovoks.nl>
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "transportconfigdialog.h"

#include "transport.h"
#include "transportconfigwidget.h"
#include "transportmanager.h"
#include "transporttypeinfo.h"

#include <QString>

#include <KDebug>

using namespace MailTransport;

class MailTransport::TransportConfigDialog::Private
{
  public:
    TransportConfigWidget *configWidget;
    // TODO not really necessary right now; used only in constructor

};

TransportConfigDialog::TransportConfigDialog( Transport *transport, QWidget *parent )
  : KDialog( parent ), d( new Private )
{
  Q_ASSERT( transport );

  d->configWidget = TransportTypeInfo::configWidgetForTransport( transport );
  kDebug() << "transport" << transport->id() << "config widget" << d->configWidget;
  Q_ASSERT( d->configWidget );
  setMainWidget( d->configWidget );

  setButtons( Ok|Cancel );
  connect( this, SIGNAL( okClicked() ), d->configWidget, SLOT( apply() ) );
}

TransportConfigDialog::~ TransportConfigDialog()
{
  delete d;
}

#include "transportconfigdialog.moc"
