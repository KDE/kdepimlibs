/*
  This file is part of the kblog library.

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

#ifndef LIVEJOURNAL_P_H
#define LIVEJOURNAL_P_H

#include "livejournal.h"

#include <kxmlrpcclient/client.h>

using namespace KBlog;

class APILiveJournal::APILiveJournalPrivate : public QObject
{
  Q_OBJECT
  public:
    QString mAppId;
    QMap<QString,QString> mCategories;
    APILiveJournal *parent;
    KXmlRpc::Client *mXmlRpcClient;
    QString mServerMessage;
    QString mUserId;
    QString mFullName;

    APILiveJournalPrivate();
    ~APILiveJournalPrivate();
    QList<QVariant> defaultArgs( const QString &id = QString() );

  private:
    bool readPostingFromMap( BlogPosting *post,
                             const QMap<QString, QVariant> &postInfo );

  public Q_SLOTS:
    void slotAddFriend( const QList<QVariant> &result, const QVariant &id );
    void slotAssignFriendToCategory( const QList<QVariant> &result,
                                     const QVariant &id );
    void slotCreatePosting( const QList<QVariant> &result, const QVariant &id );
    void slotDeleteFriend( const QList<QVariant> &result, const QVariant &id );
    void slotExpireCookie( const QList<QVariant> &result, const QVariant &id );
    void slotExpireAllCookies( const QList<QVariant> &result,
                               const QVariant &id );
    void slotFault( int, const QString&, const QVariant& );
    void slotFetchPosting( const QList<QVariant> &result, const QVariant &id );
    void slotFetchUserInfo( const QList<QVariant> &result, const QVariant &id );
    void slotGenerateCookie( const QList<QVariant> &result,
                             const QVariant &id );
    void slotListCategories( const QList<QVariant> &result,
                             const QVariant &id );
    void slotListFriends( const QList<QVariant> &result, const QVariant &id );
    void slotListFriendsOf( const QList<QVariant> &result, const QVariant &id );
    void slotListMoods( const QList<QVariant> &result, const QVariant &id );
    void slotListPictureKeywords( const QList<QVariant> &result,
                                  const QVariant &id );
    void slotListRecentPostings( const QList<QVariant> &result,
                                 const QVariant &id );
    void slotModifyPosting( const QList<QVariant> &result, const QVariant &id );

};

#endif
