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
#include <kdebug.h>
#include <klocale.h>
#include <kdatetime.h>

using namespace KBlog;

APILiveJournal::APILiveJournal( const KUrl &server, QObject *parent )
  : APIBlog( server, parent ), d( new APILiveJournalPrivate )
{
  d->parent = this;
  setUrl( server );
}

APILiveJournal::~APILiveJournal()
{
  delete d;
}

void APILiveJournal::addFriend( const QString &username, int group,
                                const QColor &fgcolor, const QColor &bgcolor )
{
  Q_UNUSED( username );
  Q_UNUSED( group );
  Q_UNUSED( fgcolor );
  Q_UNUSED( bgcolor );
  //TODO
  // LJ.XMLRPC.editfriends
}

void APILiveJournal::assignFriendToCategory ( const QString &username,
                                              int category )
{
  Q_UNUSED( username );
  Q_UNUSED( category );
  //TODO
  // LJ.XMLRPC.editfriendgroups
}

void APILiveJournal::createPosting( KBlog::BlogPosting *posting )
{
  Q_UNUSED( posting );
  //TODO
  // LJ.XMLRPC.postevent
}

void APILiveJournal::deleteFriend( const QString &username )
{
  Q_UNUSED( username );
  //TODO
  // LJ.XMLRPC.editfriends
}

void APILiveJournal::expireCookie( const QString &cookie )
{
  Q_UNUSED( cookie );
  //TODO
  // LJ.XMLRPC.sessionexpire
}

void APILiveJournal::expireAllCookies()
{
  //TODO
  // LJ.XMLRPC.sessionexpire
}

void APILiveJournal::fetchPosting( KBlog::BlogPosting *posting )
{
  Q_UNUSED( posting );
  //TODO
  // LJ.XMLRPC.getevents
}

QString APILiveJournal::fullName() const
{
  return d->mFullName;
}

void APILiveJournal::generateCookie( bool longExpiration, bool fixedIP )
{
  Q_UNUSED( longExpiration );
  Q_UNUSED( fixedIP );
  //TODO
  // LJ.XMLRPC.sessiongenerate
}

QString APILiveJournal::interfaceName() const
{
  return QLatin1String( "Movable Type API" );
}

void APILiveJournal::listCategories()
{
  //TODO
  // LJ.XMLRPC.getfriendgroups
}

void APILiveJournal::listFriends()
{
  //TODO
  // LJ.XMLRPC.getfriends and their groups
}

void APILiveJournal::listFriendsOf()
{
  //TODO
  // LJ.XMLRPC.friendof
}

void APILiveJournal::listMoods()
{
  //TODO
  // LJ.XMLRPC.login
}

void APILiveJournal::listPictureKeywords()
{
  //TODO
  // LJ.XMLRPC.login
}

void APILiveJournal::listRecentPostings( int number )
{
  Q_UNUSED( number );
  //TODO
  // LJ.XMLRPC.getevents with lastn and howmany
}

void APILiveJournal::modifyPosting( KBlog::BlogPosting *posting )
{
  Q_UNUSED( posting );
  //TODO
  // LJ.XMLRPC.editevent
}

void APILiveJournal::removePosting( KBlog::BlogPosting *posting )
{
  Q_UNUSED( posting );
  //TODO
  // LJ.XMLRPC.editevent
}

void APILiveJournal::setUrl( const KUrl &server )
{
  APIBlog::setUrl( server );
}

QString APILiveJournal::serverMessage() const {
  //TODO
  return d->mServerMessage;
}

QString APILiveJournal::userId() const {
  //TODO
  return d->mUserId;
}

#include "livejournal.moc"
