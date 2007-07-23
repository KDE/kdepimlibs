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

#include "blogposting.h"

#include <KDateTime>

#include <QStringList>

namespace KBlog {

class BlogPosting::BlogPostingPrivate
{
  public:
  bool mPublish;
  QString mPostingId;
  QString mTitle;
  QString mContent;
  QStringList mCategories;
  QString mError;
  Status mStatus;
  KDateTime mCreationDateTime;
  KDateTime mModificationDateTime;
};

BlogPosting::BlogPosting(): d( new BlogPostingPrivate )
{
  d->mPublish=false;
  d->mStatus=New;
}

BlogPosting::BlogPosting( const QString &title, const QString &content,
                          const QStringList &categories, bool publish ):
  d( new BlogPostingPrivate )
{
  d->mTitle = title;
  d->mContent = content;
  d->mCategories = categories;
  d->mPublish = publish;
  d->mCreationDateTime = KDateTime::currentDateTime( KDateTime::Spec() );
  d->mStatus = New;
}

BlogPosting::BlogPosting( const QString &postingId ):
  d( new BlogPostingPrivate )
{
  d->mPostingId = postingId;
  d->mStatus = New;
}

BlogPosting::~BlogPosting()
{
  delete d;
}

bool BlogPosting::publish() const
{
  return d->mPublish;
}

void BlogPosting::setPublish( bool publish )
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

BlogPosting::Status BlogPosting::status() const
{
  return d->mStatus;
}

void BlogPosting::setStatus( BlogPosting::Status status )
{
  d->mStatus = status;
}

} // namespace KBlog

#include "blogposting.moc"