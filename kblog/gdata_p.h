/*
    This file is part of the kblog library.

    Copyright (c) 2007 Christian Weilbach <christian@whiletaker.homeip.net>

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

#include <syndication/loader.h>

#include <QDateTime>

class KJob;

using namespace KBlog;

class GData::GDataPrivate : public QObject
{
  Q_OBJECT
  private:
    QString mFetchPostingId;
    QString mAuthenticationString;
    QDateTime mAuthenticationTime;
  public:
    GData* parent;
    QByteArray mBuffer;
    QString mUsername;
    QString mFullName;
    QString mProfileId;
    GDataPrivate();
    ~GDataPrivate();
    QString getFetchPostingId(){ return mFetchPostingId; }
    void setFetchPostingId( const QString &pId ) { mFetchPostingId=pId; }
    QString authenticate();
  public Q_SLOTS:
    void slotListedRecentPostings( Syndication::Loader *,
                                   Syndication::FeedPtr,
                                   Syndication::ErrorCode );
    void slotFetchedPosting( Syndication::Loader *,
                             Syndication::FeedPtr,
                             Syndication::ErrorCode );
    void slotListedBlogs( Syndication::Loader *,
                          Syndication::FeedPtr,
                          Syndication::ErrorCode );
    void slotData( KIO::Job *, const QByteArray& );
    void slotCreatedPosting( KJob *job );
};

#endif
