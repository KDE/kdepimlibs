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
#include "transportconfigdialog.h"

using namespace MailTransport;

class TransportManagementWidget::Private
{
  public:
    Ui::TransportManagementWidget ui;
};

TransportManagementWidget::TransportManagementWidget( QWidget *parent )
  : QWidget( parent ), d( new Private )
{
  KGlobal::locale()->insertCatalog(QString::fromLatin1("libmailtransport"));
  d->ui.setupUi( this );

  d->ui.transportList->setHeaderLabels( QStringList()
                                        << i18nc( "@title:column email transport name", "Name" )
                                        << i18nc( "@title:column email transport type", "Type" ) );
  d->ui.transportList->sortItems( 0, Qt::AscendingOrder );
  connect( d->ui.transportList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
           SLOT(updateButtonState()) );
  connect( d->ui.transportList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
           SLOT(editClicked()) );
  connect( d->ui.addButton, SIGNAL(clicked()), SLOT(addClicked()) );
  connect( d->ui.editButton, SIGNAL(clicked()), SLOT(editClicked()) );
  connect( d->ui.removeButton, SIGNAL(clicked()), SLOT(removeClicked()) );
  connect( d->ui.defaultButton, SIGNAL(clicked()), SLOT(defaultClicked()) );

  fillTransportList();
  connect( TransportManager::self(), SIGNAL(transportsChanged()),
           SLOT(fillTransportList()) );
}

TransportManagementWidget::~TransportManagementWidget()
{
  delete d;
}

void TransportManagementWidget::fillTransportList()
{
  // try to preserve the selection
  int selected = -1;
  if ( d->ui.transportList->currentItem() ) {
    selected = d->ui.transportList->currentItem()->data( 0, Qt::UserRole ).toInt();
  }

  d->ui.transportList->clear();
  foreach ( Transport *t, TransportManager::self()->transports() ) {
    QTreeWidgetItem *item = new QTreeWidgetItem( d->ui.transportList );
    item->setData( 0, Qt::UserRole, t->id() );
    item->setText( 0, t->name() );
    QString type;
    switch ( t->type() ) {
    case Transport::EnumType::SMTP:
      type = i18nc( "@option SMTP transport", "SMTP" );
      break;
    case Transport::EnumType::Sendmail:
      type = i18nc( "@option sendmail transport", "Sendmail" );
      break;
    }
    if ( TransportManager::self()->defaultTransportId() == t->id() ) {
      type += i18nc( "@label the default mail transport", " (Default)" );
    }
    item->setText( 1, type );
    if ( t->id() == selected ) {
      d->ui.transportList->setCurrentItem( item );
    }
  }

  updateButtonState();
}

void TransportManagementWidget::updateButtonState()
{
  if ( !d->ui.transportList->currentItem() ) {
    d->ui.editButton->setEnabled( false );
    d->ui.removeButton->setEnabled( false );
    d->ui.defaultButton->setEnabled( false );
  } else {
    d->ui.editButton->setEnabled( true );
    d->ui.removeButton->setEnabled( true );
    if ( d->ui.transportList->currentItem()->data( 0, Qt::UserRole ) ==
         TransportManager::self()->defaultTransportId() ) {
      d->ui.defaultButton->setEnabled( false );
    } else {
      d->ui.defaultButton->setEnabled( true );
    }
  }
}

void TransportManagementWidget::addClicked()
{
  // initialize transport
  Transport *t = TransportManager::self()->createTransport();
  t->setType( Transport::EnumType::SMTP );

  // configure transporr
  TransportConfigDialog *tcd = new TransportConfigDialog( t, this );
  connect( tcd, SIGNAL(sendmailClicked()), SLOT(slotSendmail()) );
  tcd->setCaption( i18nc( "@title:window", "Add Transport" ) );
  if ( tcd->exec() == KDialog::Accepted ) {
    TransportManager::self()->addTransport( t );
  } else {
    delete t;
  }
}

void TransportManagementWidget::slotSendmail()
{
  // initialize transport
  Transport *t = TransportManager::self()->createTransport();
  t->setType( Transport::EnumType::Sendmail );
  t->setHost( QLatin1String( "/usr/sbin/sendmail" ) );

  TransportConfigDialog tcd( t, this );
  tcd.setCaption( i18nc( "@title:window", "Add Transport" ) );
  if ( tcd.exec() == KDialog::Accepted ) {
    TransportManager::self()->addTransport( t );
  } else {
    delete t;
  }
}

void TransportManagementWidget::editClicked()
{
  Q_ASSERT( d->ui.transportList->currentItem() );

  int currentId = d->ui.transportList->currentItem()->data( 0, Qt::UserRole ).toInt();
  Transport *transport = TransportManager::self()->transportById( currentId );
  if ( !transport ) {
    return;
  }
  transport = transport->clone();
  TransportConfigDialog t( transport, this );
  t.setCaption( i18nc( "@title:window", "Modify Transport" ) );
  t.exec();
  delete transport;
}

void TransportManagementWidget::removeClicked()
{
  Q_ASSERT( d->ui.transportList->currentItem() );

  TransportManager::self()->removeTransport(
        d->ui.transportList->currentItem()->data( 0, Qt::UserRole ).toInt() );
}

void TransportManagementWidget::defaultClicked()
{
  Q_ASSERT( d->ui.transportList->currentItem() );

  TransportManager::self()->setDefaultTransport(
        d->ui.transportList->currentItem()->data( 0, Qt::UserRole ).toInt() );
}

#include "transportmanagementwidget.moc"
