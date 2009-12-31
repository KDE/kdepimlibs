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

#include "transportmanagementwidget.h"
#include "ui_transportmanagementwidget.h"
#include "transportmanager.h"
#include "transport.h"

using namespace MailTransport;

class TransportManagementWidget::Private
{
  public:

    Private( TransportManagementWidget *parent );

    Ui::TransportManagementWidget ui;
    TransportManagementWidget *q;

    // Slots
    void defaultClicked();
    void removeClicked();
    void renameClicked();
    void editClicked();
    void addClicked();
    void updateButtonState();
};

TransportManagementWidget::Private::Private( TransportManagementWidget *parent )
  : q( parent )
{
}

TransportManagementWidget::TransportManagementWidget( QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  KGlobal::locale()->insertCatalog( QString::fromLatin1( "libmailtransport" ) );
  d->ui.setupUi( this );
  d->updateButtonState();

  connect( d->ui.transportList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
           SLOT(updateButtonState()) );
  connect( d->ui.transportList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
           SLOT(editClicked()) );
  connect( d->ui.addButton, SIGNAL(clicked()), SLOT(addClicked()) );
  connect( d->ui.editButton, SIGNAL(clicked()), SLOT(editClicked()) );
  connect( d->ui.renameButton, SIGNAL(clicked()), SLOT(renameClicked()) );
  connect( d->ui.removeButton, SIGNAL(clicked()), SLOT(removeClicked()) );
  connect( d->ui.defaultButton, SIGNAL(clicked()), SLOT(defaultClicked()) );
}

TransportManagementWidget::~TransportManagementWidget()
{
  delete d;
}

void TransportManagementWidget::Private::updateButtonState()
{
  // TODO figure out current item vs. selected item (in almost every function)
  if ( !ui.transportList->currentItem() ) {
    ui.editButton->setEnabled( false );
    ui.renameButton->setEnabled( false );
    ui.removeButton->setEnabled( false );
    ui.defaultButton->setEnabled( false );
  } else {
    ui.editButton->setEnabled( true );
    ui.renameButton->setEnabled( true );
    ui.removeButton->setEnabled( true );
    if ( ui.transportList->currentItem()->data( 0, Qt::UserRole ) ==
         TransportManager::self()->defaultTransportId() ) {
      ui.defaultButton->setEnabled( false );
    } else {
      ui.defaultButton->setEnabled( true );
    }
  }
}

void TransportManagementWidget::Private::addClicked()
{
  TransportManager::self()->showTransportCreationDialog( q );
}

void TransportManagementWidget::Private::editClicked()
{
  if( !ui.transportList->currentItem() ) {
    return;
  }

  int currentId = ui.transportList->currentItem()->data( 0, Qt::UserRole ).toInt();
  Transport *transport = TransportManager::self()->transportById( currentId );
  TransportManager::self()->configureTransport( transport, q );
}

void TransportManagementWidget::Private::renameClicked()
{
  if( !ui.transportList->currentItem() ) {
    return;
  }

  ui.transportList->editItem( ui.transportList->currentItem(), 0 );
}

void TransportManagementWidget::Private::removeClicked()
{
  if( !ui.transportList->currentItem() ) {
    return;
  }

  TransportManager::self()->removeTransport(
        ui.transportList->currentItem()->data( 0, Qt::UserRole ).toInt() );
}

void TransportManagementWidget::Private::defaultClicked()
{
  if( !ui.transportList->currentItem() ) {
    return;
  }

  TransportManager::self()->setDefaultTransport(
        ui.transportList->currentItem()->data( 0, Qt::UserRole ).toInt() );
}

#include "transportmanagementwidget.moc"
