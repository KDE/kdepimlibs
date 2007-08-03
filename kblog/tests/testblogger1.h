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

#ifndef KBLOG_TEST_BLOGGER1_H
#define KBLOG_TEST_BLOGGER1_H

#include <QtCore/QObject>
#include "kblog/blogger1.h"


template <class S, class T>class QMap;
template <class T>class QList;
class QTimer;
namespace KBlog{
  class BlogPosting;
}

class TestBlogger1 : public QObject
{
  Q_OBJECT
  private:
    void dumpPosting( const KBlog::BlogPosting* );
    KBlog::Blogger1 *b;
    KBlog::BlogPosting *p;
    QTimer *fetchUserInfoTimer;
    QTimer *listBlogsTimer;
    QTimer *listRecentPostingsTimer;
    QTimer *fetchPostingTimer;
    QTimer *modifyPostingTimer;
    QTimer *createPostingTimer;
    QTimer *removePostingTimer;
  private Q_SLOTS:
    void testValidity(); 
    void fetchUserInfo( const QMap<QString,QString>& );
    void listBlogs( const QMap<QString,QString>& );
    void listRecentPostings( const QList<KBlog::BlogPosting>& postings );
    void fetchPosting( KBlog::BlogPosting* posting );
    void modifyPosting( KBlog::BlogPosting* posting );
    void createPosting( KBlog::BlogPosting* posting );
    void removePosting( KBlog::BlogPosting* posting );
};

class TestBlogger1Warnings : public QObject
{
  Q_OBJECT
  private Q_SLOTS:
    void fetchUserInfoTimeoutWarning();
    void listBlogsTimeoutWarning();
    void listRecentPostingsTimeoutWarning();
    void fetchPostingTimeoutWarning();
    void modifyPostingTimeoutWarning();
    void createPostingTimeoutWarning();
    void removePostingTimeoutWarning();
    void error( KBlog::Blog::ErrorType type, const QString &errStr, KBlog::BlogPosting* );
};

#endif

