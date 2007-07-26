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

#include "movabletype_p.h"
#include "blogposting.h"

#include <kxmlrpcclient/client.h>

using namespace KBlog;

MovableType::MovableTypePrivate::MovableTypePrivate()
{
  mXmlRpcClient = 0;
}

MovableType::MovableTypePrivate::~MovableTypePrivate()
{
  delete mXmlRpcClient;
}

QList<QVariant> MovableType::MovableTypePrivate::defaultArgs( const QString &id )
{
  QList<QVariant> args;

  if ( id.toInt() ) {
    args << QVariant( id.toInt() );
  }
  if ( !id.toInt() && !id.isNull() ){
    args << QVariant( id );
  }
  args << QVariant( parent->username() )
       << QVariant( parent->password() );
  return args;
}

bool MovableType::MovableTypePrivate::readPostingFromMap(
    BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  //TODO
  return false;
}

void MovableType::MovableTypePrivate::slotCreatePosting(
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

void MovableType::MovableTypePrivate::slotError( int number,
    const QString &errorString, const QVariant &id )
{
  //TODO
}

void MovableType::MovableTypePrivate::slotFetchPosting(
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

void MovableType::MovableTypePrivate::slotListRecentPostings(
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

void MovableType::MovableTypePrivate::slotListTrackbackPings(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO Contains:
  // String pingTitle: the title of the entry sent in the ping
  // String pingURL: the URL of the entry
  // String pingIP: the IP address of the host that sent the ping
}

void MovableType::MovableTypePrivate::slotModifyPosting(
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

#include "movabletype_p.moc"
