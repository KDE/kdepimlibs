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

#include "livejournal_p.h"
#include "blogposting.h"

#include <kxmlrpcclient/client.h>

#include <KDebug>
#include <KLocale>

using namespace KBlog;

LiveJournalPrivate::LiveJournalPrivate()
{
  mXmlRpcClient = 0;
}

LiveJournalPrivate::~LiveJournalPrivate()
{
  delete mXmlRpcClient;
}

QList<QVariant> LiveJournalPrivate::defaultArgs(
    const QString &id )
{
  Q_Q(LiveJournal);
  QList<QVariant> args;

  if ( id.toInt() ) {
    args << QVariant( id.toInt() );
  }
  if ( !id.toInt() && !id.isNull() ){
    args << QVariant( id );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

bool LiveJournalPrivate::readPostingFromMap(
    BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
  //TODO
  return false;
}

void LiveJournalPrivate::slotAddFriend(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotAssignFriendToCategory(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotCreatePosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotDeleteFriend(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotExpireCookie(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotExpireAllCookies(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}


void LiveJournalPrivate::slotError( int number,
    const QString &errorString, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotFetchPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotFetchUserInfo(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotGenerateCookie(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotListCategories(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotListFriends(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotListFriendsOf(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotListMoods(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotListPictureKeywords(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotListRecentPostings(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

void LiveJournalPrivate::slotModifyPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  //TODO
}

#include "livejournal_p.moc"
