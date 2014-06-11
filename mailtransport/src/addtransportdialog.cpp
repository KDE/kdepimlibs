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

#include "addtransportdialog.h"
#include "transport.h"
#include "transportconfigwidget.h"
#include "transportmanager.h"
#include "transporttype.h"
#include "ui_addtransportdialog.h"

#include <QDebug>


#include <agentinstance.h>
#include <agentinstancecreatejob.h>

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

    /**
      Returns the currently selected type in the type selection widget, or
      an invalid type if none is selected.
    */
    TransportType selectedType() const;

    /**
      Enables the OK button if a type is selected.
    */
    void updateOkButton(); // slot
    void doubleClicked(); //slot
    void writeConfig();
    void readConfig();

    AddTransportDialog *const q;
    ::Ui::AddTransportDialog ui;
};


void AddTransportDialog::Private::writeConfig()
{
  KConfigGroup group( KSharedConfig::openConfig(), "AddTransportDialog" );
  group.writeEntry( "Size", q->size() );
}

void AddTransportDialog::Private::readConfig()
{
  KConfigGroup group( KSharedConfig::openConfig(), "AddTransportDialog" );
  const QSize sizeDialog = group.readEntry( "Size", QSize(300,200) );
  if ( sizeDialog.isValid() ) {
    q->resize( sizeDialog );
  }
}

TransportType AddTransportDialog::Private::selectedType() const
{
  QList<QTreeWidgetItem*> sel = ui.typeListView->selectedItems();
  if ( !sel.empty() ) {
    return sel.first()->data( 0, Qt::UserRole ).value<TransportType>();
  }
  return TransportType();
}

void AddTransportDialog::Private::doubleClicked()
{
  if (selectedType().isValid() && !ui.name->text().trimmed().isEmpty()) {
    q->accept();
  }
}

void AddTransportDialog::Private::updateOkButton()
{
  // Make sure a type is selected before allowing the user to continue.
  q->enableButtonOk( selectedType().isValid() && !ui.name->text().trimmed().isEmpty() );
}

AddTransportDialog::AddTransportDialog( QWidget *parent )
  : KDialog( parent ), d( new Private( this ) )
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

#ifdef KDEPIM_MOBILE_UI
    d->ui.descLabel->hide();
    d->ui.setDefault->hide();
#endif
  }

  // Populate type list.
  foreach ( const TransportType &type, TransportManager::self()->types() ) {
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
      this, SLOT(updateOkButton()) );
  connect( d->ui.typeListView, SIGNAL(itemSelectionChanged()),
      this, SLOT(updateOkButton()) );
  connect( d->ui.typeListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
           this, SLOT(doubleClicked()) );
  connect( d->ui.name, SIGNAL(textChanged(QString)),
           this, SLOT(updateOkButton()) );
  d->readConfig();
}

AddTransportDialog::~AddTransportDialog()
{
  d->writeConfig();
  delete d;
}

void AddTransportDialog::accept()
{
  if ( !d->selectedType().isValid() ) {
    return;
  }

  // Create a new transport and configure it.
  Transport *transport = TransportManager::self()->createTransport();
  transport->setTransportType( d->selectedType() );
  if ( d->selectedType().type() == Transport::EnumType::Akonadi ) {
    // Create a resource instance if Akonadi-type transport.
    using namespace Akonadi;
    AgentInstanceCreateJob *cjob = new AgentInstanceCreateJob( d->selectedType().agentType() );
    if ( !cjob->exec() ) {
      qWarning() << "Failed to create agent instance of type"
        << d->selectedType().agentType().identifier();
      return;
    }
    transport->setHost( cjob->instance().identifier() );
  }
  transport->setName( d->ui.name->text().trimmed() );
  transport->forceUniqueName();
  if ( TransportManager::self()->configureTransport( transport, this ) ) {
    // The user clicked OK and the transport settings were saved.
    TransportManager::self()->addTransport( transport );
#ifndef KDEPIM_MOBILE_UI
    if ( d->ui.setDefault->isChecked() ) {
      TransportManager::self()->setDefaultTransport( transport->id() );
    }
#endif
    KDialog::accept();
  }
}

#include "moc_addtransportdialog.cpp"
