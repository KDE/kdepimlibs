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

#ifndef BLOG_P_H
#define BLOG_P_H

#include <blog.h>

#include <QtCore/QMutex>

using namespace KBlog;

class APIBlog::APIBlogPrivate
{
  public:
    APIBlog *parent;
    QString mAppId;
    QString mBlogId;
    QString mUsername;
    QString mPassword;
    KUrl mUrl;
    KTimeZone mTimeZone;
    unsigned int mDownloadCount;
    QMutex mLock;
};

#endif
