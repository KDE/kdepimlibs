/*
  This file is part of the kblog library.

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

#include "movabletype.h"
#include "movabletype_p.h"
#include "blogposting.h"

#include <kxmlrpcclient/client.h>

using namespace KBlog;

MovableType::MovableType( const KUrl &server, QObject *parent )
  : MetaWeblog( server, *new MovableTypePrivate, parent )
{
  setUrl( server );
}

MovableType::~MovableType()
{
}

void MovableType::createPosting( KBlog::BlogPosting *posting )
{
  //TODO
}

void MovableType::fetchPosting( KBlog::BlogPosting *posting )
{
  //TODO
}

QString MovableType::interfaceName() const
{
  return QLatin1String( "Movable Type " );
}

void MovableType::setUrl( const KUrl &server )
{
  Q_D(MovableType);
  MetaWeblog::setUrl( server );
  delete d->mXmlRpcClient;
  d->mXmlRpcClient = new KXmlRpc::Client( server );
  d->mXmlRpcClient->setUserAgent( userAgent() );
}

void MovableType::listRecentPostings( int number )
{
  //TODO
}

void MovableType::listTrackbackPings( KBlog::BlogPosting *posting ) {
  //TODO
  /*
  Q_D(MovableType);
  d->mXmlRpcClient->call( "mt.getTracebackPings", args,
    d, SLOT( slotListTrackbackPings(
              const QList<QVariant>&, const QVariant& ) ),
    d, SLOT( slotError( int, const QString&, const QVariant& ) ) );
  */
}

void MovableType::modifyPosting( KBlog::BlogPosting *posting )
{
  //TODO
}

MovableTypePrivate::MovableTypePrivate()
{
  mXmlRpcClient = 0;
}

MovableTypePrivate::~MovableTypePrivate()
{
  delete mXmlRpcClient;
}

QList<QVariant> MovableTypePrivate::defaultArgs( const QString &id )
{
  Q_Q(MovableType);
  QList<QVariant> args;

  if ( id.toInt() ) {
    args << QVariant( id.toInt() );
  }
  if ( !id.toInt() && !id.isNull() ){
    args << QVariant( id );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

bool MovableTypePrivate::readPostingFromMap(
    BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  //TODO
  return false;
}

void MovableTypePrivate::slotCreatePosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO 7 new keys are:
  // int mt_allow_comments: the value for the allow_comments field
  // int mt_allow_pings, the value for the allow_pings field
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // String mt_excerpt, the value for the excerpt field
  // String mt_keywords, the value for the keywords field
  // array mt_tb_ping_urls, the list of TrackBack ping URLs for this entry
}

void MovableTypePrivate::slotError( int number,
    const QString &errorString, const QVariant &id )
{
  //TODO
}

void MovableTypePrivate::slotFetchPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO 6 new keys are:
  // int mt_allow_comments: the value for the allow_comments field
  // int mt_allow_pings, the value for the allow_pings field
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // String mt_excerpt, the value for the excerpt field
  // String mt_keywords, the value for the keywords field
}

void MovableTypePrivate::slotListRecentPostings(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO 6 new keys are:
  // int mt_allow_comments: the value for the allow_comments field
  // int mt_allow_pings, the value for the allow_pings field
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // String mt_excerpt, the value for the excerpt field
  // String mt_keywords, the value for the keywords field
}

void MovableTypePrivate::slotListTrackbackPings(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO Contains:
  // String pingTitle: the title of the entry sent in the ping
  // String pingURL: the URL of the entry
  // String pingIP: the IP address of the host that sent the ping
}

void MovableTypePrivate::slotModifyPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO 5 new keys are:
  // int mt_allow_comments: the value for the allow_comments field
  // int mt_allow_pings, the value for the allow_pings field
  // String mt_convert_breaks, the value for the convert_breaks field
  // String mt_text_more, the value for the additional entry text
  // String mt_excerpt, the value for the excerpt field
  // String mt_keywords, the value for the keywords field
  // array mt_tb_ping_urls, the list of TrackBack ping URLs for this entry
}

#include "movabletype.moc"
