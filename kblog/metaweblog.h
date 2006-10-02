/*
    This file is part of the kblog library.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2006 Christian Weilbach <christian@whiletaker.homeip.net>

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

#ifndef API_METAWEBLOG_H
#define API_METAWEBLOG_H

#include "blog.h"

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QList>

namespace KBlog {

class KBLOG_EXPORT APIMetaWeblog : public APIBlog
{
  public:
    APIMetaWeblog( const KUrl &server, QObject *parent = 0L, const char *name = 0L );
    ~APIMetaWeblog();
    QString getFunctionName( blogFunctions type );
    QString interfaceName() const { return "MetaWeblog API"; }

    void userInfo();
    void listBlogs();
    void listPostings();
    void listCategories();
    void fetchPosting( const QString &postId );
    void modifyPosting( KBlog::BlogPosting *posting );
    void createPosting( KBlog::BlogPosting *posting );
    void createMedia( KBlog::BlogMedia *media );
    void removePosting( const QString &postId );

public slots:
    void slotUserInfo( const QList<QVariant> &result, const QVariant &id );
    void slotListBlogs( const QList<QVariant> &result, const QVariant &id );
    void slotListPostings( const QList<QVariant> &result, const QVariant &id );
    void slotListCategories( const QList<QVariant> &result, const QVariant &id );
    void slotFetchPosting( const QList<QVariant> &result, const QVariant &id );
    void slotCreatePosting( const QList<QVariant> &result, const QVariant &id );
    void slotCreateMedia( const QList<QVariant> &result, const QVariant &id );
    void faultSlot( int, const QString&, const QVariant& );
protected:
    bool readPostingFromMap( BlogPosting *post, const QMap<QString, QVariant> &postInfo );

private:
  class Private;
  Private* const d;
};

}
#endif
