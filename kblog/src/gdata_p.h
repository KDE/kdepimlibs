/*
  This file is part of the kblog library.

  Copyright (c) 2007 Christian Weilbach <christian_weilbach@web.de>

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

#ifndef KBLOG_GDATA_P_H
#define KBLOG_GDATA_P_H

#include "gdata.h"
#include "blog_p.h"

#include <syndication/loader.h>

class KJob;
class QDateTime;
class QByteArray;
template <class T, class S>class QMap;

namespace KIO
{
class Job;
}

namespace KBlog
{

class GDataPrivate : public BlogPrivate
{
public:
    QString mAuthenticationString;
    QDateTime mAuthenticationTime;
    QMap<KJob *, KBlog::BlogPost *> mCreatePostMap;
    QMap<KJob *, QMap<KBlog::BlogPost *, KBlog::BlogComment *> > mCreateCommentMap;
    QMap<KJob *, QMap<KBlog::BlogPost *, KBlog::BlogComment *> > mRemoveCommentMap;
    QMap<KJob *, KBlog::BlogPost *> mModifyPostMap;
    QMap<KJob *, KBlog::BlogPost *> mRemovePostMap;
    QMap<Syndication::Loader *, KBlog::BlogPost *> mFetchPostMap;
    QMap<Syndication::Loader *, KBlog::BlogPost *> mListCommentsMap;
    QMap<Syndication::Loader *, int> mListRecentPostsMap;
    QString mFullName;
    QString mProfileId;
    GDataPrivate();
    ~GDataPrivate();
    bool authenticate();
    virtual void slotFetchProfileId(KJob *);
    virtual void slotListBlogs(Syndication::Loader *,
                               Syndication::FeedPtr, Syndication::ErrorCode);
    virtual void slotListComments(Syndication::Loader *,
                                  Syndication::FeedPtr, Syndication::ErrorCode);
    virtual void slotListAllComments(Syndication::Loader *,
                                     Syndication::FeedPtr, Syndication::ErrorCode);
    virtual void slotListRecentPosts(Syndication::Loader *,
                                     Syndication::FeedPtr, Syndication::ErrorCode);
    virtual void slotFetchPost(Syndication::Loader *,
                               Syndication::FeedPtr, Syndication::ErrorCode);
    virtual void slotCreatePost(KJob *);
    virtual void slotModifyPost(KJob *);
    virtual void slotRemovePost(KJob *);
    virtual void slotCreateComment(KJob *);
    virtual void slotRemoveComment(KJob *);
    Q_DECLARE_PUBLIC(GData)
};

}
#endif
