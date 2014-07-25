/*
  This file is part of the kblog library.

  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>

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

#ifndef KBLOG_TEST_DATA_H_
#define KBLOG_TEST_DATA_H_

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QUrl>
#include <ktimezone.h>
#include <QtCore/QDateTime>

QUrl mUrl(QLatin1String("http://kblogunittests.wordpress.com/xmlrpc.php"));
QString mUsername(QLatin1String("kblogunittests"));
QString mPassword(QLatin1String("k0nt4ctbl0g"));
QString mBlogId(QLatin1String("1"));

QDateTime mCreationDateTime(QDateTime::currentDateTime());
QDateTime mModificationDateTime(QDateTime::currentDateTime());
QString mTitle(QLatin1String("TestBlog"));
QString mContent(QLatin1String("TestBlog: <strong>posted</strong> content."));
QString mModifiedContent(QLatin1String("TestBlog: <strong>modified</strong>content."));
bool mPrivate = false;
QString mPostId(QLatin1String("113"));

QString mCommentTitle(QLatin1String("TestBlog Comment"));
QString mCommentContent(QLatin1String("TestBlog: posted comment."));
QString mCommentEmail(QLatin1String("fancy_mail@not.valid"));
bool mCommentAllowed = true;
bool mTrackBackAllowed = true;
QStringList mTags(QLatin1String("funny"));
QStringList mCategories(QLatin1String("funny"));
QString mSummary = QLatin1String("A simple summary.");

#endif
