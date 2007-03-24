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

#include <metaweblog.h>

#include <kxmlrpcclient/client.h>
#include <kdebug.h>
#include <klocale.h>
#include <metaweblog_p.h>

#include <QtCore/QList>

using namespace KBlog;

APIMetaWeblog::APIMetaWeblog( const KUrl &server, QObject *parent,
                              const char *name )
  : APIBlog( server, parent/*, name*/ ), d( new APIMetaWeblogPrivate )
{
  d->parent = this;
  setUrl( server );
}

APIMetaWeblog::~APIMetaWeblog()
{
  delete d;
}

void APIMetaWeblog::setUrl( const KUrl &server )
{
  APIBlog::setUrl( server );
  delete d->mXmlRpcClient;
  d->mXmlRpcClient = new KXmlRpc::Client( server );
  d->mXmlRpcClient->setUserAgent( "KDE-KBlog" );
}

void APIMetaWeblog::userInfo()
{
  kDebug() << "Fetching user information is not available in MetaWeblog API."
           << endl;
  emit error( NotSupported,
              i18n("Fetching user information is not available in "
                   "MetaWeblog API.") );
}

void APIMetaWeblog::listBlogs()
{
  kDebug() << "Fetching user's blogs is not available in MetaWeblog API."
           << endl;
  emit error( NotSupported,
              i18n("Fetching user's blogs is not available in "
                   "MetaWeblog API.") );
}

void APIMetaWeblog::listPostings()
{
  kDebug() << "Fetching List of Posts..." << endl;
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  args << QVariant( downloadCount() );
  d->mXmlRpcClient->call(
    "metaWeblog.getRecentPosts", args,
    d, SLOT( slotListPostings( const QList<QVariant>&, const QVariant& ) ),
    d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::listCategories()
{
  kDebug() << "Fetching List of Categories..." << endl;
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  d->mXmlRpcClient->call(
    "metaWeblog.getCategories", args,
    d, SLOT( slotListCategories( const QList<QVariant>&, const QVariant& ) ),
    d, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::fetchPosting( const QString &postingId )
{
  kDebug() << "Fetching Posting with url " << postingId << endl;
  QList<QVariant> args( d->defaultArgs( postingId ) );
  d->mXmlRpcClient->call(
    "metaWeblog.getPost", args,
    d, SLOT( slotFetchPosting( const QList<QVariant>&, const QVariant& ) ),
    d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::modifyPosting( KBlog::BlogPosting *posting )
{
  if ( !posting ) {
    kDebug() << "APIMetaWeblog::modifyPosting: posting null pointer" << endl;
    emit error ( Other, i18n("Posting is a null pointer.") );
    return;
  }
  kDebug() << "Uploading Posting with postId " << posting->postingId() << endl;

  QList<QVariant> args( d->defaultArgs( posting->postingId() ) );
  QMap<QString, QVariant> map;
  QList<QVariant> list;
  list.append( QString( posting->category() ) );
  map["categories"] = list;
  map["description"] = posting->content();
  map["title"] = posting->title();
  map["lastModified"] = posting->modificationDateTime().toUtc().dateTime();
  args << map;
  args << QVariant( posting->publish() );
  d->mXmlRpcClient->call(
    "metaWeblog.editPost", args,
    d, SLOT( slotModifyPosting( const QList<QVariant>&, const QVariant& ) ),
    d, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::createPosting( KBlog::BlogPosting *posting )
{
  if ( !posting ) {
    kDebug() << "APIMetaWeblog::createPosting: posting null pointer" << endl;
    emit error ( Other, i18n("Posting is a null pointer.") );
    return;
  }
  kDebug() << "Creating new Posting with blogId " << blogId() << endl;
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  QMap<QString, QVariant> map;
  QList<QVariant> list;
  list.append( QString( posting->category() ) );
  map["categories"] = list;
  map["description"] = posting->content();
  map["title"] = posting->title();
  map["dateCreated"] = posting->creationDateTime().toUtc().dateTime();
  args << map;
  args << QVariant( posting->publish() );
  d->mXmlRpcClient->call (
    "metaWeblog.newPost", args,
    d, SLOT( slotCreatePosting( const QList<QVariant>&, const QVariant& ) ),
    d, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::createMedia( KBlog::BlogMedia *media )
{
  kDebug() << "APIMetaWeblog::createMedia: name="<< media->name() << endl;
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  QMap<QString, QVariant> map;
  QList<QVariant> list;
  map["name"] = media->name();
  map["type"] = media->mimetype();
  map["bits"] = media->data();
  args << map;
  d->mXmlRpcClient->call(
    "metaWeblog.newMediaObject", args,
    d, SLOT( slotCreateMedia( const QList<QVariant>&, const QVariant& ) ),
    d, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::removePosting( const QString &postingId )
{
  kDebug() << "APIMetaWeblog::removePosting: postingId=" << postingId << endl;
  QList<QVariant> args( d->defaultArgs( postingId ) );
  args << QVariant( /*publish=*/true );
  d->mXmlRpcClient->call(
    "metaWeblog.deletePost", args,
    d, SLOT( slotCreateMedia( const QList<QVariant>&, const QVariant& ) ),
    d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

#include "metaweblog.moc"
