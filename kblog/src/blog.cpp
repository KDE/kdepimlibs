/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  Copyright (c) 2007 Mike McQuaid <mike@mikemcquaid.com>

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

#include "blog.h"
#include "blog_p.h"
#include "blogpost_p.h"

#include <kdeversion.h>

#include <QDebug>

using namespace KBlog;

Blog::Blog( const QUrl &server, QObject *parent, const QString &applicationName,
            const QString &applicationVersion ) :
    QObject( parent ), d_ptr( new BlogPrivate )
{
  Q_UNUSED( server );
  d_ptr->q_ptr = this;
  setUserAgent( applicationName, applicationVersion );
}

Blog::Blog( const QUrl &server, BlogPrivate &dd, QObject *parent,
            const QString &applicationName, const QString &applicationVersion )
  : QObject( parent ), d_ptr( &dd )
{
  Q_UNUSED( server );
  d_ptr->q_ptr = this;
  setUserAgent( applicationName, applicationVersion );
}

Blog::~Blog()
{
  qDebug() << "~Blog()";
  delete d_ptr;
}

QString Blog::userAgent() const
{
  Q_D( const Blog );
  return d->mUserAgent;
}

void Blog::setUserAgent( const QString &applicationName,
                         const QString &applicationVersion )
{
  Q_D( Blog );
  QString userAgent;
  if ( !applicationName.isEmpty() &&
       !applicationVersion.isEmpty() ) {
    userAgent = QLatin1Char('(') + applicationName + QLatin1Char('/') + applicationVersion + QStringLiteral(") KDE-KBlog/");
  } else {
    userAgent = QStringLiteral("KDE-KBlog/");
  }
  userAgent += QLatin1String(KDE_VERSION_STRING);
  d->mUserAgent = userAgent;
}

void Blog::setPassword( const QString &pass )
{
  Q_D( Blog );
  d->mPassword = pass;
}

QString Blog::password() const
{
  Q_D( const Blog );
  return d->mPassword;
}

QString Blog::username() const
{
  Q_D( const Blog );
  return d->mUsername;
}

void Blog::setUsername( const QString &username )
{
  Q_D( Blog );
  d->mUsername = username;
}

void Blog::setBlogId( const QString &blogId )
{
  Q_D( Blog );
  d->mBlogId = blogId;
}

QString Blog::blogId() const
{
  Q_D( const Blog );
  return d->mBlogId;
}

void Blog::setUrl( const QUrl &url )
{
  Q_D( Blog );
  d->mUrl = url;
}

QUrl Blog::url() const
{
  Q_D( const Blog );
  return d->mUrl;
}

void Blog::setTimeZone( const KTimeZone &tz )
{
  Q_D( Blog );
  d->mTimeZone = tz;
}

KTimeZone Blog::timeZone()
{
  Q_D( const Blog );
  return d->mTimeZone;
}

BlogPrivate::BlogPrivate() : q_ptr(0)
{
}

BlogPrivate::~BlogPrivate()
{
  qDebug() << "~BlogPrivate()";
}
