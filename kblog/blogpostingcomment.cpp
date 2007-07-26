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

#include "blogpostingcomment.h"
#include "blogpostingcomment_p.h"

#include <KDateTime>
#include <KUrl>

#include <QStringList>

namespace KBlog {

BlogPostingComment::BlogPostingComment(
    const QString &postingId, QObject* parent ) :
    QObject( parent ), d( new BlogPostingCommentPrivate )
{
  d->mStatus = New;
}

BlogPostingComment::~BlogPostingComment()
{
  delete d;
}

QString BlogPostingComment::title() const
{
  return d->mTitle;
}

void BlogPostingComment::setTitle( const QString &title )
{
  d->mTitle = title;
}

QString BlogPostingComment::content() const
{
  return d->mContent;
}

void BlogPostingComment::setContent( const QString &content )
{
  d->mContent = content;
}

QString BlogPostingComment::email() const
{
  return d->mEmail;
}

void BlogPostingComment::setEmail( const QString &email )
{
  d->mEmail = email;
}

KUrl BlogPostingComment::url() const
{
  return d->mUrl;
}

void BlogPostingComment::setUrl( const KUrl &url )
{
  d->mUrl = url;
}

BlogPostingComment::Status BlogPostingComment::status() const
{
  return d->mStatus;
}

void BlogPostingComment::setStatus( BlogPostingComment::Status status )
{
  d->mStatus = status;
}

QString BlogPostingComment::error() const
{
  return d->mError;
}

void BlogPostingComment::setError( const QString &error )
{
  d->mError = error;
}

} // namespace KBlog

#include "blogpostingcomment.moc"
