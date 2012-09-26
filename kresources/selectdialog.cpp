/*
    This file is part of libkresources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "selectdialog.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <QGroupBox>
#include <QLayout>
#include <QListWidget>

#include "resource.h"

using namespace KRES;

class SelectDialog::SelectDialogPrivate
{
  public:
    QListWidget *mResourceId;
    QMap<int, Resource*> mResourceMap;
};

static bool resourceNameLessThan( Resource *a, Resource *b )
{
  return a->resourceName() < b->resourceName();
}

SelectDialog::SelectDialog( QList<Resource *> list, QWidget *parent )
  : KDialog( parent ), d( new SelectDialogPrivate )
{
  setModal( true );
  setCaption( i18n( "Resource Selection" ) );
  resize( 300, 200 );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );

  QWidget *widget = new QWidget( this );
  setMainWidget( widget );

  QVBoxLayout *mainLayout = new QVBoxLayout( widget );
  mainLayout->setMargin( 0 );

  QGroupBox *groupBox = new QGroupBox( widget );
  QGridLayout *grid = new QGridLayout;
  groupBox->setLayout( grid );
  groupBox->setTitle( i18n( "Resources" ) );

  d->mResourceId = new QListWidget( groupBox );
  grid->addWidget( d->mResourceId, 0, 0 );

  mainLayout->addWidget( groupBox );

  // sort resources by name
  qSort( list.begin(), list.end(), resourceNameLessThan );

  // setup listbox
  uint counter = 0;
  for ( int i = 0; i < list.count(); ++i ) {
    Resource *resource = list.at( i );
    if ( resource && !resource->readOnly() ) {
      d->mResourceMap.insert( counter, resource );
      d->mResourceId->addItem( resource->resourceName() );
      counter++;
    }
  }

  d->mResourceId->setCurrentRow( 0 );
  connect( d->mResourceId, SIGNAL(itemActivated(QListWidgetItem*)),
           SLOT(accept()) );
}

SelectDialog::~SelectDialog()
{
  delete d;
}

Resource *SelectDialog::resource()
{
  if ( d->mResourceId->currentRow() != -1 ) {
    return d->mResourceMap[ d->mResourceId->currentRow() ];
  } else {
    return 0;
  }
}

Resource *SelectDialog::getResource( QList<Resource *> list, QWidget *parent )
{
  if ( list.count() == 0 ) {
    KMessageBox::error( parent, i18n( "There is no resource available." ) );
    return 0;
  }

  if ( list.count() == 1 ) {
    return list.first();
  }

  // the following lines will return a writeable resource if only _one_
  // writeable resource exists
  Resource *found = 0;

  for ( int i=0; i< list.size(); ++i ) {
    if ( !list.at( i )->readOnly() ) {
      if ( found ) {
        found = 0;
        break;
      }
    } else {
      found = list.at( i );
    }
  }

  if ( found ) {
    return found;
  }

  SelectDialog dlg( list, parent );
  if ( dlg.exec() == KDialog::Accepted ) {
    return dlg.resource();
  } else {
    return 0;
  }
}
