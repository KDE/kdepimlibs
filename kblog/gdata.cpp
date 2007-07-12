/*
    This file is part of the kblog library.

    Copyright (c) 2007 Christian Weilbach <christian@whiletaker.homeip.net>

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
#include <gdata_p.h>

#include <syndication/loader.h>

#include <kdebug.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kio/http.h>

#include <QtCore/QList>

using namespace KBlog;

APIGData::APIGData( const KUrl &server, QObject *parent )
  : APIBlog( server, parent ), d( new APIGDataPrivate )
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

void APIGData::userInfo()
{
  kDebug() << "Fetching user information is not available in GData API." << endl;
  emit error( NotSupported, i18n( "Fetching user information is not available in GData API." ) );
}

void APIGData::getIntrospection(){
  // fetch the introspection file synchronously and parse it
  QByteArray data;
  KIO::Job *job = KIO::get( url(), false, false ); 
  if ( KIO::NetAccess::synchronousRun( job, (QWidget*)0, &data, &url() ) ) {
    kDebug() << "Fetched Homepage data." << endl;
//     QRegExp pp( "<link.+rel=\"service.post\".+href=\"(.+)\".*/>" );
//     if( pp.indexIn( data )!=-1 )
//        mCreatePostingsPath = pp.cap(1);
//     else
//        emit parent->error( Other, i18n( "Could not regexp the service.post path." ) );
//     kDebug(5323)<<"QRegExp pp( \"<link.+rel=\"service.post\".+href=\"(.+)\".*/>\" ) matches "<< pp.cap(1) << endl;

    QRegExp bid( "<link.+blogID=(\\d+).*/>" );
    if( bid.indexIn( data )!=-1 )
       setBlogId( bid.cap(1) );
    else
      emit error( Other, i18n( "Could not regexp the blogID path." ) );
    kDebug(5323)<<"QRegExp bid( '<link.+blogID=(\\d+).*/>' matches "<< bid.cap(1) << endl;
  }
  else {
    emit error( Other, i18n( "Could not fetch the homepage data." ) );
    kDebug(5323)<< "Could not fetch the homepage data." << endl;
  }
}

void APIGData::listBlogs()
{
  kDebug() << "listBlogs()" << endl;
  Syndication::Loader *loader = Syndication::Loader::create();
  connect( loader, SIGNAL(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)),
           d, SLOT(slotLoadingBlogsComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)) );
  loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + username() + QString( "/blogs" ) );
}

void APIGData::listPostings()
{
  kDebug() << "listPostings()" << endl;
  Syndication::Loader *loader = Syndication::Loader::create();
  connect( loader, SIGNAL(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)),
           d, SLOT(slotLoadingPostingsComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)) );
  loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + blogId() + QString( "/posts/default" ) );
}

void APIGData::listCategories()
{
  kDebug() << "Fetching categories is not available in GData API." << endl;
  emit error( NotSupported, i18n( "Fetching categories is not available in GData API." ) );
}

void APIGData::fetchPosting( const QString &postingId )
{
  kDebug() << "fetchPosting()" << endl;
  Syndication::Loader *loader = Syndication::Loader::create();
  d->setFetchPostingId( postingId );
  connect( loader, SIGNAL(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)),
           d, SLOT(slotFetchingPostingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));
  loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + blogId() + QString( "/posts/default" ) );
}

void APIGData::modifyPosting( KBlog::BlogPosting* posting )
{
//FIXME
//   kDebug() << "Modifying postings is not available in GData API." << endl;
//   emit error( NotSupported, i18n( "Modifying postings is not available in GData API." ) );
}

void APIGData::createPosting( KBlog::BlogPosting* posting )
{
  kDebug() << "createPosting()" << endl;
  d->authenticate();
}

void APIGData::createMedia( KBlog::BlogMedia* media )
{
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
