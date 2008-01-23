/*
  This file is part of the kcal library.

  Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include "resourcelocalconfig.h"
#include "resourcelocal.h"
#include "resourcelocal_p.h"
#include "vcalformat.h"
#include "icalformat.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>

#include <typeinfo>

#include "resourcelocalconfig.moc"

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::ResourceLocalConfig::Private
{
  public:
    Private()
    {}
    KUrlRequester *mURL;
    QGroupBox *mFormatGroup;
    QRadioButton *mIcalButton;
    QRadioButton *mVcalButton;
};
//@endcond

ResourceLocalConfig::ResourceLocalConfig( QWidget *parent )
  : KRES::ConfigWidget( parent ), d( new KCal::ResourceLocalConfig::Private )
{
  resize( 245, 115 );
  QGridLayout *mainLayout = new QGridLayout( this );

  QLabel *label = new QLabel( i18n( "Location:" ), this );
  d->mURL = new KUrlRequester( this );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( d->mURL, 1, 1 );

  d->mFormatGroup = new QGroupBox( i18n( "Calendar Format" ), this );

  d->mIcalButton = new QRadioButton( i18n( "iCalendar" ), d->mFormatGroup );
  d->mVcalButton = new QRadioButton( i18n( "vCalendar" ), d->mFormatGroup );

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget( d->mIcalButton );
  vbox->addWidget( d->mVcalButton );
  vbox->addStretch( 1 );
  d->mFormatGroup->setLayout( vbox );

  mainLayout->addWidget( d->mFormatGroup, 2, 1 );
}

ResourceLocalConfig::~ResourceLocalConfig()
{
  delete d;
}

void ResourceLocalConfig::loadSettings( KRES::Resource *resource )
{
  ResourceLocal* res = static_cast<ResourceLocal*>( resource );
  if ( res ) {
    d->mURL->setUrl( res->d->mURL.prettyUrl() );
    kDebug() << "Format typeid().name():" << typeid( res->d->mFormat ).name();
    if ( typeid( *(res->d->mFormat) ) == typeid( ICalFormat ) ) {
      d->mIcalButton->setChecked(true);
    } else if ( typeid( *(res->d->mFormat) ) == typeid( VCalFormat ) ) {
      d->mVcalButton->setChecked(true);
    } else {
      kDebug() << "ERROR: Unknown format type";
    }
  } else {
    kDebug() << "ERROR: no ResourceLocal, cast failed";
  }
}

void ResourceLocalConfig::saveSettings( KRES::Resource *resource )
{
  KUrl url = d->mURL->url();

  if( url.isEmpty() ) {
    KStandardDirs dirs;
    QString saveFolder = dirs.saveLocation( "data", "korganizer" );
    QFile file( saveFolder + "/std.ics" );

    // find a non-existent name
    for ( int i = 0; file.exists(); ++i ) {
      file.setFileName( saveFolder + "/std" + QString::number(i) + ".ics" );
    }

    KMessageBox::information(
      this,
      i18n( "You did not specify a URL for this resource. "
            "Therefore, the resource will be saved in %1. "
            "It is still possible to change this location "
            "by editing the resource properties.", file.fileName() ) );

    url = KUrl::fromPath( file.fileName() );
  }

  ResourceLocal* res = static_cast<ResourceLocal*>( resource );
  if ( res ) {
    res->d->mURL = url;

    delete res->d->mFormat;
    if ( d->mIcalButton->isDown() ) {
      res->d->mFormat = new ICalFormat();
    } else {
      res->d->mFormat = new VCalFormat();
    }
  } else {
    kDebug() << "ERROR: no ResourceLocal, cast failed";
  }
}
