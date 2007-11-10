/*
  This file is part of the kcal library.

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

#include "resourcelocaldirconfig.h"
#include "resourcelocaldir.h"
#include "resourcelocaldir_p.h"

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QGridLayout>

#include <typeinfo>

#include "resourcelocaldirconfig.moc"

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::ResourceLocalDirConfig::Private
{
  public:
    Private()
    {}
    KUrlRequester *mURL;
};
//@endcond

ResourceLocalDirConfig::ResourceLocalDirConfig( QWidget *parent )
  : KRES::ConfigWidget( parent ), d( new KCal::ResourceLocalDirConfig::Private )
{
  resize( 245, 115 );
  QGridLayout *mainLayout = new QGridLayout( this );

  QLabel *label = new QLabel( i18n( "Location:" ), this );
  d->mURL = new KUrlRequester( this );
  d->mURL->setMode( KFile::Directory | KFile::LocalOnly );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( d->mURL, 1, 1 );
}

ResourceLocalDirConfig::~ResourceLocalDirConfig()
{
  delete d;
}

void ResourceLocalDirConfig::loadSettings( KRES::Resource *resource )
{
  ResourceLocalDir *res = static_cast<ResourceLocalDir*>( resource );
  if ( res ) {
    d->mURL->setUrl( res->d->mURL.prettyUrl() );
  } else {
    kDebug(5800) << "ERROR: ResourceLocalDirConfig::loadSettings(): "
                 << "no ResourceLocalDir, cast failed";
  }
}

void ResourceLocalDirConfig::saveSettings( KRES::Resource *resource )
{
  ResourceLocalDir *res = static_cast<ResourceLocalDir*>( resource );
  if (res) {
    res->d->mURL = d->mURL->url();
  } else {
    kDebug(5800) << "ERROR: ResourceLocalDirConfig::saveSettings(): "
                 << "no ResourceLocalDir, cast failed";
  }
}
