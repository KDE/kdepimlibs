/*
  This file is part of the kblog library.

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

#include "blogger1_p.h"
#include "blogposting.h"

#include <kxmlrpcclient/client.h>

#include <KDebug>
#include <KDateTime>
#include <KLocale>

#include <QList>

using namespace KBlog;

Blogger1Private::Blogger1Private()
{
  mXmlRpcClient = 0;
  callCounter = 1;
}

Blogger1Private::~Blogger1Private()
{
  delete mXmlRpcClient;
}

QList<QVariant> Blogger1Private::defaultArgs( const QString &id )
{
  QList<QVariant> args;
  args << QVariant( QString( "0123456789ABCDEF" ) ); //AppKey
  if ( !id.isNull() ) {
    args << QVariant( id );
  }
  args << QVariant( parent->username() )
       << QVariant( parent->password() );
  return args;
}

#include "blogger1_p.moc"
