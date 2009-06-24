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

#include "akonadiconfigwidget.h"
#include "transportconfigwidget_p.h"
#include "ui_akonadisettings.h"

#include <KStandardDirs>

#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/agentinstance.h>

using namespace Akonadi;
using namespace MailTransport;


class MailTransport::AkonadiConfigWidgetPrivate : public TransportConfigWidgetPrivate
{
  public:
    Ui::AkonadiSettings ui;

};

AkonadiConfigWidget::AkonadiConfigWidget( Transport *transport, QWidget *parent )
  : TransportConfigWidget( *new AkonadiConfigWidgetPrivate, transport, parent )
{
  init();
}

AkonadiConfigWidget::AkonadiConfigWidget( AkonadiConfigWidgetPrivate &dd, Transport *transport, QWidget *parent )
  : TransportConfigWidget( dd, transport, parent )
{
  init();
}

void AkonadiConfigWidget::init()
{
  Q_D( AkonadiConfigWidget );

  d->ui.setupUi( this );
  // The KConfigDialogManager is useless here, it doesn't handle the Akonadi::AgentInstanceWidget
  //d->manager->addWidget( this ); // otherwise it doesn't find out about these widgets
  //d->manager->updateWidgets();

  d->ui.agentInstances->agentFilterProxyModel()->addCapabilityFilter( QLatin1String( "MailTransport" ) );
}

void AkonadiConfigWidget::apply()
{
  Q_D( AkonadiConfigWidget );

  const AgentInstance instance = d->ui.agentInstances->currentAgentInstance();
  if( instance.isValid() ) {
    d->transport->setHost( instance.identifier() );
  }

  TransportConfigWidget::apply();
}


#include "akonadiconfigwidget.moc"
