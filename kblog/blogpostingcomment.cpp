/*
  This file is part of the kblog library.

  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
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
    const BlogPostingComment& c ) :
    d_ptr( new BlogPostingCommentPrivate )
{
  d_ptr->q_ptr=this;
  d_ptr->mTitle=c.title();
  d_ptr->mContent=c.content();
  d_ptr->mEmail=c.email();
  d_ptr->mName=c.name();
  d_ptr->mCommentId=c.commentId();
  d_ptr->mUrl=c.url();
  d_ptr->mError=c.error();
  d_ptr->mStatus=c.status();
  d_ptr->mModificationDateTime=c.modificationDateTime();
  d_ptr->mCreationDateTime=c.creationDateTime();
}

BlogPostingComment::BlogPostingComment(
    const QString &commentId ) :
    d_ptr( new BlogPostingCommentPrivate )
{
  d_ptr->q_ptr=this;
  d_ptr->mStatus = New;
  d_ptr->mCommentId = commentId;
}

BlogPostingComment::~BlogPostingComment()
{
  delete d_ptr;
}

QString BlogPostingComment::title() const
{
  return d_ptr->mTitle;
}

void BlogPostingComment::setTitle( const QString &title )
{
  d_ptr->mTitle = title;
}

QString BlogPostingComment::content() const
{
  return d_ptr->mContent;
}

void BlogPostingComment::setContent( const QString &content )
{
  d_ptr->mContent = content;
}

QString BlogPostingComment::commentId() const
{
  return d_ptr->mCommentId;
}

void BlogPostingComment::setCommentId( const QString &commentId )
{
  d_ptr->mCommentId = commentId;
}

QString BlogPostingComment::email() const
{
  return d_ptr->mEmail;
}

void BlogPostingComment::setEmail( const QString &email )
{
  d_ptr->mEmail = email;
}

QString BlogPostingComment::name() const
{
  return d_ptr->mName;
}

void BlogPostingComment::setName( const QString &name )
{
  d_ptr->mName = name;
}
KUrl BlogPostingComment::url() const
{
  return d_ptr->mUrl;
}

void BlogPostingComment::setUrl( const KUrl &url )
{
  d_ptr->mUrl = url;
}

KDateTime BlogPostingComment::modificationDateTime() const
{
  return d_ptr->mModificationDateTime;
}

void BlogPostingComment::setModificationDateTime( const KDateTime &datetime )
{
  d_ptr->mModificationDateTime=datetime;
}

KDateTime BlogPostingComment::creationDateTime() const
{
  return d_ptr->mCreationDateTime;
}

void BlogPostingComment::setCreationDateTime( const KDateTime &datetime )
{
  d_ptr->mCreationDateTime= datetime;
}


BlogPostingComment::Status BlogPostingComment::status() const
{
  return d_ptr->mStatus;
}

void BlogPostingComment::setStatus( BlogPostingComment::Status status )
{
  d_ptr->mStatus = status;
}

QString BlogPostingComment::error() const
{
  return d_ptr->mError;
}

void BlogPostingComment::setError( const QString &error )
{
  d_ptr->mError = error;
}

BlogPostingComment& BlogPostingComment::operator=(const BlogPostingComment &comment)
{
  *this = BlogPostingComment ( comment );
  return *this;
}

} // namespace KBlog
