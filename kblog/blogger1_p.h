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

#ifndef BLOGGER1_P_H
#define BLOGGER1_P_H

#include "blogger1.h"
#include "blog_p.h"

#include <kxmlrpcclient/client.h>

#include <QList>

namespace KBlog {

class Blogger1Private : public BlogPrivate
{
  public:
    QString mAppId;
    KXmlRpc::Client *mXmlRpcClient;
    unsigned int callCounter; // TODO a better counter
    QMap<unsigned int,KBlog::BlogPosting*> callMap;
    Blogger1Private();
    virtual ~Blogger1Private();
    QList<QVariant> defaultArgs( const QString &id = QString() );

    virtual void slotListBlogs( const QList<QVariant> &result,
                                const QVariant &id );
    virtual void slotListRecentPostings( const QList<QVariant> &result,
                                         const QVariant &id );
    virtual void slotFetchPosting( const QList<QVariant> &result,
                                   const QVariant &id );
    virtual void slotCreatePosting( const QList<QVariant> &result,
                                    const QVariant &id );
    virtual void slotModifyPosting( const QList<QVariant> &result,
                                    const QVariant &id );
    virtual void slotRemovePosting( const QList<QVariant> &result,
                                    const QVariant &id );
    virtual void slotError( int number, const QString &errorString,
                            const QVariant &id );

    Q_DECLARE_PUBLIC(KBlog::Blogger1)

  private:
    virtual bool readPostingFromMap( BlogPosting *post,
                                     const QMap<QString, QVariant> &postInfo );
};

}

#endif
