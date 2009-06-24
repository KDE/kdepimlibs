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

#include "addtransportassistant.h"

#include <KConfigDialogManager>
#include <KConfigSkeleton>
#include <KDebug>
#include <KVBox>

#include "transportconfigwidget.h"
#include "transport.h"
#include "transportbase.h"
#include "transportmanager.h"

#include "ui_addtransportassistanttypepage.h"
#include "ui_addtransportassistantnamepage.h"


using namespace MailTransport;


/**
  @internal
*/
class AddTransportAssistant::Private
{
  public:
    Private()
      : typePage( 0 )
      , configPage( 0 )
      , namePage( 0 )
      , configPageContents( 0 )
      , transport( 0 )
      , lastType( -1 )
    {
    }

    QTreeWidgetItem* selectedType();

    KPageWidgetItem *typeItem;
    KPageWidgetItem *configItem;
    KPageWidgetItem *nameItem;
    QWidget *typePage;
    KVBox *configPage;
    QWidget *namePage;
    TransportConfigWidget *configPageContents;
    Transport *transport;
    int lastType;
    Ui::AddTransportAssistantTypePage uiTypePage;
    Ui::AddTransportAssistantNamePage uiNamePage;

};

AddTransportAssistant::AddTransportAssistant( QWidget *parent )
  : KAssistantDialog( parent )
  , d( new Private )
{
  // type page
  d->typePage = new QWidget( this );
  d->uiTypePage.setupUi( d->typePage );
  d->uiTypePage.typeListView->setColumnCount( 2 );
  QStringList header;
  header << i18n( "Type" ) << i18n( "Description" );
  d->uiTypePage.typeListView->setHeaderLabels( header );

  d->typeItem = addPage( d->typePage, d->typePage->windowTitle() );
  setValid( d->typeItem, false );

  // populate type list
  // TODO: HACKish way to get transport descriptions...
  // TODO TransportManagementWidget has i18ns for transport types -> share?
  Q_ASSERT( d->transport == 0 );
  d->transport = TransportManager::self()->createTransport();
  Q_ASSERT( d->transport );
  int enumid = 0;
  const KConfigSkeleton::ItemEnum *const item = d->transport->typeItem();
  foreach( const KConfigSkeleton::ItemEnum::Choice2 &choice, item->choices2() ) {
    QTreeWidgetItem *treeItem = new QTreeWidgetItem( d->uiTypePage.typeListView );
    treeItem->setData( 0, Qt::UserRole, enumid ); // the transport type
    enumid++;
    treeItem->setText( 0, choice.label );
    treeItem->setText( 1, choice.whatsThis );
  }
  d->uiTypePage.typeListView->resizeColumnToContents( 0 );
  d->uiTypePage.typeListView->setFocus();

  // connect user input
  connect( d->uiTypePage.typeListView, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
      this, SLOT( typeListClicked() ) );
  connect( d->uiTypePage.typeListView, SIGNAL( itemSelectionChanged() ),
      this, SLOT( typeListClicked() ) );
  connect( d->uiTypePage.typeListView, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
      this, SLOT( typeListDoubleClicked() ) );

  // settings page
  d->configPage = new KVBox( this );
  d->configItem = addPage( d->configPage, i18n( "Step Two: Transport Settings" ) );

  // name page
  d->namePage = new QWidget( this );
  d->uiNamePage.setupUi( d->namePage );
  // TODO set up sensible default name
  d->nameItem = addPage( d->namePage, d->namePage->windowTitle() );
}

QTreeWidgetItem* AddTransportAssistant::Private::selectedType()
{
  QList<QTreeWidgetItem*> sel = uiTypePage.typeListView->selectedItems();
  if( !sel.empty() )
     return sel.first();
  return 0;
}

void AddTransportAssistant::typeListClicked()
{
  // Make sure a type is selected before allowing the user to continue.
  setValid( d->typeItem, d->selectedType() != 0 );
}

void AddTransportAssistant::typeListDoubleClicked()
{
  // Proceed to the next page if a type is double clicked.
  next();
}

void AddTransportAssistant::accept()
{
  // register transport
  d->configPageContents->apply();
  TransportManager::self()->addTransport( d->transport );
  d->transport = 0; // don't delete it
  if( d->uiNamePage.setDefault->isChecked() ) {
    TransportManager::self()->setDefaultTransport( d->transport->id() );
  }
  KAssistantDialog::accept();
}

void AddTransportAssistant::next()
{
  if( currentPage() == d->typeItem ) {
    QTreeWidgetItem *item = d->selectedType();
    int type = item->data( 0, Qt::UserRole ).toInt(); // the transport type
    Q_ASSERT( type >= 0 && type < TransportBase::EnumType::COUNT );

    // create appropriate config widget
    if( d->configPageContents && d->lastType == type ) {
      // same type as before; keep settings
    } else {
      d->transport->setType( type );
      d->lastType = type;
      delete d->configPageContents;
      d->configPageContents = TransportManager::self()->configWidgetForTransport( d->transport, d->configPage );

      // let the configWidget's KConfigDialogManager handle kcfg_name:
      KConfigDialogManager *mgr = d->configPageContents->configManager();
      Q_ASSERT( mgr );
      Q_ASSERT( d->namePage );
      mgr->addWidget( d->namePage );
    }
  }

  KAssistantDialog::next();
}

void AddTransportAssistant::reject()
{
  delete d->transport;
  d->transport = 0;
  KAssistantDialog::reject();
}

AddTransportAssistant::~AddTransportAssistant()
{
  delete d->transport;
  delete d;
}


#include "addtransportassistant.moc"

