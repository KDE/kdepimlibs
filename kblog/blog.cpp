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

#include <kdebug.h>

#include <QtCore/QVariant>

using namespace KBlog;

BlogPosting::BlogPosting(): d( new BlogPostingPrivate )
{
  d->mPublish=false;
  d->mDeleted=false;
  d->mUploaded=false;
}

BlogPosting::BlogPosting( const QString &title, const QString &content,
                          const QStringList &categories, const bool publish ):
  d( new BlogPostingPrivate )
{
  d->mTitle = title;
  d->mContent = content;
  d->mCategories = categories;
  d->mPublish = publish;
  d->mCreationDateTime = KDateTime::currentDateTime( KDateTime::Spec() );
  d->mUploaded = false;
  d->mDeleted = false;
  d->mDateFormatExtended=true;
  d->mTimeFormatExtended=true;
}

BlogPosting::~BlogPosting()
{
  delete d;
}

bool BlogPosting::publish() const
{
  return d->mPublish;
}

void BlogPosting::setPublish( const bool publish )
{
  d->mPublish = publish;
}

QString BlogPosting::postingId() const
{
  return d->mPostingId;
}

void BlogPosting::setPostingId( const QString &postingId )
{
  d->mPostingId = postingId;
}

QString BlogPosting::title() const
{
  return d->mTitle;
}

void BlogPosting::setTitle( const QString &title )
{
  d->mTitle = title;
}

QString BlogPosting::content() const
{
  return d->mContent;
}

void BlogPosting::setContent( const QString &content )
{
  d->mContent = content;
}

QStringList BlogPosting::categories() const
{
  return d->mCategories;
}

void BlogPosting::setCategories( const QStringList &categories )
{
  d->mCategories = categories;
}

KDateTime BlogPosting::creationDateTime() const
{
  return d->mCreationDateTime;
}

void BlogPosting::setCreationDateTime( const KDateTime &datetime )
{
  d->mCreationDateTime = datetime;
}

KDateTime BlogPosting::modificationDateTime() const
{
  return d->mModificationDateTime;
}

void BlogPosting::setModificationDateTime( const KDateTime &datetime )
{
  d->mModificationDateTime = datetime;
}

bool BlogPosting::useExtendedDateFormat() const
{
  return d->mDateFormatExtended;
}

void BlogPosting::setUseExtendedDateFormat( bool extended )
{
  d->mDateFormatExtended=extended;
}

bool BlogPosting::useExtendedTimeFormat() const
{
  return d->mTimeFormatExtended;
}

void BlogPosting::setUseExtendedTimeFormat( bool extended )
{
  d->mDateFormatExtended=extended;
}

BlogMedia::BlogMedia(): d( new BlogMediaPrivate )
{
}

BlogMedia::~BlogMedia()
{
  delete d;
}

QString BlogMedia::name() const
{
  return d->mName;
}

void BlogMedia::setName( const QString &name )
{
  d->mName = name;
}

QString BlogMedia::mimetype() const
{
  return d->mMimetype;
}

void BlogMedia::setMimetype( const QString &mimetype )
{
  d->mMimetype = mimetype;
}

QByteArray BlogMedia::data() const
{
  return d->mData;
}

void BlogMedia::setData( const QByteArray &data )
{
  d->mData = data;
}

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

void APIBlog::setUsername( const QString &uname )
{
  d->mUsername = uname;
}

QString APIBlog::username() const
{
  return d->mUsername;
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

bool APIBlog::removePosting( KBlog::BlogPosting *posting )
{
  return removePosting( posting->postingId() );
}

#include "blog.moc"
