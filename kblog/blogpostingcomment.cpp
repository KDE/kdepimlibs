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
    QObject( parent ), d_ptr( new BlogPostingCommentPrivate )
{
  d_func()->mStatus = New;
}

BlogPostingComment::~BlogPostingComment()
{
  delete d_ptr;
}

QString BlogPostingComment::title() const
{
  return d_func()->mTitle;
}

void BlogPostingComment::setTitle( const QString &title )
{
  d_func()->mTitle = title;
}

QString BlogPostingComment::content() const
{
  return d_func()->mContent;
}

void BlogPostingComment::setContent( const QString &content )
{
  d_func()->mContent = content;
}

QString BlogPostingComment::commentId() const
{
  return d_func()->mCommentId;
}

void BlogPostingComment::setCommentId( const QString &commentId )
{
  d_func()->mCommentId = commentId;
}

QString BlogPostingComment::email() const
{
  return d_func()->mEmail;
}

void BlogPostingComment::setEmail( const QString &email )
{
  d_func()->mEmail = email;
}

KUrl BlogPostingComment::url() const
{
  return d_func()->mUrl;
}

void BlogPostingComment::setUrl( const KUrl &url )
{
  d_func()->mUrl = url;
}

KDateTime BlogPostingComment::modificationDateTime() const
{
  return d_func()->mModificationDateTime;
}

void BlogPostingComment::setModificationDateTime( const KDateTime &datetime )
{
  d_func()->mModificationDateTime=datetime;
}

KDateTime BlogPostingComment::creationDateTime() const
{
  return d_func()->mCreationDateTime;
}

void BlogPostingComment::setCreationDateTime( const KDateTime &datetime )
{
  d_func()->mCreationDateTime= datetime;
}


BlogPostingComment::Status BlogPostingComment::status() const
{
  return d_func()->mStatus;
}

void BlogPostingComment::setStatus( BlogPostingComment::Status status )
{
  d_func()->mStatus = status;
}

QString BlogPostingComment::error() const
{
  return d_func()->mError;
}

void BlogPostingComment::setError( const QString &error )
{
  d_func()->mError = error;
}

} // namespace KBlog

#include "blogpostingcomment.moc"
