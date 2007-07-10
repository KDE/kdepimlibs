/*
    This file is part of the kblog library.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2006 Christian Weilbach <christian@whiletaker.homeip.net>

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

#include <gdata.h>

#include <kdebug.h>
#include <klocale.h>
#include <gdata_p.h>
#include <syndication/loader.h>

#include <QtCore/QList>

using namespace KBlog;


APIGData::APIGData( const KUrl &server, QObject *parent ) : 
                      APIBlog( server, parent ), d( new APIGDataPrivate )
{
  d->parent = this;
  setUrl( server );
}

APIGData::~APIGData()
{
  delete d;
}

QString APIGData::interfaceName() const
{
  return QLatin1String( "GData API" );
}

void APIGData::setUrl( const KUrl &server )
{
  APIBlog::setUrl( server );
  d->getIntrospection();
}

void APIGData::userInfo()
{
  kDebug() << "Fetching user information is not available in GData API." << endl;
  emit error( NotSupported, i18n( "Fetching user information is not available in GData API." ) );
}

void APIGData::listBlogs()
{
  kDebug() << "listBlogs()" << endl;
  Syndication::Loader *loader = Syndication::Loader::create();
  connect(loader, SIGNAL(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)),
          d, SLOT(slotLoadingBlogsComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));
  loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + username() + QString( "/blogs" ) );
}

void APIGData::listPostings()
{
  kDebug() << "listPostings()" << endl;
  Syndication::Loader *loader = Syndication::Loader::create();
  connect(loader, SIGNAL(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)),
          d, SLOT(slotLoadingPostingsComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));
  loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + blogId() + QString( "/posts/default" ) );
}

void APIGData::listCategories(){
  kDebug() << "Fetching categories is not available in GData API." << endl;
  emit error( NotSupported, i18n( "Fetching categories is not available in GData API." ) );
}

void APIGData::fetchPosting( const QString &postingId )
{
// TODO clone listPostings() code
}

void APIGData::modifyPosting( KBlog::BlogPosting* posting )
{
//FIXME
//   kDebug() << "Modifying postings is not available in GData API." << endl;
//   emit error( NotSupported, i18n( "Modifying postings is not available in GData API." ) );
}

void APIGData::createPosting( KBlog::BlogPosting* posting )
{

}

void APIGData::createMedia( KBlog::BlogMedia* media ){
  kDebug() << "Creating media is not available in GData API." << endl;
  emit error( NotSupported, i18n( "Creating media is not available in GData API." ) );
}

void APIGData::removePosting( const QString &postingId )
{
//FIXME
/*  kDebug() << "Removing postings is not available in GData API." << endl;
  emit error( NotSupported, i18n( "Removing postings is not available in GData API." ) );*/
}

#include "gdata.moc"
