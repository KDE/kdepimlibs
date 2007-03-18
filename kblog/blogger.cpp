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

#include <blogger.h>

#include <kxmlrpcclient/client.h>
#include <kdebug.h>
#include <klocale.h>
#include <blogger_p.h>

#include <QtCore/QList>

using namespace KBlog;


APIBlogger::APIBlogger( const KUrl &server, QObject *parent, const char *name ) : APIBlog( server, parent, name ), 
d( new APIBloggerPrivate )
{
  d->parent = this;
  setUrl( server );
}

APIBlogger::~APIBlogger()
{
  delete d;
}

void APIBlogger::setUrl( const KUrl &server )
{
  APIBlog::setUrl( server );
  delete d->mXmlRpcClient;
  d->mXmlRpcClient = new KXmlRpc::Client( server );
  d->mXmlRpcClient->setUserAgent( "KDE-KBlog" );
}

void APIBlogger::userInfo()
{
  kDebug() << "read user info..." << endl;
  QList<QVariant> args( d->defaultArgs() );
  d->mXmlRpcClient->call( "blogger.getUserInfo", args, 
                          d, SLOT( slotUserInfo( const QList<QVariant>&, const QVariant& ) ), 
			  d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::listBlogs()
{
  kDebug() << "Fetch List of Blogs..." << endl;
  QList<QVariant> args( d->defaultArgs() );
  d->mXmlRpcClient->call( "blogger.getUsersBlogs", args, 
                          d, SLOT( slotListBlogs( const QList<QVariant>&, const QVariant& ) ), 
			  d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::listPostings()
{
  kDebug() << "Fetching List of Posts..." << endl;
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  args << QVariant( downloadCount() );
  d->mXmlRpcClient->call( "blogger.getRecentPosts", args, 
                          d, SLOT( slotListPostings( const QList<QVariant>&, const QVariant& ) ), 
			  d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::listCategories(){
  emit error( NotSupported, i18n( "Categories are not supported in Blogger API 1.0." ) );
  kDebug() << "Categories are not supported in Blogger API 1.0." << endl;
}

void APIBlogger::fetchPosting( const QString &postingId )
{
  kDebug() << "Fetching Posting with url " << postingId << endl;
  QList<QVariant> args( d->defaultArgs( postingId ) );
  d->mXmlRpcClient->call( "blogger.getPost", args, 
                    d, SLOT( slotFetchPosting( const QList<QVariant>&, const QVariant& ) ), 
		    d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::modifyPosting( KBlog::BlogPosting* posting )
{
  if ( !posting ) {
    kDebug() << "APIBlogger::modifyPosting: posting is null pointer" << endl;
  }
    kDebug() << "Uploading Posting with postingId " << posting->postingId() << endl;
    QList<QVariant> args( d->defaultArgs( posting->postingId() ) );
    args << QVariant( posting->content() );
    args << QVariant( posting->publish() );
    d->mXmlRpcClient->call( "blogger.editPost", args, 
                            d, SLOT( slotModifyPosting( const QList<QVariant>&, const QVariant& ) ), 
			    d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::createPosting( KBlog::BlogPosting* posting )
{
  if ( !posting ) {
    kDebug() << "APIBlogger::createPosting: posting is null pointer" << endl;
  }
    kDebug() << "Creating new Posting with blogid " << blogId() << endl;
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( posting->content() );
    args << QVariant( posting->publish() );
    d->mXmlRpcClient->call( "blogger.newPost", args, 
                            d, SLOT( slotCreatePosting( const QList<QVariant>&, const QVariant& ) ), 
			    d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::createMedia( KBlog::BlogMedia* media ){
  emit error( NotSupported, i18n( "Media upload not available in Blogger API 1.0." ) );
  kDebug() << "Media upload not available in Blogger API 1.0." << endl;
}

void APIBlogger::removePosting( const QString &postingId )
{
  kDebug() << "APIBlogger::removePosting: postingId=" << postingId << endl;
  QList<QVariant> args( d->defaultArgs( postingId ) );
  args << QVariant( /*publish=*/true );
  d->mXmlRpcClient->call( "blogger.deletePost", args, 
                          d, SLOT( slotCreateMedia( QList<QVariant> &result, QVariant &id ) ), 
			  d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

#include "blogger.moc"
