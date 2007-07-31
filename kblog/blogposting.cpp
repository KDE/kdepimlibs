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

#include "blogposting.h"
#include "blogposting_p.h"

#include <KDateTime>
#include <KUrl>

#include <QStringList>

namespace KBlog {

BlogPosting::BlogPosting( const QString &postingId, QObject* parent ) :
    QObject( parent ), d_ptr( new BlogPostingPrivate )
{
  d_ptr->mPublished = false;
  d_ptr->mPostingId = postingId;
  d_ptr->mStatus = New;
}

BlogPosting::~BlogPosting()
{
  delete d_ptr;
}

bool BlogPosting::isPublished() const
{
  return d_func()->mPublished;
}

void BlogPosting::setPublished( bool published )
{
  d_func()->mPublished = published;
}

QString BlogPosting::postingId() const
{
  return d_func()->mPostingId;
}

void BlogPosting::setPostingId( const QString &postingId )
{
  d_func()->mPostingId = postingId;
}

QString BlogPosting::title() const
{
  return d_func()->mTitle;
}

void BlogPosting::setTitle( const QString &title )
{
  d_func()->mTitle = title;
}

QString BlogPosting::content() const
{
  return d_func()->mContent;
}

void BlogPosting::setContent( const QString &content )
{
  d_func()->mContent = content;
}

QString BlogPosting::abbreviatedContent() const
{
  //TODO
  return 0;
}

void BlogPosting::setAbbreviatedContent( const QString &abbreviatedContent )
{
  //TODO
}

KUrl BlogPosting::link() const
{
  //TODO
  return KUrl();
}

void BlogPosting::setLink( const KUrl &link ) const
{
  //TODO
}

KUrl BlogPosting::permalink() const
{
  //TODO
  return KUrl();
}

void BlogPosting::setPermalink( const KUrl &permalink ) const
{
  //TODO
}

bool BlogPosting::isCommentAllowed() const
{
  //TODO
  return false;
}

void BlogPosting::setCommentAllowed( bool commentAllowed )
{
  //TODO
}

bool BlogPosting::isTrackBackAllowed() const
{
  //TODO
  return false;
}

void BlogPosting::setTrackBackAllowed ( bool allowTrackBacks )
{
  //TODO
}

QString BlogPosting::summary() const
{
  //TODO
  return QString();
}

void BlogPosting::setSummary( const QString &summary )
{
  //TODO
}

QString BlogPosting::tags() const
{
  //TODO
  return QString();
}

void BlogPosting::setTags( const QString &tags )
{
  //TODO
}

QList<KUrl> BlogPosting::trackBackUrls() const
{
  //TODO
  return QList<KUrl>();
}

void BlogPosting::setTrackBackUrls( const QList<KUrl> &trackBackUrls )
{
  //TODO
}

QString BlogPosting::mood() const
{
  //TODO
  return QString();
}

void BlogPosting::setMood( const QString &mood )
{
  //TODO
}

QString BlogPosting::music() const
{
  //TODO
  return QString();
}

void BlogPosting::setMusic( const QString &music )
{
  //TODO
}

QStringList BlogPosting::categories() const
{
  return d_func()->mCategories;
}

void BlogPosting::setCategories( const QStringList &categories )
{
  d_func()->mCategories = categories;
}

KDateTime BlogPosting::creationDateTime() const
{
  return d_func()->mCreationDateTime;
}

void BlogPosting::setCreationDateTime( const KDateTime &datetime )
{
  d_func()->mCreationDateTime = datetime;
}

KDateTime BlogPosting::modificationDateTime() const
{
  return d_func()->mModificationDateTime;
}

void BlogPosting::setModificationDateTime( const KDateTime &datetime )
{
  d_func()->mModificationDateTime = datetime;
}

BlogPosting::Status BlogPosting::status() const
{
  return d_func()->mStatus;
}

void BlogPosting::setStatus( BlogPosting::Status status )
{
  d_func()->mStatus = status;
  emit statusChanged( status );
}

QString BlogPosting::error() const
{
  return d_func()->mError;
}

void BlogPosting::setError( const QString &error )
{
  d_func()->mError = error;
}

} // namespace KBlog

#include "blogposting.moc"