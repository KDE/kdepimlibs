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

#include "metaweblog.h"
#include "metaweblog_p.h"
#include "blogposting.h"
#include "blogmedia.h"

#include <KDebug>
#include <KLocale>
#include <KDateTime>

using namespace KBlog;

MetaWeblog::MetaWeblog( const KUrl &server, QObject *parent )
  : Blogger1( server, *new MetaWeblogPrivate, parent )
{
  Q_D(MetaWeblog);
  setUrl( server );
}

MetaWeblog::MetaWeblog( const KUrl &server, MetaWeblogPrivate &dd,
                        QObject *parent )
  : Blogger1( server, dd, parent )
{
  Q_D(MetaWeblog);
  setUrl( server );
}

MetaWeblog::~MetaWeblog()
{
}

QString MetaWeblog::interfaceName() const
{
  return QLatin1String( "MetaWeblog " );
}

void MetaWeblog::setUrl( const KUrl &server )
{
  Q_D(MetaWeblog);
  Blogger1::setUrl( server );
  delete d->mXmlRpcClient;
  d->mXmlRpcClient = new KXmlRpc::Client( server );
  d->mXmlRpcClient->setUserAgent( userAgent() );
}

void MetaWeblog::listRecentPostings( int number )
{
    Q_D(MetaWeblog);
    kDebug(5323) << "Fetching List of Posts..." << endl;
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( number );
    d->mXmlRpcClient->call(
      "metaWeblog.getRecentPosts", args,
      this, SLOT( slotListRecentPostings( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT( slotError( int, const QString&, const QVariant& ) ) );
}

void MetaWeblog::listCategories()
{
    Q_D(MetaWeblog);
    kDebug(5323) << "Fetching List of Categories..." << endl;
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    d->mXmlRpcClient->call(
      "metaWeblog.getCategories", args,
      this, SLOT( slotListCategories( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT ( slotError( int, const QString&, const QVariant& ) ) );
}

void MetaWeblog::fetchPosting( KBlog::BlogPosting *posting )
{
//   Q_D(MetaWeblog);
//   if ( d->mLock.tryLock() ) {
//     kDebug(5323) << "Fetching Posting with url " << postingId << endl;
//     QList<QVariant> args( d->defaultArgs( postingId ) );
//     d->mXmlRpcClient->call(
//       "metaWeblog.getPost", args,
//       d, SLOT( slotFetchPosting( const QList<QVariant>&, const QVariant& ) ),
//       d, SLOT( slotError( int, const QString&, const QVariant& ) ) );
//     return true;
//   }
//   return false;
}

void MetaWeblog::modifyPosting( KBlog::BlogPosting *posting )
{
  Q_D(MetaWeblog);
  if ( !posting ) {
    kDebug(5323) << "MetaWeblog::modifyPosting: posting null pointer"
        << endl;
    emit error ( Other, i18n( "Posting is a null pointer." ) );
  }
    kDebug(5323) << "Uploading Posting with postId " << posting->postingId()
        << endl;

    QList<QVariant> args( d->defaultArgs( posting->postingId() ) );
    QMap<QString, QVariant> map;
    map["categories"] = posting->categories();
    map["description"] = posting->content();
    map["title"] = posting->title();
    map["lastModified"] = posting->modificationDateTime().toUtc().dateTime();
    args << map;
    args << QVariant( posting->isPublished() );
    d->mXmlRpcClient->call(
      "metaWeblog.editPost", args,
      this, SLOT( slotModifyPosting( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT ( slotError( int, const QString&, const QVariant& ) ) );
}

void MetaWeblog::createPosting( KBlog::BlogPosting *posting )
{
  Q_D(MetaWeblog);
  if ( !posting ) {
    kDebug(5323) << "MetaWeblog::createPosting: posting null pointer"
        << endl;
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
    args << QVariant( posting->isPublished() );
    d->mXmlRpcClient->call (
      "metaWeblog.newPost", args,
      this, SLOT( slotCreatePosting( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT ( slotError( int, const QString&, const QVariant& ) ) );
}

void MetaWeblog::createMedia( KBlog::BlogMedia *media )
{
    Q_D(MetaWeblog);
    kDebug(5323) << "MetaWeblog::createMedia: name="<< media->name() << endl;
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    QMap<QString, QVariant> map;
    QList<QVariant> list;
    map["name"] = media->name();
    map["type"] = media->mimetype();
    map["bits"] = media->data();
    args << map;
    d->mXmlRpcClient->call(
      "metaWeblog.newMediaObject", args,
      this, SLOT( slotCreateMedia( const QList<QVariant>&, const QVariant& ) ),
      this, SLOT ( slotError( int, const QString&, const QVariant& ) ) );

}

#include "metaweblog.moc"
