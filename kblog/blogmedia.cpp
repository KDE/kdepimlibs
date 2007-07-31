/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006 Christian Weilbach <christian_weilbach@web.de>
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

#include "blogmedia.h"

#include <QByteArray>
#include <QString>
#include <kurl.h>

namespace KBlog {

class BlogMediaPrivate
{
  public:
    BlogMedia *q_ptr;
    QString mName;
    KUrl mUrl;
    QString mMimetype;
    QByteArray mData;
    BlogMedia::Status mStatus;
    Q_DECLARE_PUBLIC(BlogMedia)
};

BlogMedia::BlogMedia( QObject* parent ): QObject( parent ),
                      d_ptr( new BlogMediaPrivate )
{
}

BlogMedia::~BlogMedia()
{
  delete d_ptr;
}

QString BlogMedia::name() const
{
  return d_func()->mName;
}

void BlogMedia::setName( const QString &name )
{
  d_func()->mName = name;
}

KUrl BlogMedia::url() const
{
  return d_func()->mUrl;
}

void BlogMedia::setUrl( const KUrl &url )
{
  d_func()->mUrl = url;
}

QString BlogMedia::mimetype() const
{
  return d_func()->mMimetype;
}

void BlogMedia::setMimetype( const QString &mimetype )
{
  d_func()->mMimetype = mimetype;
}

QByteArray BlogMedia::data() const
{
  return d_func()->mData;
}

void BlogMedia::setData( const QByteArray &data )
{
  d_func()->mData = data;
}

BlogMedia::Status BlogMedia::status() const
{
  return d_func()->mStatus;
}

void BlogMedia::setStatus( BlogMedia::Status status )
{
  emit statusChanged( status );
  d_func()->mStatus = status;
}

} //namespace KBlog

#include "blogmedia.moc"

