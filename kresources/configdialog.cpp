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
/**
  @file
  This file is part of the KDE resource framework and defines the
  ConfigDialog class.

  @brief
  Provides a resource configuration dialog.

  @author Tobias Koenig
  @author Jan-Pascal van Best
*/

#include "configdialog.h"
#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QCheckBox>

#include "factory.h"

using namespace KRES;

class ConfigDialog::Private
{
  public:
    ConfigWidget *mConfigWidget;
    Resource *mResource;
    KLineEdit *mName;
    QCheckBox *mReadOnly;
};

ConfigDialog::ConfigDialog( QWidget *parent, const QString &resourceFamily,
                             Resource *resource )
  : KDialog( parent ), d( new Private )
{
  setModal( true );
  setCaption( i18nc( "@title:window", "Resource Configuration" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( false );

  d->mResource = resource;
  Factory *factory = Factory::self( resourceFamily );

  QFrame *main = new QFrame( this );
  setMainWidget( main );

  QVBoxLayout *mainLayout = new QVBoxLayout( main );
  mainLayout->setSpacing( spacingHint() );
  mainLayout->setMargin( 0 );

  QGroupBox *generalGroupBox = new QGroupBox( main );
  QGridLayout *gbLayout = new QGridLayout;
  gbLayout->setSpacing( spacingHint() );
  generalGroupBox->setLayout( gbLayout );

  generalGroupBox->setTitle( i18nc( "@title:group", "General Settings" ) );

  gbLayout->addWidget( new QLabel( i18nc( "@label resource name", "Name:" ),
                                   generalGroupBox ), 0, 0 );

  d->mName = new KLineEdit();
  gbLayout->addWidget( d->mName, 0, 1 );

  d->mReadOnly =
    new QCheckBox( i18nc( "@option:check if resource is read-only", "Read-only" ),
                   generalGroupBox );
  gbLayout->addWidget( d->mReadOnly, 1, 0, 1, 2 );

  d->mName->setText( d->mResource->resourceName() );
  d->mReadOnly->setChecked( d->mResource->readOnly() );

  mainLayout->addWidget( generalGroupBox );

  QGroupBox *resourceGroupBox = new QGroupBox( main );
  QGridLayout *resourceLayout = new QGridLayout;
  resourceLayout->setSpacing( spacingHint() );
  resourceLayout->setMargin( marginHint() );
  resourceGroupBox->setLayout( resourceLayout );

  resourceGroupBox->setTitle( i18nc( "@title:group", "%1 Resource Settings",
                                    factory->typeName( resource->type() ) ) );
  mainLayout->addWidget( resourceGroupBox );

  mainLayout->addStretch();

  d->mConfigWidget = factory->configWidget( resource->type(), resourceGroupBox );
  if ( d->mConfigWidget ) {
    resourceLayout->addWidget( d->mConfigWidget );
    d->mConfigWidget->setInEditMode( false );
    d->mConfigWidget->loadSettings( d->mResource );
    d->mConfigWidget->show();
    connect( d->mConfigWidget, SIGNAL( setReadOnly( bool ) ),
             SLOT( setReadOnly( bool ) ) );
  }

  connect( d->mName, SIGNAL( textChanged(const QString &) ),
           SLOT( slotNameChanged(const QString &) ) );

  slotNameChanged( d->mName->text() );
  setMinimumSize( sizeHint() );
}

ConfigDialog::~ConfigDialog()
{
  delete d;
}

void ConfigDialog::setInEditMode( bool value )
{
  if ( d->mConfigWidget ) {
    d->mConfigWidget->setInEditMode( value );
  }
}

void ConfigDialog::slotNameChanged( const QString &text )
{
  enableButtonOk( !text.isEmpty() );
}

void ConfigDialog::setReadOnly( bool value )
{
  d->mReadOnly->setChecked( value );
}

void ConfigDialog::accept()
{
  if ( d->mName->text().isEmpty() ) {
    KMessageBox::sorry( this, i18nc( "@info", "Please enter a resource name." ) );
    return;
  }

  d->mResource->setResourceName( d->mName->text() );
  d->mResource->setReadOnly( d->mReadOnly->isChecked() );

  if ( d->mConfigWidget ) {
    // First save generic information
    // Also save setting of specific resource type
    d->mConfigWidget->saveSettings( d->mResource );
  }

  KDialog::accept();
}

#include "configdialog.moc"
