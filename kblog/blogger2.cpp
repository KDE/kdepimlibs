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

#include <blogger2.h>

#include <kdebug.h>
#include <klocale.h>
#include <blogger2_p.h>
#include <syndication/loader.h>

#include <QtCore/QList>

using namespace KBlog;


APIBlogger2::APIBlogger2( const KUrl &server, QObject *parent ) : 
                      APIBlog( server, parent ), d( new APIBlogger2Private )
{
  d->parent = this;
  setUrl( server );
}

APIBlogger2::~APIBlogger2()
{
  delete d;
}

QString APIBlogger2::interfaceName() const
{
  return QLatin1String( "Blogger API 2.0" );
}

void APIBlogger2::setUrl( const KUrl &server )
{
  APIBlog::setUrl( server );
  d->getIntrospection();
}

void APIBlogger2::userInfo()
{
  kDebug() << "Fetching user information is not available in Blogger2 API." << endl;
  emit error( NotSupported, i18n( "Fetching user information is not available in Blogger2 API." ) );
}

void APIBlogger2::listBlogs()
{
  kDebug() << "Fetching user's blogs is not available in Blogger2 API." << endl;
  emit error( NotSupported, i18n( "Fetching user's blogs is not available in Blogger2 API." ) );
}

void APIBlogger2::listPostings()
{
  kDebug() << "listPostings()" << endl;
  Syndication::Loader *loader = Syndication::Loader::create();
  connect(loader, SIGNAL(loadingComplete(Loader*, FeedPtr, ErrorCode)),
          d, SLOT(slotLoadingPostingsComplete(Loader*, FeedPtr, ErrorCode)));
  loader->loadFrom( d->getFetchPostingsPath() );
}

void APIBlogger2::listCategories(){
  kDebug() << "Fetching categories is not available in Blogger2 API." << endl;
  emit error( NotSupported, i18n( "Fetching categories is not available in Blogger2 API." ) );
}

void APIBlogger2::fetchPosting( const QString &postingId )
{

}

void APIBlogger2::modifyPosting( KBlog::BlogPosting* posting )
{
  kDebug() << "Modifying postings is not available in Blogger2 API." << endl;
  emit error( NotSupported, i18n( "Modifying postings is not available in Blogger2 API." ) );
}

void APIBlogger2::createPosting( KBlog::BlogPosting* posting )
{

}

void APIBlogger2::createMedia( KBlog::BlogMedia* media ){
  kDebug() << "Creating media is not available in Blogger2 API." << endl;
  emit error( NotSupported, i18n( "Creating media is not available in Blogger2 API." ) );
}

void APIBlogger2::removePosting( const QString &postingId )
{
  kDebug() << "Removing postings is not available in Blogger2 API." << endl;
  emit error( NotSupported, i18n( "Removing postings is not available in Blogger2 API." ) );
}

#include "blogger2.moc"
