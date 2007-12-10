/*
  This file is part of the kcal library.

  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#include "resourcecached.h"

#include <khbox.h>
#include <klocale.h>
#include <kdebug.h>

#include <QtGui/QLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>

#include "resourcecachedconfig.moc"

using namespace KCal;

//@cond PRIVATE
class ResourceCachedConfigPrivate
{
  public:
    ResourceCachedConfigPrivate()
      : mGroup( 0 ),
        mIntervalSpin( 0 ) {}

    QButtonGroup *mGroup;
    QSpinBox *mIntervalSpin;
};

class KCal::ResourceCachedReloadConfig::Private
  : public ResourceCachedConfigPrivate
{
};

class KCal::ResourceCachedSaveConfig::Private
  : public ResourceCachedConfigPrivate
{
};
//@endcond

ResourceCachedReloadConfig::ResourceCachedReloadConfig( QWidget *parent )
  : QWidget( parent ), d( new KCal::ResourceCachedReloadConfig::Private() )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  QGroupBox *groupBox = new QGroupBox( i18nc( "@title:group", "Automatic Reload" ), this );
  topLayout->addWidget( groupBox );
  QRadioButton *noAutomaticReload =
    new QRadioButton(
      i18nc( "@option:radio never reload the cache", "Never" ), groupBox );
  QRadioButton *automaticReloadOnStartup =
    new QRadioButton(
      i18nc( "@option:radio reload the cache on startup", "On startup" ), groupBox );
  QRadioButton *intervalRadio =
    new QRadioButton(
      i18nc( "@option:radio reload the cache at regular intervals",
             "Regular interval" ), groupBox );
  d->mGroup = new QButtonGroup( this );
  d->mGroup->addButton( noAutomaticReload, 0 );
  d->mGroup->addButton( automaticReloadOnStartup, 1 );
  d->mGroup->addButton( intervalRadio, 2 );

  connect( intervalRadio, SIGNAL( toggled( bool ) ),
           SLOT( slotIntervalToggled( bool ) ) );

  KHBox *intervalBox = new KHBox;
  new QLabel( i18nc( "@label:spinbox", "Interval in minutes" ), intervalBox );
  d->mIntervalSpin = new QSpinBox( intervalBox );
  d->mIntervalSpin->setRange( 1, 900 );
  d->mIntervalSpin->setEnabled( false );

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(noAutomaticReload);
  vbox->addWidget(automaticReloadOnStartup);
  vbox->addWidget(intervalRadio);
  vbox->addWidget(intervalBox);
  vbox->addStretch(1);
  groupBox->setLayout(vbox);
}

ResourceCachedReloadConfig::~ResourceCachedReloadConfig()
{
   delete d;
}

void ResourceCachedReloadConfig::loadSettings( ResourceCached *resource )
{
  d->mGroup->button( resource->reloadPolicy() )->setChecked( true );
  d->mIntervalSpin->setValue( resource->reloadInterval() );
}

void ResourceCachedReloadConfig::saveSettings( ResourceCached *resource )
{
  resource->setReloadPolicy( d->mGroup->checkedId() );
  resource->setReloadInterval( d->mIntervalSpin->value() );
}

void ResourceCachedReloadConfig::slotIntervalToggled( bool checked )
{
  if ( checked ) {
    d->mIntervalSpin->setEnabled( true );
  } else {
    d->mIntervalSpin->setEnabled( false );
  }
}

ResourceCachedSaveConfig::ResourceCachedSaveConfig( QWidget *parent )
  : QWidget( parent ), d( new KCal::ResourceCachedSaveConfig::Private() )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  QGroupBox *groupBox = new QGroupBox( i18nc( "@title:group", "Automatic Save" ), this );
  d->mGroup = new QButtonGroup( this );
  topLayout->addWidget( groupBox );
  QRadioButton *never =
    new QRadioButton(
      i18nc( "@option:radio never save the cache automatically", "Never" ), groupBox );
  QRadioButton *onExit =
    new QRadioButton(
      i18nc( "@option:radio save the cache on exit", "On exit" ), groupBox );

  QRadioButton *intervalRadio =
    new QRadioButton(
      i18nc( "@option:radio save the cache at regular intervals", "Regular interval" ), groupBox );

  d->mGroup = new QButtonGroup( this );
  d->mGroup->addButton( never, 0 );
  d->mGroup->addButton( onExit, 1 );
  d->mGroup->addButton( intervalRadio, 2 );

  connect( intervalRadio, SIGNAL( toggled( bool ) ),
           SLOT( slotIntervalToggled( bool ) ) );

  KHBox *intervalBox = new KHBox;
  new QLabel( i18nc( "@label:spinbox", "Interval in minutes" ), intervalBox );
  d->mIntervalSpin = new QSpinBox( intervalBox );
  d->mIntervalSpin->setRange( 1, 900 );
  d->mIntervalSpin->setEnabled( false );

  QRadioButton *delay =
    new QRadioButton(
      i18nc( "@option:radio save the cache after some delay",
             "Delayed after changes" ), groupBox );
  QRadioButton *every =
    new QRadioButton(
      i18nc( "@option:radio save the cache after every modification",
             "On every change" ), groupBox );
  d->mGroup->addButton( delay, 3 );
  d->mGroup->addButton( every, 4 );

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(never);
  vbox->addWidget(onExit);
  vbox->addWidget(intervalRadio);
  vbox->addWidget(intervalBox);
  vbox->addWidget(delay);
  vbox->addWidget(every);
  vbox->addStretch(1);
  groupBox->setLayout(vbox);

}

ResourceCachedSaveConfig::~ResourceCachedSaveConfig()
{
  delete d;
}

void ResourceCachedSaveConfig::loadSettings( ResourceCached *resource )
{
  d->mGroup->button( resource->savePolicy() )->setChecked( true );
  d->mIntervalSpin->setValue( resource->saveInterval() );
}

void ResourceCachedSaveConfig::saveSettings( ResourceCached *resource )
{
  resource->setSavePolicy( d->mGroup->checkedId() );
  resource->setSaveInterval( d->mIntervalSpin->value() );
}

void ResourceCachedSaveConfig::slotIntervalToggled( bool checked )
{
  if ( checked ) {
    d->mIntervalSpin->setEnabled( true );
  } else {
    d->mIntervalSpin->setEnabled( false );
  }
}
