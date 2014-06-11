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

#include <KMessageBox>
#include <QMenu>


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
    void slotCustomContextMenuRequested( const QPoint & );
};

TransportManagementWidget::Private::Private( TransportManagementWidget *parent )
  : q( parent )
{
}

TransportManagementWidget::TransportManagementWidget( QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  d->ui.setupUi( this );
  d->updateButtonState();

  d->ui.transportList->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( d->ui.transportList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
           SLOT(updateButtonState()) );
  connect( d->ui.transportList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
           SLOT(editClicked()) );
  connect( d->ui.addButton, SIGNAL(clicked()), SLOT(addClicked()) );
  connect( d->ui.editButton, SIGNAL(clicked()), SLOT(editClicked()) );
  connect( d->ui.renameButton, SIGNAL(clicked()), SLOT(renameClicked()) );
  connect( d->ui.removeButton, SIGNAL(clicked()), SLOT(removeClicked()) );
  connect( d->ui.defaultButton, SIGNAL(clicked()), SLOT(defaultClicked()) );
  connect( d->ui.transportList, SIGNAL(customContextMenuRequested(QPoint)),
           SLOT(slotCustomContextMenuRequested(QPoint)) );
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
  if ( !ui.transportList->currentItem() ) {
    return;
  }

  const int currentId = ui.transportList->currentItem()->data( 0, Qt::UserRole ).toInt();
  Transport *transport = TransportManager::self()->transportById( currentId );
  TransportManager::self()->configureTransport( transport, q );
}

void TransportManagementWidget::Private::renameClicked()
{
  if ( !ui.transportList->currentItem() ) {
    return;
  }

  ui.transportList->editItem( ui.transportList->currentItem(), 0 );
}

void TransportManagementWidget::Private::removeClicked()
{
  if ( !ui.transportList->currentItem() ) {
    return;
  }
  const int rc =
    KMessageBox::questionYesNo(
      q,
      i18n( "Do you want to remove outgoing account '%1'?",
            ui.transportList->currentItem()->text( 0 ) ),
      i18n( "Remove outgoing account?" ) );
  if ( rc == KMessageBox::No ) {
    return;
  }

  TransportManager::self()->removeTransport(
        ui.transportList->currentItem()->data( 0, Qt::UserRole ).toInt() );
}

void TransportManagementWidget::Private::defaultClicked()
{
  if ( !ui.transportList->currentItem() ) {
    return;
  }

  TransportManager::self()->setDefaultTransport(
        ui.transportList->currentItem()->data( 0, Qt::UserRole ).toInt() );
}

void TransportManagementWidget::Private::slotCustomContextMenuRequested( const QPoint &pos )
{
  QMenu *menu = new QMenu( q );
  menu->addAction( i18n( "Add..." ), q, SLOT(addClicked()) );
  QTreeWidgetItem *item = ui.transportList->itemAt( pos );
  if ( item ) {
    menu->addAction( i18n( "Modify..." ), q, SLOT(editClicked()) );
    menu->addAction( i18n( "Rename" ), q, SLOT(renameClicked()) );
    menu->addAction( i18n( "Remove" ), q, SLOT(removeClicked()) );
    if ( item->data( 0, Qt::UserRole ) != TransportManager::self()->defaultTransportId() ) {
       menu->addSeparator();
       menu->addAction( i18n( "Set as Default" ), q, SLOT(defaultClicked()) );
    }
  }
  menu->exec( ui.transportList->viewport()->mapToGlobal( pos ) );
  delete menu;
}

#include "moc_transportmanagementwidget.cpp"
