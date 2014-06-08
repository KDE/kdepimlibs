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

QUrl mUrl( "http://kblogunittests.wordpress.com/xmlrpc.php" );
QString mUsername( "kblogunittests" );
QString mPassword( "k0nt4ctbl0g" );
QString mBlogId( "1" );

QDateTime mCreationDateTime( QDateTime::currentDateTime() );
QDateTime mModificationDateTime( QDateTime::currentDateTime() );
QString mTitle( "TestBlog" );
QString mContent( "TestBlog: <strong>posted</strong> content." );
QString mModifiedContent( "TestBlog: <strong>modified</strong>content." );
bool mPrivate = false;
QString mPostId( QString( "113" ) );

QString mCommentTitle( "TestBlog Comment" );
QString mCommentContent( "TestBlog: posted comment." );
QString mCommentEmail( QString( "fancy_mail@not.valid" ) );
bool mCommentAllowed = true;
bool mTrackBackAllowed = true;
QStringList mTags( "funny" );
QStringList mCategories( "funny" );
QString mSummary = "A simple summary.";

#endif
