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

APIMovableType::APIMovableTypePrivate::APIMovableTypePrivate()
{
  mXmlRpcClient = 0;
}

APIMovableType::APIMovableTypePrivate::~APIMovableTypePrivate()
{
  delete mXmlRpcClient;
}

QList<QVariant> APIMovableType::APIMovableTypePrivate::defaultArgs( const QString &id )
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

bool APIMovableType::APIMovableTypePrivate::readPostingFromMap(
    BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  //TODO
}

void APIMovableType::APIMovableTypePrivate::slotCreatePosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APIMovableType::APIMovableTypePrivate::slotFault( int number,
    const QString &errorString, const QVariant &id )
{
  //TODO
}

void APIMovableType::APIMovableTypePrivate::slotFetchPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APIMovableType::APIMovableTypePrivate::slotListRecentPostings(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APIMovableType::APIMovableTypePrivate::slotListTrackbackPings(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APIMovableType::APIMovableTypePrivate::slotModifyPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

#include "movabletype_p.moc"
