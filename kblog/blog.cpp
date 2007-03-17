/*
    This file is part of the kblog library.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2006 Christian Weilbach <christian@whiletaker.homeip.net>

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

#include <kxmlrpcclient/client.h>
#include <blog.h>

#include <kdebug.h>
#include <kdatetime.h>

#include <QtCore/QVariant>
#include <QtCore/QList>

using namespace KBlog;

class BlogPosting::Private 
{
  public:
    bool mPublish;
    QString mPostingId;
    QString mTitle;
    QString mContent;
    QString mCategory;
    QString mFingerprint; // TODO what is that for?
    KDateTime mCreationDateTime;
    KDateTime mModificationDateTime;
    bool mDeleted;
    bool mUploaded;
};

BlogPosting::BlogPosting(): d( new Private ) 
{
  d->mPublish=false;
  d->mDeleted=false;
  d->mUploaded=false;
}

BlogPosting::BlogPosting( const QString& title, const QString& content,
                          const QString& category, const bool publish ):
			  d( new Private )
{
  d->mTitle = title;
  d->mContent = content;
  d->mCategory = category;
  d->mPublish = publish;
  d->mCreationDateTime = KDateTime::currentDateTime( KDateTime::Spec() );
  d->mUploaded = false;
  d->mDeleted = false;
}

BlogPosting::~BlogPosting() 
{
  delete d;
}

bool BlogPosting::publish() const { return d->mPublish; }
void BlogPosting::setPublish( const bool publish ) { d->mPublish = publish; }

QString BlogPosting::postingId() const { return d->mPostingId; }
void BlogPosting::setPostingId( const QString &postingId ) { assignPostId( postingId ); d->mPostingId = postingId; }

QString BlogPosting::title() const { return d->mTitle; }
void BlogPosting::setTitle( const QString &title ) { d->mTitle = title; }

QString BlogPosting::content() const { return d->mContent; }
void BlogPosting::setContent( const QString &content ) { d->mContent = content; }

QString BlogPosting::category() const { return d->mCategory; }
void BlogPosting::setCategory( const QString &category ) { d->mCategory = category; }

//QString BlogPosting::fingerprint() const { return d->mFingerprint; } // TODO do we really need it?
//void BlogPosting::setFingerprint( const QString &fp ) { d->mFingerprint = fp; }

KDateTime BlogPosting::creationDateTime() const { return d->mCreationDateTime; }
void BlogPosting::setCreationDateTime( const KDateTime &datetime ) { d->mCreationDateTime = datetime; }

KDateTime BlogPosting::modificationDateTime() const { return d->mModificationDateTime; }
void BlogPosting::setModificationDateTime( const KDateTime &datetime ) { d->mModificationDateTime = datetime; }

bool BlogPosting::uploaded() const { return d->mUploaded; }
void BlogPosting::setUploaded( const bool uploaded ) { d->mUploaded = uploaded; }

bool BlogPosting::deleted() const { return d->mDeleted; }
void BlogPosting::setDeleted( const bool deleted ) { d->mDeleted =  deleted; }


class BlogMedia::Private 
{
  public:
    QString mName;
    QString mMimetype;
    QByteArray mData;
};

BlogMedia::BlogMedia(): d( new Private ) {}

BlogMedia::~BlogMedia() 
{
  delete d;
}

QString BlogMedia::name() const { return d->mName; }
void BlogMedia::setName( const QString& name ) { d->mName = name; }

QString BlogMedia::mimetype() const { return d->mMimetype; }
void BlogMedia::setMimetype( const QString& mimetype ) { d->mMimetype = mimetype; }

QByteArray BlogMedia::data() const { return d->mData; }
void BlogMedia::setData( const QByteArray& data ) { d->mData = data; }


class APIBlog::Private
{
  public:
    QString mAppId;
    QString mBlogId;
    QString mUsername;
    QString mPassword;
    KUrl mUrl;
    KTimeZone mTimezone;
    unsigned int mDownloadCount;
};

APIBlog::APIBlog( const KUrl &url, QObject *parent, const char *name ) :
    QObject( parent ), d( new Private ) 
{
}

APIBlog::~APIBlog()
{
// TODO check for pending queries of client? smart pointer or sth.?
  delete d;
}

void APIBlog::setPassword( const QString &pass ) { d->mPassword = pass; }
QString APIBlog::password() const { return d->mPassword; }

void APIBlog::setUsername( const QString &uname ) { d->mUsername = uname; }
QString APIBlog::username() const { return d->mUsername; }

void APIBlog::setBlogId( const QString &blogId ) { d->mBlogId = blogId; }
QString APIBlog::blogId() const { return d->mBlogId; }

void APIBlog::setUrl( const KUrl& url ) { d->mUrl = url; }
KUrl APIBlog::url() const { return d->mUrl; }

void APIBlog::setTimezone( const KTimeZone& tz ) { d->mTimezone = tz; }
KTimeZone APIBlog::timezone() { return d->mTimezone; }

void APIBlog::setDownloadCount( int nr ) { d->mDownloadCount = nr; }
int APIBlog::downloadCount() const { return d->mDownloadCount; }

void APIBlog::removePosting( KBlog::BlogPosting *posting )
{
  removePosting( posting->postingId() );
  posting->setDeleted( true );
}

void APIBlog::fetchPosting( KBlog::BlogPosting *posting )
{
  fetchPosting( posting->postingId() );
}

#include "blog.moc"

