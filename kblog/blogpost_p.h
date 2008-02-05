/*
  This file is part of the kblog library.

  Copyright (c) 2007 Christian Weilbach <christian_weilbach@web.de>
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

#ifndef BLOGPOST_P_H
#define BLOGPOST_P_H

#include "blogpost.h"

#include <QtCore/QStringList>
#include <kdatetime.h>
#include <kurl.h>

namespace KBlog{

class BlogPostPrivate
{
  public:
    bool mPrivate;
    BlogPost *q_ptr;
    QString mPostId;
    QString mTitle;
    QString mContent;
    QStringList mCategories;
    QString mError;
    QString mJournalId;
    QString mSummary;
    QStringList mTags;
    QString mMood;
    QString mMusic;
    bool mCommentAllowed;
    bool mTrackBackAllowed;
    KUrl mLink, mPermaLink;
    BlogPost::Status mStatus;
    KDateTime mCreationDateTime;
    KDateTime mModificationDateTime;
};

} // namespace
#endif
