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
#include "blogpost.h"

#include <kxmlrpcclient/client.h>

#include <KDebug>
#include <KLocale>
#include <KDateTime>

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

void LiveJournal::createPosting( KBlog::BlogPost *posting )
{
  Q_D(LiveJournal); // Enable d-pointer access to the LiveJournalPrivate object
  if ( !posting ) { // Check if posting has a valid memory address (>0)
    kError(5323) << "Blogger1::createPosting: posting is null pointer"; // If it doesn't print an error to the console.
    return; // If it doesnt, exit the method
  }
  unsigned int i = d->mCallCounter++; // Add one to the call counter and assign it
  d->mCallMap[ i ] = posting; // Put the posting in the map at location i
  kDebug(5323) << "LiveJournal::createPosting()"; // Send a message to the console to state which method we have entered.
  QList<QVariant> args; // Create the argument list, in this case will just contain the map.
  QMap<QString,QVariant> map( d->defaultArgs() ); // Create the initial map from the default arguments.
  map.insert( "event", posting->content() ); // Insert the posting's content into the struct.
  map.insert( "subject", posting->title() ); // Insert the posting's subject into the struct.
  // TODO map.insert( "allowmask", posting->categories() ); // We want to use the allowmask to use categories/tags
  KDateTime date = posting->creationDateTime(); // Get the date of the posting's creation
  int year = date.toString( "%Y" ).toInt(); // Get the year from the date using a format string and converting string to an integer
  int month = date.toString( "%m" ).toInt(); // Get the month from the date using a format string and converting string to an integer
  int day = date.toString( "%d" ).toInt(); // Get the day from the date using a format string and converting string to an integer
  int hour = date.toString( "%H" ).toInt(); // Get the hour from the date using a format string and converting string to an integer
  int minute = date.toString( "%M" ).toInt(); // Get the minute from the date using a format string and converting string to an integer
  map.insert( "year", year ); // Insert the year into the struct.
  map.insert( "mon", month ); // Insert the month into the struct.
  map.insert( "day", day ); // Insert the day into the struct.
  map.insert( "hour", hour ); // Insert the hour into the struct.
  map.insert( "min", minute ); // Insert the minute into the struct.
  args << map ; // Add the map to the arguments list.
  d->mXmlRpcClient->call("LJ.XMLRPC.postevent", // The XML-RPC procedure to call.
                         args, // A list containing all the arguments to pass to the procedure.
                         this, // The object containing the slot to use on success.
                         SLOT( slotCreatePosting( const QList<QVariant>&, const QVariant& ) ), // The slot to call on success.
                         this, // The object containing the slot to call on failure.
                         SLOT( slotError( int, const QString&, const QVariant& ) ), // The slot to call on failure
                         QVariant( i ) ); // The ID, as we haven't created a post, the location in the map.
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

void LiveJournal::fetchPosting( KBlog::BlogPost *posting )
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

void LiveJournal::modifyPosting( KBlog::BlogPost *posting )
{
  Q_UNUSED( posting );
  //TODO
  // LJ.XMLRPC.editevent
}

void LiveJournal::removePosting( KBlog::BlogPost *posting )
{
  Q_D(LiveJournal); // Enable d-pointer access to the LiveJournalPrivate object
  kDebug(5323) << "LiveJournal::removePosting()"; // Send a message to the console to state which method we have entered.
  QList<QVariant> args; // Create the argument list, in this case will just contain the map.
  QMap<QString,QVariant> map( d->defaultArgs() ); // Create the initial map from the default arguments.
  map.insert( "itemid", posting->postingId().toInt() ); // Insert the posting's unique ID into the struct.
  map.insert( "event", QString() ); // Insert no content into the struct to delete the post.
  map.insert( "subject", posting->title() ); // Insert the posting's subject into the struct.
  // TODO map.insert( "allowmask", posting->categories() );
  KDateTime date = posting->creationDateTime(); // Get the date of the posting's creation
  int year = date.toString( "%Y" ).toInt(); // Get the year from the date using a format string and converting string to an integer
  int month = date.toString( "%m" ).toInt(); // Get the month from the date using a format string and converting string to an integer
  int day = date.toString( "%d").toInt(); // Get the day from the date using a format string and converting string to an integer
  int hour = date.toString( "%H" ).toInt(); // Get the hour from the date using a format string and converting string to an integer
  int minute = date.toString( "%M" ).toInt(); // Get the minute from the date using a format string and converting string to an integer
  map.insert( "year", year ); // Insert the year into the struct.
  map.insert( "mon", month ); // Insert the month into the struct.
  map.insert( "day", day ); // Insert the day into the struct.
  map.insert( "hour", hour ); // Insert the hour into the struct.
  map.insert( "min", minute ); // Insert the minute into the struct.
  args << QVariant( map ); // Add the map to the arguments list.
  d->mXmlRpcClient->call("LJ.XMLRPC.editevent", // The XML-RPC procedure to call.
                         args, // A list containing all the arguments to pass to the procedure.
                         this, // The object containing the slot to use on success.
                         SLOT( slotRemovePosting( const QList<QVariant>&, const QVariant& ) ), // The slot to call on success.
                         this, // The object containing the slot to call on failure.
                         SLOT( slotError( int, const QString&, const QVariant& ) ) ); // The slot to call on failure.
}

void LiveJournal::setUrl( const KUrl &server )
{
  Q_D(LiveJournal);
  Blog::setUrl( server );
  delete d->mXmlRpcClient;
  d->mXmlRpcClient = new KXmlRpc::Client( server );
  d->mXmlRpcClient->setUserAgent( userAgent() );
}

QString LiveJournal::serverMessage() const {
  //TODO
  return d_func()->mServerMessage;
}

QString LiveJournal::userId() const {
  //TODO
  return d_func()->mUserId;
}

LiveJournalPrivate::LiveJournalPrivate() : mXmlRpcClient(0)
{
  mCallCounter = 1;
}

LiveJournalPrivate::~LiveJournalPrivate()
{
  delete mXmlRpcClient;
}

QMap<QString,QVariant> LiveJournalPrivate::defaultArgs()
{
  Q_Q(LiveJournal); // Get access to the q object which allows access to LiveJournal.* from LiveJournalPrivate
  QMap<QString,QVariant> args; // Create a map which is converted to a struct on the XML-RPC send.
  args.insert( "username", q->username() ); // Add a username key with the username as it's value.
  args.insert( "password", q->password() ); // Add a password key with the password as it's value.
  args.insert( "ver", "1" ); // Add a version key indicating we support unicode.
  return args; // return the QMap.
}

bool LiveJournalPrivate::readPostingFromMap(
    BlogPost *post, const QMap<QString, QVariant> &postInfo )
{
  Q_UNUSED( post );
  Q_UNUSED( postInfo );
  //TODO
  return false;
}

void LiveJournalPrivate::slotAddFriend(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotAssignFriendToCategory(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotCreatePosting( const QList<QVariant> &result,
                                            const QVariant &id )
{
  kDebug(5323) << "LiveJournal::slotCreatePosting: " << id; // Print method name and id to the console.
  Q_Q(LiveJournal); // Get access to the q object which allows access to LiveJournal.* from LiveJournalPrivate
  KBlog::BlogPost* posting = mCallMap[ id.toInt() ]; // Retrieve the posting from the calling map
  mCallMap.remove( id.toInt() ); // Remove the posting as it is now owned by the signal catcher

  // struct containing String anum, String itemid
  kDebug (5323) << "TOP:" << result[0].typeName(); // Print first return type to the console.
  if ( result[0].type() != QVariant::Map ) { // Make sure the only return type is a struct.
    kError(5323) << "Could not fetch posting's ID out of the result from the server,"
        << "not a map."; // If not a struct, print error.
    emit q->errorPosting( LiveJournal::ParsingError,
                   i18n( "Could not read the posting ID, result not a map." ), posting ); // Emit an error signal if we can't get the posting ID.
  } else {
    QString itemid = result[0].value<QMap<QString,QVariant> >().value( "itemid" ).value<QString>(); // Get post ID from struct.
    posting->setPostingId( itemid ); // Set the posting ID to the anum value from the return struct.
    posting->setStatus( KBlog::BlogPost::Created ); // Set the posting's status to indicate it has been successfully created.
    emit q->createdPosting( posting ); // Emit the created posting
    kDebug(5323) << "emitting createdPosting()" <<
        "for" << itemid; // Notify emission to the console
  }
}

void LiveJournalPrivate::slotDeleteFriend(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotExpireCookie(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotExpireAllCookies(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}


void LiveJournalPrivate::slotError( int number,
    const QString &errorString, const QVariant &id )
{
  Q_UNUSED( number );
  Q_UNUSED( errorString );
  kError(5323) << "XML-RPC error for " << id;
}

void LiveJournalPrivate::slotFetchPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotFetchUserInfo(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotGenerateCookie(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotListCategories(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotListFriends(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotListFriendsOf(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotListMoods(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotListPictureKeywords(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotListRecentPostings(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotModifyPosting(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}

void LiveJournalPrivate::slotRemovePosting( const QList<QVariant> &result,
                                            const QVariant &id )
{
  kDebug(5323) << "LiveJournal::slotCreatePosting: " << id; // Print method name and id to the console.
  Q_Q(LiveJournal); // Get access to the q object which allows access to LiveJournal.* from LiveJournalPrivate
  KBlog::BlogPost* posting = mCallMap[ id.toInt() ]; // Retrieve the posting from the calling map
  mCallMap.remove( id.toInt() ); // Remove the posting as it is now owned by the signal catcher

  // struct containing String anum, String itemid
  kDebug (5323) << "TOP:" << result[0].typeName(); // Print first return type to the console.
  if ( result[0].type() != QVariant::Map ) { // Make sure the only return type is a struct.
    kError(5323) << "Could not fetch posting's ID out of the result from the server,"
        << "not a map."; // If not a struct, print error.
    emit q->errorPosting( LiveJournal::ParsingError,
                   i18n( "Could not read the posting ID, result not a map." ), posting ); // Emit an error signal if we can't get the posting ID.
  } else {
    QString itemid = result[0].value<QMap<QString,QVariant> >().value( "itemid" ).value<QString>();
    if ( itemid == posting->postingId() ) { // Check the posting ID matches the anum value from the return struct.
      posting->setStatus( KBlog::BlogPost::Removed ); // Set the posting's status to indicate it has been successfully removed.
      emit q->removedPosting( posting ); // Emit the removed posting
      kDebug(5323) << "emitting createdPosting()" <<
          "for" << itemid; // Notify emission to the console
    }
    else {
      kError(5323) << "The returned posting ID did not match the sent one."; // If not matching, print error.
      emit q->errorPosting( LiveJournal::ParsingError,
                     i18n( "The returned posting ID did not match the sent one: " ), posting ); // Emit an error signal if the posting IDs don't match.
    }
  }
}

#include "livejournal.moc"
