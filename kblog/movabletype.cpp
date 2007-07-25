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

using namespace KBlog;

APIMovableType::APIMovableType( const KUrl &server, QObject *parent )
  : APIMetaWeblog( server, parent ), d( new APIMovableTypePrivate )
{
  d->parent = this;
  setUrl( server );
}

APIMovableType::~APIMovableType()
{
  delete d;
}

void APIMovableType::createPosting( KBlog::BlogPosting *posting )
{
  //TODO
}

void APIMovableType::fetchPosting( KBlog::BlogPosting *posting )
{
  //TODO
}

QString APIMovableType::interfaceName() const
{
  return QLatin1String( "Movable Type API" );
}

void APIMovableType::setUrl( const KUrl &server )
{
  APIMetaWeblog::setUrl( server );
  delete d->mXmlRpcClient;
  d->mXmlRpcClient = new KXmlRpc::Client( server );
  d->mXmlRpcClient->setUserAgent( userAgent() );
}

void APIMovableType::listRecentPostings( int number )
{
  //TODO
}

void APIMovableType::listTrackbackPings( KBlog::BlogPosting *posting ) {
  //TODO
  /*
  d->mXmlRpcClient->call( "mt.getTracebackPings", args,
    d, SLOT( slotListTrackbackPings(
              const QList<QVariant>&, const QVariant& ) ),
    d, SLOT( faultSlot( int, const QString&, const QVariant& ) ) );
  */
}

void APIMovableType::modifyPosting( KBlog::BlogPosting *posting )
{
  //TODO
}

#include "movabletype.moc"
