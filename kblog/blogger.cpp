/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006 Christian Weilbach <christian@whiletaker.homeip.net>
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

#include <blogger.h>
#include <blogger_p.h>

#include <kdebug.h>
#include <klocale.h>

#include <QtCore/QList>

using namespace KBlog;

APIBlogger::APIBlogger( const KUrl &server, QObject *parent )
  : APIBlog( server, parent ), d( new APIBloggerPrivate )
{
  d->parent = this;
  setUrl( server );
}

APIBlogger::~APIBlogger()
{
  delete d;
}

QString APIBlogger::interfaceName() const
{
  return QLatin1String( "Blogger API 1.0" );
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
    kDebug(5323) << "read user info..." << endl;
    QList<QVariant> args( d->defaultArgs() );
    d->mXmlRpcClient->call(
      "blogger.getUserInfo", args,
      d, SLOT( slotUserInfo( const QList<QVariant>&, const QVariant& ) ),
      d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::listBlogs()
{

    kDebug(5323) << "Fetch List of Blogs..." << endl;
    QList<QVariant> args( d->defaultArgs() );
    d->mXmlRpcClient->call(
      "blogger.getUsersBlogs", args,
      d, SLOT( slotListBlogs( const QList<QVariant>&, const QVariant& ) ),
      d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::listPostings()
{

    kDebug(5323) << "Fetching List of Posts..." << endl;
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( downloadCount() );
    d->mXmlRpcClient->call(
      "blogger.getRecentPosts", args,
      d, SLOT( slotListPostings( const QList<QVariant>&, const QVariant& ) ),
      d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::listCategories()
{
  emit error( NotSupported,
              i18n( "Categories are not supported in Blogger API 1.0." ) );
  kDebug(5323) << "Categories are not supported in Blogger API 1.0." << endl;
}

void APIBlogger::fetchPosting( KBlog::BlogPosting *posting )
{
//   if ( d->mLock.tryLock() ) {
//     kDebug(5323) << "Fetching Posting with url " << postingId << endl;
//     QList<QVariant> args( d->defaultArgs( postingId ) );
//     d->mXmlRpcClient->call(
//       "blogger.getPost", args,
//       d, SLOT( slotFetchPosting( const QList<QVariant>&, const QVariant& ) ),
//       d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
//     return true;
//   }
//   return false;
}

void APIBlogger::modifyPosting( KBlog::BlogPosting *posting )
{
  if ( !posting ) {
    kDebug(5323) << "APIBlogger::modifyPosting: posting is null pointer" << endl;
  }
    kDebug(5323) << "Uploading Posting with postingId "
            << posting->postingId() << endl;

    QList<QVariant> args( d->defaultArgs( posting->postingId() ) );
    args << QVariant( posting->content() );
    args << QVariant( posting->publish() );
    d->mXmlRpcClient->call(
      "blogger.editPost", args,
      d, SLOT( slotModifyPosting( const QList<QVariant>&, const QVariant& ) ),
      d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::createPosting( KBlog::BlogPosting *posting )
{
  if ( !posting ) {
    kDebug(5323) << "APIBlogger::createPosting: posting is null pointer" << endl;
  }
    kDebug(5323) << "Creating new Posting with blogid " << blogId() << endl;
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    QStringList categories = posting->categories();
    QString content = "<title>" + posting->title() + "</title>";
    QStringList::const_iterator it;
    for ( it = categories.constBegin(); it != categories.constEnd(); ++it ) {
      content += "<category>" + *it + "</category>";
    }
    content += posting->content();
    args << QVariant( content );
    args << QVariant( posting->publish() );
    d->mXmlRpcClient->call(
      "blogger.newPost", args,
      d, SLOT( slotCreatePosting( const QList<QVariant>&, const QVariant& ) ),
      d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
}

void APIBlogger::createMedia( KBlog::BlogMedia *media )
{
  Q_UNUSED( media );
  emit error( NotSupported,
              i18n( "Media upload not available in Blogger API 1.0." ) );
  kDebug(5323) << "Media upload not available in Blogger API 1.0." << endl;
}

void APIBlogger::removePosting( KBlog::BlogPosting *posting )
{
//   if ( d->mLock.tryLock() ) {
//     kDebug(5323) << "APIBlogger::removePosting: postingId=" << postingId << endl;
//     QList<QVariant> args( d->defaultArgs( postingId ) );
//     args << QVariant( /*publish=*/true );
//     d->mXmlRpcClient->call(
//       "blogger.deletePost", args,
//       d, SLOT( slotModifyPosting( QList<QVariant> &result, QVariant &id ) ),
//       d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
//     return true;
//   }
//   return false;
}

#include "blogger.moc"
