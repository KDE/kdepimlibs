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

#include <blog.h>
#include <blog_p.h>
#include <blogposting_p.h>

#include <kdebug.h>

#include <QtCore/QVariant>

using namespace KBlog;

APIBlog::APIBlog( const KUrl &server, QObject *parent ) :
  QObject( parent ), d( new APIBlogPrivate )
{
  Q_UNUSED( server );
}

APIBlog::~APIBlog()
{
  delete d;
}

void APIBlog::setPassword( const QString &pass )
{
  d->mPassword = pass;
}

QString APIBlog::password() const
{
  return d->mPassword;
}

QString APIBlog::username() const
{
  return d->mUsername;
}

void APIBlog::setUsername( const QString &userName )
{
  d->mUsername = userName;
}

void APIBlog::setBlogId( const QString &blogId )
{
  d->mBlogId = blogId;
}

QString APIBlog::blogId() const
{
  return d->mBlogId;
}

void APIBlog::setUrl( const KUrl &url )
{
  d->mUrl = url;
}

KUrl APIBlog::url() const
{
  return d->mUrl;
}

void APIBlog::setTimeZone( const KTimeZone &tz )
{
  d->mTimeZone = tz;
}

KTimeZone APIBlog::timeZone()
{
  return d->mTimeZone;
}

void APIBlog::setDownloadCount( int nr )
{
  d->mDownloadCount = nr;
}

int APIBlog::downloadCount() const
{
  return d->mDownloadCount;
}

#include "blog.moc"
