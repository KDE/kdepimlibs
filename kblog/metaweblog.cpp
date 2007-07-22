/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Christian Weilbach <christian@whiletaker.homeip.net>
  Copyright (c) 2007 Mike Arthur <mike@mikearthur.co.uk>

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
#include <metaweblog_p.h>

#include <kdebug.h>
#include <klocale.h>

using namespace KBlog;

APIMetaWeblog::APIMetaWeblog( const KUrl &server, QObject *parent )
  : APIBlogger( server, parent ), d( new APIMetaWeblogPrivate )
{
  d->parent = this;
  setUrl( server );
}

APIMetaWeblog::~APIMetaWeblog()
{
  delete d;
}

QString APIMetaWeblog::interfaceName() const
{
  return QLatin1String( "MetaWeblog API" );
}

void APIMetaWeblog::setUrl( const KUrl &server )
{
  APIBlogger::setUrl( server );
  delete d->mXmlRpcClient;
  d->mXmlRpcClient = new KXmlRpc::Client( server );
  d->mXmlRpcClient->setUserAgent( "KDE-KBlog" );
}

void APIMetaWeblog::listPostings()
{
    kDebug(5323) << "Fetching List of Posts..." << endl;
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( downloadCount() );
    d->mXmlRpcClient->call(
      "metaWeblog.getRecentPosts", args,
      d, SLOT( slotListPostings( const QList<QVariant>&, const QVariant& ) ),
      d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::listCategories()
{
    kDebug(5323) << "Fetching List of Categories..." << endl;
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    d->mXmlRpcClient->call(
      "metaWeblog.getCategories", args,
      d, SLOT( slotListCategories( const QList<QVariant>&, const QVariant& ) ),
      d, SLOT ( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIMetaWeblog::fetchPosting( KBlog::BlogPosting *posting )
{
//   if ( d->mLock.tryLock() ) {
//     kDebug(5323) << "Fetching Posting with url " << postingId << endl;
//     QList<QVariant> args( d->defaultArgs( postingId ) );
//     d->mXmlRpcClient->call(
//       "metaWeblog.getPost", args,
//       d, SLOT( slotFetchPosting( const QList<QVariant>&, const QVariant& ) ),
//       d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
//     return true;
//   }
//   return false;
}

void APIMetaWeblog::modifyPosting( KBlog::BlogPosting *posting )
{
  if ( !posting ) {
    kDebug(5323) << "APIMetaWeblog::modifyPosting: posting null pointer" << endl;
    emit error ( Other, i18n( "Posting is a null pointer." ) );
  }
    kDebug(5323) << "Uploading Posting with postId " << posting->postingId() << endl;

    QList<QVariant> args( d->defaultArgs( posting->postingId() ) );
    QMap<QString, QVariant> map;
    map["categories"] = posting->categories();
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
    kDebug(5323) << "APIMetaWeblog::createPosting: posting null pointer" << endl;
    emit error ( Other, i18n( "Posting is a null pointer." ) );
  }
    kDebug(5323) << "Creating new Posting with blogId " << blogId() << endl;
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    QMap<QString, QVariant> map;
    map["categories"] = posting->categories();
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
    kDebug(5323) << "APIMetaWeblog::createMedia: name="<< media->name() << endl;
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

#include "metaweblog.moc"
