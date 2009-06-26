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

#include "addtransportdialog.h"
#include "transport.h"
#include "transportconfigwidget.h"
#include "transportmanager.h"
#include "transporttype.h"
#include "ui_addtransportdialog.h"

#include <KConfigDialogManager>
#include <KConfigSkeleton>
#include <KDebug>

#include <akonadi/agentinstance.h>
#include <akonadi/agentinstancecreatejob.h>

using namespace MailTransport;

/**
  @internal
*/
class AddTransportDialog::Private
{
  public:
    Private( AddTransportDialog *qq )
      : q( qq )
    {
    }

    TransportType selectedType() const;

    // slots
    void typeListClicked();

    AddTransportDialog *const q;
    Ui::AddTransportDialog ui;
};

TransportType AddTransportDialog::Private::selectedType() const
{
  QList<QTreeWidgetItem*> sel = ui.typeListView->selectedItems();
  if( !sel.empty() ) {
    return sel.first()->data( 0, Qt::UserRole ).value<TransportType>();
  }
  return TransportType();
}

void AddTransportDialog::Private::typeListClicked()
{
  // Make sure a type is selected before allowing the user to continue.
  q->enableButtonOk( selectedType().isValid() );
}



AddTransportDialog::AddTransportDialog( QWidget *parent )
  : KDialog( parent )
  , d( new Private( this ) )
{
  // Setup UI.
  {
    QWidget *widget = new QWidget( this );
    d->ui.setupUi( widget );
    setMainWidget( widget );
    setCaption( i18n( "Create Outgoing Account" ) );
    setButtons( Ok|Cancel );
    enableButtonOk( false );
    setButtonText( Ok, i18nc( "create and configure a mail transport", "Create and Configure" ) );
  }

  // Populate type list.
  foreach( const TransportType &type, TransportManager::self()->types() ) {
    QTreeWidgetItem *treeItem = new QTreeWidgetItem( d->ui.typeListView );
    treeItem->setText( 0, type.name() );
    treeItem->setText( 1, type.description() );
    treeItem->setData( 0, Qt::UserRole, QVariant::fromValue( type ) ); // the transport type
  }
  d->ui.typeListView->resizeColumnToContents( 0 );
  updateGeometry();
  d->ui.typeListView->setFocus();

  // Connect user input.
  connect( d->ui.typeListView, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
      this, SLOT(typeListClicked()) );
  connect( d->ui.typeListView, SIGNAL(itemSelectionChanged()),
      this, SLOT(typeListClicked()) );
}

AddTransportDialog::~AddTransportDialog()
{
}

void AddTransportDialog::accept()
{
  if( !d->selectedType().isValid() ) {
    return;
  }

  // Create a new transport and configure it.
  Transport *transport = TransportManager::self()->createTransport();
  transport->setTransportType( d->selectedType() );
  if( d->selectedType().type() == Transport::EnumType::Akonadi ) {
    // Create a resource instance if Akonadi-type transport.
    using namespace Akonadi;
    AgentInstanceCreateJob *cjob = new AgentInstanceCreateJob( d->selectedType().agentType() );
    if( !cjob->exec() ) {
      kWarning() << "Failed to create agent instance of type"
        << d->selectedType().agentType().identifier();
      return;
    }
    transport->setHost( cjob->instance().identifier() );
  }
  transport->setName( d->ui.name->text() );
  transport->forceUniqueName();
  if( TransportManager::self()->configureTransport( transport, this ) ) {
    // The user clicked OK and the transport settings were saved.
    TransportManager::self()->addTransport( transport );
    if( d->ui.setDefault->isChecked() ) {
      TransportManager::self()->setDefaultTransport( transport->id() );
    }
    KDialog::accept();
  }
}

#include "addtransportdialog.moc"
