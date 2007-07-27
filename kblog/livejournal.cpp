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

#include <KDebug>

using namespace KBlog;

LiveJournal::LiveJournal( const KUrl &server, QObject *parent )
  : Blog( server, parent ), d( new LiveJournalPrivate )
{
  d->parent = this;
  setUrl( server );
}

LiveJournal::~LiveJournal()
{
  delete d;
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
  return d->mFullName;
}

void LiveJournal::generateCookie( const GenerateCookieOptions& options )
{
  Q_UNUSED( options );
  //TODO
  // LJ.XMLRPC.sessiongenerate
}

QString LiveJournal::interfaceName() const
{
  return QLatin1String( "Movable Type " );
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
  return d->mServerMessage;
}

QString LiveJournal::userId() const {
  //TODO
  return d->mUserId;
}

#include "livejournal.moc"
