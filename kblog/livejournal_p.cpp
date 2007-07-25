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

#include <livejournal_p.h>
#include "blogposting.h"

#include <kxmlrpcclient/client.h>

#include <kdebug.h>
#include <kdatetime.h>
#include <klocale.h>

#include <QtCore/QVariant>

using namespace KBlog;

APILiveJournal::APILiveJournalPrivate::APILiveJournalPrivate()
{
  mXmlRpcClient = 0;
}

APILiveJournal::APILiveJournalPrivate::~APILiveJournalPrivate()
{
  delete mXmlRpcClient;
}

QList<QVariant> APILiveJournal::APILiveJournalPrivate::defaultArgs(
    const QString &id )
{
  QList<QVariant> args;

  if ( id.toInt() ) {
    args << QVariant( id.toInt() );
  }
  if ( !id.toInt() && !id.isNull() ){
    args << QVariant( id );
  }
  args << QVariant( parent->username() )
       << QVariant( parent->password() );
  return args;
}

bool APILiveJournal::APILiveJournalPrivate::readPostingFromMap(
    BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotAddFriend(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotAssignFriendToCategory(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotCreatePosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotDeleteFriend(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotExpireCookie(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotExpireAllCookies(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}


void APILiveJournal::APILiveJournalPrivate::slotFault( int number,
    const QString &errorString, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotFetchPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotFetchUserInfo(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotGenerateCookie(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotListCategories(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotListFriends(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotListFriendsOf(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotListMoods(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotListPictureKeywords(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotListRecentPostings(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void APILiveJournal::APILiveJournalPrivate::slotModifyPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

#include "livejournal_p.moc"
