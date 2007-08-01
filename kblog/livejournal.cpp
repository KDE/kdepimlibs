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

#include "livejournal.h"
#include "livejournal_p.h"
#include "blogposting.h"

#include <kxmlrpcclient/client.h>

#include <KDebug>
#include <KLocale>

using namespace KBlog;

LiveJournal::LiveJournal( const KUrl &server, QObject *parent )
  : Blog( server, *new LiveJournalPrivate, parent )
{
  setUrl( server );
}

LiveJournal::~LiveJournal()
{
}

void LiveJournal::addFriend( const QString &username, int group,
                                const QColor &fgcolor, const QColor &bgcolor )
{
  Q_UNUSED( username );
  Q_UNUSED( group );
  Q_UNUSED( fgcolor );
  Q_UNUSED( bgcolor );
  //TODO
  // LJ.XMLRPC.editfriends
}

void LiveJournal::assignFriendToCategory ( const QString &username,
                                              int category )
{
  Q_UNUSED( username );
  Q_UNUSED( category );
  //TODO
  // LJ.XMLRPC.editfriendgroups
}

void LiveJournal::createPosting( KBlog::BlogPosting *posting )
{
  Q_UNUSED( posting );
  //TODO
  // LJ.XMLRPC.postevent
}

void LiveJournal::deleteFriend( const QString &username )
{
  Q_UNUSED( username );
  //TODO
  // LJ.XMLRPC.editfriends
}

void LiveJournal::expireCookie( const QString &cookie )
{
  Q_UNUSED( cookie );
  //TODO
  // LJ.XMLRPC.sessionexpire
}

void LiveJournal::expireAllCookies()
{
  //TODO
  // LJ.XMLRPC.sessionexpire
}

void LiveJournal::fetchPosting( KBlog::BlogPosting *posting )
{
  Q_UNUSED( posting );
  //TODO
  // LJ.XMLRPC.getevents
}

QString LiveJournal::fullName() const
{
  return d_func()->mFullName;
}

void LiveJournal::generateCookie( const GenerateCookieOptions& options )
{
  Q_UNUSED( options );
  //TODO
  // LJ.XMLRPC.sessiongenerate
}

QString LiveJournal::interfaceName() const
{
  return QLatin1String( "LiveJournal" );
}

void LiveJournal::fetchUserInfo()
{
  //TODO
}

void LiveJournal::listCategories()
{
  //TODO
  // LJ.XMLRPC.getfriendgroups
}

void LiveJournal::listFriends()
{
  //TODO
  // LJ.XMLRPC.getfriends and their groups
}

void LiveJournal::listFriendsOf()
{
  //TODO
  // LJ.XMLRPC.friendof
}

void LiveJournal::listMoods()
{
  //TODO
  // LJ.XMLRPC.login
}

void LiveJournal::listPictureKeywords()
{
  //TODO
  // LJ.XMLRPC.login
}

void LiveJournal::listRecentPostings( int number )
{
  Q_UNUSED( number );
  //TODO
  // LJ.XMLRPC.getevents with lastn and howmany
}

void LiveJournal::modifyPosting( KBlog::BlogPosting *posting )
{
  Q_UNUSED( posting );
  //TODO
  // LJ.XMLRPC.editevent
}

void LiveJournal::removePosting( KBlog::BlogPosting *posting )
{
  Q_UNUSED( posting );
  //TODO
  // LJ.XMLRPC.editevent
}

void LiveJournal::setUrl( const KUrl &server )
{
  Blog::setUrl( server );
}

QString LiveJournal::serverMessage() const {
  //TODO
  return d_func()->mServerMessage;
}

QString LiveJournal::userId() const {
  //TODO
  return d_func()->mUserId;
}

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

#include "livejournal.moc"
