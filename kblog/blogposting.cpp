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

#include "blogposting.h"
#include "blogposting_p.h"

#include "blog.h"

#include <KDateTime>
#include <KUrl>
#include <kcal/journal.h>

#include <QStringList>

namespace KBlog {

BlogPosting::BlogPosting( const KBlog::BlogPosting& posting ):
    d_ptr( new BlogPostingPrivate )
{
  d_ptr->q_ptr=this;
  d_ptr->mPublished=posting.isPublished();
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

BlogPosting::BlogPosting( const QString &postingId ) :
    d_ptr( new BlogPostingPrivate )
{
  d_ptr->q_ptr = this;
  d_ptr->mPublished = false;
  d_ptr->mPostingId = postingId;
  d_ptr->mStatus = New;
}

BlogPosting::BlogPosting( const QString &postingId, BlogPostingPrivate &dd )
  : d_ptr( &dd )
{
  d_ptr->q_ptr = this;
  d_ptr->mPublished = false;
  d_ptr->mPostingId = postingId;
  d_ptr->mStatus = New;
}

BlogPosting::BlogPosting( const KCal::Journal &journal ) :
    d_ptr( new BlogPostingPrivate )
{
  d_ptr->q_ptr = this;
  d_ptr->mPublished = false;
  d_ptr->mPostingId = journal.customProperty( "KBLOG", "ID" );
  d_ptr->mJournalId = journal.uid();
  d_ptr->mStatus = New;
  d_ptr->mTitle = journal.summary();
  d_ptr->mContent = journal.description();
  d_ptr->mCategories = journal.categories();
  d_ptr->mCreationDateTime = journal.dtStart();
}

BlogPosting::BlogPosting( const KCal::Journal &journal, BlogPostingPrivate &dd )
  : d_ptr( &dd )
{
  d_ptr->q_ptr = this;
  d_ptr->mPublished = false;
  d_ptr->mPostingId = journal.customProperty( "KBLOG", "ID" );
  d_ptr->mJournalId = journal.uid();
  d_ptr->mStatus = New;
  d_ptr->mTitle = journal.summary();
  d_ptr->mContent = journal.description();
  d_ptr->mCategories = journal.categories();
  d_ptr->mCreationDateTime = journal.dtStart();
}

BlogPosting::~BlogPosting()
{
  delete d_ptr;
}

KCal::Journal* BlogPosting::journal( const Blog &blog ) const
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
  journal->setDescription( d_ptr->mContent );
  journal->setDtStart( d_ptr->mCreationDateTime );
  journal->setCustomProperty( "KBLOG", "URL", url );
  journal->setCustomProperty( "KBLOG", "USER", blog.username() );
  journal->setCustomProperty( "KBLOG", "BLOG", blogId );
  journal->setCustomProperty( "KBLOG", "ID", d_ptr->mPostingId );
  return journal;
}

QString BlogPosting::journalId() const
{
  return d_ptr->mJournalId;
}

bool BlogPosting::isPublished() const
{
  return d_ptr->mPublished;
}

void BlogPosting::setPublished( bool published )
{
  d_ptr->mPublished = published;
}

QString BlogPosting::postingId() const
{
  return d_ptr->mPostingId;
}

void BlogPosting::setPostingId( const QString &postingId )
{
  d_ptr->mPostingId = postingId;
}

QString BlogPosting::title() const
{
  return d_ptr->mTitle;
}

void BlogPosting::setTitle( const QString &title )
{
  d_ptr->mTitle = title;
}

QString BlogPosting::content() const
{
  return d_ptr->mContent;
}

void BlogPosting::setContent( const QString &content )
{
  d_ptr->mContent = content;
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
  return d_ptr->mCategories;
}

void BlogPosting::setCategories( const QStringList &categories )
{
  d_ptr->mCategories = categories;
}

KDateTime BlogPosting::creationDateTime() const
{
  return d_ptr->mCreationDateTime;
}

void BlogPosting::setCreationDateTime( const KDateTime &datetime )
{
  d_ptr->mCreationDateTime = datetime;
}

KDateTime BlogPosting::modificationDateTime() const
{
  return d_ptr->mModificationDateTime;
}

void BlogPosting::setModificationDateTime( const KDateTime &datetime )
{
  d_ptr->mModificationDateTime = datetime;
}

BlogPosting::Status BlogPosting::status() const
{
  return d_ptr->mStatus;
}

void BlogPosting::setStatus( BlogPosting::Status status )
{
  d_ptr->mStatus = status;
//   emit statusChanged( status );
}

QString BlogPosting::error() const
{
  return d_ptr->mError;
}

void BlogPosting::setError( const QString &error )
{
  d_ptr->mError = error;
}

BlogPosting& BlogPosting::operator=(const BlogPosting &posting)
{
  *this = BlogPosting ( posting );
  return *this;
}

} // namespace KBlog

#include "blogposting.moc"
