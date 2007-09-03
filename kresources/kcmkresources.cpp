/*
    This file is part of libkresources.

    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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
#include "kcmkresources.h"

#include <QtGui/QLayout>

#include <kaboutdata.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <klocale.h>

#include "configpage.h"

K_PLUGIN_FACTORY( ResourcesFactory, registerPlugin<KCMKResources>(); )
K_EXPORT_PLUGIN( ResourcesFactory( "kcmkresources" ) )

KCMKResources::KCMKResources( QWidget *parent, const QVariantList &l )
  : KCModule( ResourcesFactory::componentData(), parent, QVariantList() )
{
  Q_UNUSED( l );

  QVBoxLayout *layout = new QVBoxLayout( this );
  mConfigPage = new KRES::ConfigPage( this );
  layout->addWidget( mConfigPage );
  connect( mConfigPage, SIGNAL( changed( bool ) ), SIGNAL( changed( bool ) ) );
  setButtons( Help | Apply );
  KAboutData *about =
   new KAboutData( I18N_NOOP( "kcmkresources" ), 0,
                   ki18n( "KDE Resources configuration module" ),
                   0, KLocalizedString(), KAboutData::License_GPL,
                   ki18n( "(c) 2003 Tobias Koenig" ) );

  about->addAuthor( ki18n( "Tobias Koenig" ), KLocalizedString(), "tokoe@kde.org" );
  setAboutData( about );
}

void KCMKResources::load()
{
  mConfigPage->load();
}

void KCMKResources::save()
{
  mConfigPage->save();
}

void KCMKResources::defaults()
{
  mConfigPage->defaults();
}

#include "kcmkresources.moc"
