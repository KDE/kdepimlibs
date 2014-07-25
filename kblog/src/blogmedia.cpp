/*
  This file is part of the kblog library.

  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  Copyright (c) 2007 Mike McQuaid <mike@mikemcquaid.com>

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

#include "blogmedia.h"

#include <QByteArray>
#include <QString>
#include <QUrl>

namespace KBlog
{

class BlogMediaPrivate
{
public:
    BlogMedia *q_ptr;
    QString mName;
    QUrl mUrl;
    QString mMimetype;
    QString mError;
    QByteArray mData;
    BlogMedia::Status mStatus;
};

BlogMedia::BlogMedia(): d_ptr(new BlogMediaPrivate)
{
    d_ptr->q_ptr = this;
}

BlogMedia::BlogMedia(const BlogMedia &m)
    : d_ptr(new BlogMediaPrivate)
{
    d_ptr->q_ptr = this;
    d_ptr->mName = m.name();
    d_ptr->mUrl = m.url();
    d_ptr->mMimetype = m.mimetype();
    d_ptr->mData = m.data();
    d_ptr->mStatus = m.status();
    d_ptr->mError = m.error();
}

BlogMedia::~BlogMedia()
{
    delete d_ptr;
}

QString BlogMedia::name() const
{
    return d_ptr->mName;
}

void BlogMedia::setName(const QString &name)
{
    d_ptr->mName = name;
}

QUrl BlogMedia::url() const
{
    return d_ptr->mUrl;
}

void BlogMedia::setUrl(const QUrl &url)
{
    d_ptr->mUrl = url;
}

QString BlogMedia::mimetype() const
{
    return d_ptr->mMimetype;
}

void BlogMedia::setMimetype(const QString &mimetype)
{
    d_ptr->mMimetype = mimetype;
}

QByteArray BlogMedia::data() const
{
    return d_ptr->mData;
}

void BlogMedia::setData(const QByteArray &data)
{
    d_ptr->mData = data;
}

BlogMedia::Status BlogMedia::status() const
{
    return d_ptr->mStatus;
}

void BlogMedia::setStatus(BlogMedia::Status status)
{
    d_ptr->mStatus = status;
}

QString BlogMedia::error() const
{
    return d_ptr->mError;
}

void BlogMedia::setError(const QString &error)
{
    d_ptr->mError = error;
}

BlogMedia &BlogMedia::operator=(const BlogMedia &m)
{
    BlogMedia copy(m);
    swap(copy);
    return *this;
}

} //namespace KBlog
