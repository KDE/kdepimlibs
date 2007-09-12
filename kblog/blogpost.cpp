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

#include "blogpost.h"
#include "blogpost_p.h"

#include "blog.h"

#include <KDateTime>
#include <KUrl>
#include <kcal/journal.h>

#include <QStringList>

namespace KBlog {

BlogPost::BlogPost( const KBlog::BlogPost& posting ):
    d_ptr( new BlogPostPrivate )
{
  d_ptr->q_ptr=this;
  d_ptr->mPrivate=posting.isPrivate();
  d_ptr->mPostingId=posting.postingId();
  d_ptr->mTitle=posting.title();
  d_ptr->mContent=posting.content();
  d_ptr->mCategories=posting.categories();
  d_ptr->mError=posting.error();
  d_ptr->mJournalId=posting.journalId();
  d_ptr->mStatus=posting.status();
  d_ptr->mCreationDateTime=posting.creationDateTime();
  d_ptr->mModificationDateTime=posting.modificationDateTime();
}

BlogPost::BlogPost( const QString &postingId ) :
    d_ptr( new BlogPostPrivate )
{
  d_ptr->q_ptr = this;
  d_ptr->mPrivate = false;
  d_ptr->mPostingId = postingId;
  d_ptr->mStatus = New;
}

BlogPost::BlogPost( const KCal::Journal &journal ) :
    d_ptr( new BlogPostPrivate )
{
  d_ptr->q_ptr = this;
  d_ptr->mPrivate = false;
  d_ptr->mPostingId = journal.customProperty( "KBLOG", "ID" );
  d_ptr->mJournalId = journal.uid();
  d_ptr->mStatus = New;
  d_ptr->mTitle = journal.summary();
  d_ptr->mContent = journal.description();
  d_ptr->mCategories = journal.categories();
  d_ptr->mCreationDateTime = journal.dtStart();
}

// BlogPost::BlogPost( const KCal::Journal &journal, BlogPostPrivate &dd )
//   : d_ptr( &dd )
// {
//   d_ptr->q_ptr = this;
//   d_ptr->mPrivate = false;
//   d_ptr->mPostingId = journal.customProperty( "KBLOG", "ID" );
//   d_ptr->mJournalId = journal.uid();
//   d_ptr->mStatus = New;
//   d_ptr->mTitle = journal.summary();
//   d_ptr->mContent = journal.description();
//   d_ptr->mCategories = journal.categories();
//   d_ptr->mCreationDateTime = journal.dtStart();
// }

BlogPost::~BlogPost()
{
  delete d_ptr;
}

KCal::Journal* BlogPost::journal( const Blog &blog ) const
{
  QString url = blog.url().url();
  QString username = blog.username();
  QString blogId = blog.blogId();
  // Generate unique ID. Should be unique enough...
  QString id = "kblog-" + url + '-' + blogId  + '-' + username +
      '-' + d_ptr->mPostingId;
  KCal::Journal *journal = new KCal::Journal();
  journal->setUid( id );
  journal->setSummary( d_ptr->mTitle );
  journal->setCategories( d_ptr->mCategories );
  journal->setDescription( d_ptr->mContent, true );
  journal->setDtStart( d_ptr->mCreationDateTime );
  journal->setCustomProperty( "KBLOG", "URL", url );
  journal->setCustomProperty( "KBLOG", "USER", blog.username() );
  journal->setCustomProperty( "KBLOG", "BLOG", blogId );
  journal->setCustomProperty( "KBLOG", "ID", d_ptr->mPostingId );
  return journal;
}

QString BlogPost::journalId() const
{
  return d_ptr->mJournalId;
}

bool BlogPost::isPrivate() const
{
  return d_ptr->mPrivate;
}

void BlogPost::setPrivate( bool privatePosting )
{
  d_ptr->mPrivate = privatePosting;
}

QString BlogPost::postingId() const
{
  return d_ptr->mPostingId;
}

void BlogPost::setPostingId( const QString &postingId )
{
  d_ptr->mPostingId = postingId;
}

QString BlogPost::title() const
{
  return d_ptr->mTitle;
}

void BlogPost::setTitle( const QString &title )
{
  d_ptr->mTitle = title;
}

QString BlogPost::content() const
{
  return d_ptr->mContent;
}

void BlogPost::setContent( const QString &content )
{
  d_ptr->mContent = content;
}

// QString BlogPost::abbreviatedContent() const
// {
//   //TODO
//   return 0;
// }
// 
// void BlogPost::setAbbreviatedContent( const QString &abbreviatedContent )
// {
//   Q_UNUSED( abbreviatedContent );
//   //TODO
// }

KUrl BlogPost::link() const
{
  return d_ptr->mLink;
}

void BlogPost::setLink( const KUrl &link ) const
{
  d_ptr->mLink = link;
}

KUrl BlogPost::permaLink() const
{
  return d_ptr->mPermaLink;
}

void BlogPost::setPermaLink( const KUrl &permalink ) const
{
  d_ptr->mPermaLink = permalink;
}

bool BlogPost::isCommentAllowed() const
{
  return d_ptr->mCommentAllowed;
}

void BlogPost::setCommentAllowed( bool commentAllowed )
{
  d_ptr->mCommentAllowed = commentAllowed;
}

bool BlogPost::isTrackBackAllowed() const
{
  return d_ptr->mCommentAllowed;
}

void BlogPost::setTrackBackAllowed ( bool allowTrackBacks )
{
  d_ptr->mTrackBackAllowed = allowTrackBacks;
}

QString BlogPost::summary() const
{
  return d_ptr->mSummary;
}

void BlogPost::setSummary( const QString &summary )
{
  d_ptr->mSummary = summary;
}

QStringList BlogPost::tags() const
{
  return d_ptr->mTags;
}

void BlogPost::setTags( const QStringList &tags )
{
  d_ptr->mTags = tags;
}

// QList<KUrl> BlogPost::trackBackUrls() const
// {
//   //TODO
//   return QList<KUrl>();
// }
// 
// void BlogPost::setTrackBackUrls( const QList<KUrl> &trackBackUrls )
// {
//   Q_UNUSED( trackBackUrls );
//   //TODO
// }

QString BlogPost::mood() const
{
  return d_ptr->mMood;
}

void BlogPost::setMood( const QString &mood )
{
  d_ptr->mMood = mood;
}

QString BlogPost::music() const
{
  return d_ptr->mMusic;
}

void BlogPost::setMusic( const QString &music )
{
  d_ptr->mMusic = music;
}

QStringList BlogPost::categories() const
{
  return d_ptr->mCategories;
}

void BlogPost::setCategories( const QStringList &categories )
{
  d_ptr->mCategories = categories;
}

KDateTime BlogPost::creationDateTime() const
{
  return d_ptr->mCreationDateTime;
}

void BlogPost::setCreationDateTime( const KDateTime &datetime )
{
  d_ptr->mCreationDateTime = datetime;
}

KDateTime BlogPost::modificationDateTime() const
{
  return d_ptr->mModificationDateTime;
}

void BlogPost::setModificationDateTime( const KDateTime &datetime )
{
  d_ptr->mModificationDateTime = datetime;
}

BlogPost::Status BlogPost::status() const
{
  return d_ptr->mStatus;
}

void BlogPost::setStatus( BlogPost::Status status )
{
  d_ptr->mStatus = status;
}

QString BlogPost::error() const
{
  return d_ptr->mError;
}

void BlogPost::setError( const QString &error )
{
  d_ptr->mError = error;
}

BlogPost& BlogPost::operator=(const BlogPost &posting)
{
  *this = BlogPost ( posting );
  return *this;
}

} // namespace KBlog

