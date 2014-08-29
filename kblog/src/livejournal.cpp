/*
  This file is part of the kblog library.

  Copyright (c) 2007-2008 Mike McQuaid <mike@mikemcquaid.com>
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

#include "livejournal.h"
#include "livejournal_p.h"
#include "blogpost.h"

#include <kxmlrpcclient/client.h>

#include <QDebug>
#include <KLocalizedString>
#include <KDateTime>

using namespace KBlog;

LiveJournal::LiveJournal(const QUrl &server, QObject *parent)
    : Blog(server, *new LiveJournalPrivate, parent)
{
    setUrl(server);
}

LiveJournal::~LiveJournal()
{
}

void LiveJournal::addFriend(const QString &username, int group,
                            const QColor &fgcolor, const QColor &bgcolor)
{
    // LJ.XMLRPC.editfriends
    Q_D(LiveJournal);   // Enable d-pointer access to the LiveJournalPrivate object
    unsigned int i = d->mCallCounter++; // Add one to the call counter and assign it
    d->mCallMapAddFriend[ i ] = username; // Put the post in the map at location i
    qDebug() << "LiveJournal::addFriend(): username: "
             << username; // Send a message to the console to state which method we have entered.
    QList<QVariant> args; // Create the argument list, in this case will just contain the map.
    QMap<QString, QVariant> map(d->defaultArgs());  // Create the initial map from the default arguments.
    QList<QVariant> users;
    QMap<QString, QVariant> user;
    user.insert("username", username);
    user.insert("group", group);
    user.insert("fgcolor", fgcolor);
    user.insert("bgcolor", bgcolor);
    users << user;
    map.insert("add", users);
    args << map;
    d->mXmlRpcClient->call("LJ.XMLRPC.editfriends",  // The XML-RPC procedure to call.
                           args, // A list containing all the arguments to pass to the procedure.
                           this, // The object containing the slot to use on success.
                           SLOT(slotAddFriend(QList<QVariant>,QVariant)), // The slot to call on success.
                           this, // The object containing the slot to call on failure.
                           SLOT(slotError(int,QString,QVariant)), // The slot to call on failure
                           QVariant(i));    // The ID, as we haven't created a post, the location in the map.
}

void LiveJournal::assignFriendToCategory(const QString &username, int category)
{
    Q_UNUSED(username);
    Q_UNUSED(category);
    //TODO
    // LJ.XMLRPC.editfriendgroups
}

void LiveJournal::createPost(KBlog::BlogPost *post)
{
    Q_D(LiveJournal);   // Enable d-pointer access to the LiveJournalPrivate object
    if (!post) {   // Check if post has a valid memory address (>0)
        qCritical() << "LiveJournal::createPost: post is null pointer"; // If it doesn't print an error to the console.
        return; // If it does not, exit the method
    }
    unsigned int i = d->mCallCounter++; // Add one to the call counter and assign it
    d->mCallMap[ i ] = post; // Put the post in the map at location i
    qDebug() << "LiveJournal::createPost()"; // Send a message to the console to state which method we have entered.
    QList<QVariant> args; // Create the argument list, in this case will just contain the map.
    QMap<QString, QVariant> map(d->defaultArgs());  // Create the initial map from the default arguments.
    map.insert("lineendings", "pc");   // PC line endings
    map.insert("event", post->content());   // Insert the post's content into the struct.
    map.insert("subject", post->title());   // Insert the post's subject into the struct.
    // TODO map.insert( "allowmask", post->categories() ); // We want to use the allowmask to use categories/tags
    KDateTime date = post->creationDateTime(); // Get the date of the post's creation
    int year = date.toString("%Y").toInt();   // Get the year from the date using a format string and converting string to an integer
    int month = date.toString("%m").toInt();   // Get the month from the date using a format string and converting string to an integer
    int day = date.toString("%d").toInt();   // Get the day from the date using a format string and converting string to an integer
    int hour = date.toString("%H").toInt();   // Get the hour from the date using a format string and converting string to an integer
    int minute = date.toString("%M").toInt();   // Get the minute from the date using a format string and converting string to an integer
    map.insert("year", year);   // Insert the year into the struct.
    map.insert("mon", month);   // Insert the month into the struct.
    map.insert("day", day);   // Insert the day into the struct.
    map.insert("hour", hour);   // Insert the hour into the struct.
    map.insert("min", minute);   // Insert the minute into the struct.
    args << map ; // Add the map to the arguments list.
    d->mXmlRpcClient->call("LJ.XMLRPC.postevent",  // The XML-RPC procedure to call.
                           args, // A list containing all the arguments to pass to the procedure.
                           this, // The object containing the slot to use on success.
                           SLOT(slotCreatePost(QList<QVariant>,QVariant)), // The slot to call on success.
                           this, // The object containing the slot to call on failure.
                           SLOT(slotError(int,QString,QVariant)), // The slot to call on failure
                           QVariant(i));    // The ID, as we haven't created a post, the location in the map.
}

void LiveJournal::deleteFriend(const QString &username)
{
    Q_UNUSED(username);
    //TODO
    // LJ.XMLRPC.editfriends
}

void LiveJournal::fetchPost(KBlog::BlogPost *post)
{
    Q_UNUSED(post);
    //TODO
    // LJ.XMLRPC.getevents
}

QString LiveJournal::fullName() const
{
    return d_func()->mFullName;
}

QString LiveJournal::interfaceName() const
{
    return QLatin1String("LiveJournal");
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

void LiveJournal::listRecentPosts(int number)
{
    Q_UNUSED(number);
    //TODO
    // LJ.XMLRPC.getevents with lastn and howmany
}

void LiveJournal::modifyPost(KBlog::BlogPost *post)
{
    // LJ.XMLRPC.editevent
    Q_D(LiveJournal);   // Enable d-pointer access to the LiveJournalPrivate object
    if (!post) {   // Check if post has a valid memory address (>0)
        qCritical() << "LiveJournal::modifyPost: post is null pointer"; // If it doesn't print an error to the console.
        return; // If it does not, exit the method
    }
    unsigned int i = d->mCallCounter++; // Add one to the call counter and assign it
    d->mCallMap[ i ] = post; // Put the post in the map at location i
    qDebug() << "LiveJournal::modifyPost()"; // Send a message to the console to state which method we have entered.
    QList<QVariant> args; // Create the argument list, in this case will just contain the map.
    QMap<QString, QVariant> map(d->defaultArgs());  // Create the initial map from the default arguments.
    map.insert("lineendings", "pc");   // PC line endings
    map.insert("event", post->content());   // Insert the post's content into the struct.
    map.insert("subject", post->title());   // Insert the post's subject into the struct.
    // TODO map.insert( "allowmask", post->categories() ); // We want to use the allowmask to use categories/tags
    KDateTime date = post->creationDateTime(); // Get the date of the post's creation
    int year = date.toString("%Y").toInt();   // Get the year from the date using a format string and converting string to an integer
    int month = date.toString("%m").toInt();   // Get the month from the date using a format string and converting string to an integer
    int day = date.toString("%d").toInt();   // Get the day from the date using a format string and converting string to an integer
    int hour = date.toString("%H").toInt();   // Get the hour from the date using a format string and converting string to an integer
    int minute = date.toString("%M").toInt();   // Get the minute from the date using a format string and converting string to an integer
    map.insert("year", year);   // Insert the year into the struct.
    map.insert("mon", month);   // Insert the month into the struct.
    map.insert("day", day);   // Insert the day into the struct.
    map.insert("hour", hour);   // Insert the hour into the struct.
    map.insert("min", minute);   // Insert the minute into the struct.
    args << map ; // Add the map to the arguments list.
    d->mXmlRpcClient->call("LJ.XMLRPC.editevent",  // The XML-RPC procedure to call.
                           args, // A list containing all the arguments to pass to the procedure.
                           this, // The object containing the slot to use on success.
                           SLOT(slotCreatePost(QList<QVariant>,QVariant)), // The slot to call on success.
                           this, // The object containing the slot to call on failure.
                           SLOT(slotError(int,QString,QVariant)), // The slot to call on failure
                           QVariant(i));    // The ID, as we haven't created a post, the location in the map.
}

void LiveJournal::removePost(KBlog::BlogPost *post)
{
    Q_D(LiveJournal);   // Enable d-pointer access to the LiveJournalPrivate object
    qDebug() << "LiveJournal::removePost()"; // Send a message to the console to state which method we have entered.
    QList<QVariant> args; // Create the argument list, in this case will just contain the map.
    QMap<QString, QVariant> map(d->defaultArgs());  // Create the initial map from the default arguments.
    map.insert("itemid", post->postId().toInt());   // Insert the post's unique ID into the struct.
    map.insert("event", QString());   // Insert no content into the struct to delete the post.
    map.insert("subject", post->title());   // Insert the post's subject into the struct.
    // TODO map.insert( "allowmask", post->categories() );
    KDateTime date = post->creationDateTime(); // Get the date of the post's creation
    int year = date.toString("%Y").toInt();   // Get the year from the date using a format string and converting string to an integer
    int month = date.toString("%m").toInt();   // Get the month from the date using a format string and converting string to an integer
    int day = date.toString("%d").toInt();   // Get the day from the date using a format string and converting string to an integer
    int hour = date.toString("%H").toInt();   // Get the hour from the date using a format string and converting string to an integer
    int minute = date.toString("%M").toInt();   // Get the minute from the date using a format string and converting string to an integer
    map.insert("year", year);   // Insert the year into the struct.
    map.insert("mon", month);   // Insert the month into the struct.
    map.insert("day", day);   // Insert the day into the struct.
    map.insert("hour", hour);   // Insert the hour into the struct.
    map.insert("min", minute);   // Insert the minute into the struct.
    args << QVariant(map);   // Add the map to the arguments list.
    d->mXmlRpcClient->call("LJ.XMLRPC.editevent",  // The XML-RPC procedure to call.
                           args, // A list containing all the arguments to pass to the procedure.
                           this, // The object containing the slot to use on success.
                           SLOT(slotRemovePost(QList<QVariant>,QVariant)), // The slot to call on success.
                           this, // The object containing the slot to call on failure.
                           SLOT(slotError(int,QString,QVariant))); // The slot to call on failure.
}

void LiveJournal::setUrl(const QUrl &server)
{
    Q_D(LiveJournal);
    Blog::setUrl(server);
    delete d->mXmlRpcClient;
    d->mXmlRpcClient = new KXmlRpc::Client(server);
    d->mXmlRpcClient->setUserAgent(userAgent());
}

QString LiveJournal::serverMessage() const
{
    //TODO
    return d_func()->mServerMessage;
}

QString LiveJournal::userId() const
{
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

QMap<QString, QVariant> LiveJournalPrivate::defaultArgs()
{
    Q_Q(LiveJournal);   // Get access to the q object which allows access to LiveJournal.* from LiveJournalPrivate
    QMap<QString, QVariant> args; // Create a map which is converted to a struct on the XML-RPC send.
    args.insert("username", q->username());   // Add a username key with the username as it's value.
    args.insert("password", q->password());   // Add a password key with the password as it's value.
    args.insert("ver", "1");   // Add a version key indicating we support unicode.
    return args; // return the QMap.
}

void LiveJournalPrivate::generateCookie(const GenerateCookieOptions &options)
{
    Q_UNUSED(options);
    //TODO
    // LJ.XMLRPC.sessiongenerate
}

void LiveJournalPrivate::expireCookie(const QString &cookie, bool expireAll)
{
    Q_UNUSED(cookie);
    Q_UNUSED(expireAll);
    //TODO
    // LJ.XMLRPC.sessionexpire
}

bool LiveJournalPrivate::readPostFromMap(BlogPost *post, const QMap<QString, QVariant> &postInfo)
{
    Q_UNUSED(post);
    Q_UNUSED(postInfo);
    //TODO
    return false;
}

void LiveJournalPrivate::slotAddFriend(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}

void LiveJournalPrivate::slotAssignFriendToCategory(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}

void LiveJournalPrivate::slotCreatePost(const QList<QVariant> &result, const QVariant &id)
{
    qDebug() << "LiveJournal::slotCreatePost: " << id; // Print method name and id to the console.
    Q_Q(LiveJournal);   // Get access to the q object which allows access to LiveJournal.* from LiveJournalPrivate
    KBlog::BlogPost *post = mCallMap[ id.toInt() ]; // Retrieve the post from the calling map
    mCallMap.remove(id.toInt());   // Remove the post as it is now owned by the signal catcher

    // struct containing String anum, String itemid
    qDebug() << "TOP:" << result[0].typeName();  // Print first return type to the console.
    if (result[0].type() != QVariant::Map) {   // Make sure the only return type is a struct.
        qCritical() << "Could not fetch post's ID out of the result from the server,"
                    << "not a map."; // If not a struct, print error.
        emit q->errorPost(LiveJournal::ParsingError,
                          i18n("Could not read the post ID, result not a map."), post);    // Emit an error signal if we can't get the post ID.
        return;
    }
    QString itemid = result[0].value<QMap<QString, QVariant> >().value("itemid").value<QString>();  // Get post ID from struct.
    post->setPostId(itemid);   // Set the post ID to the anum value from the return struct.
    post->setStatus(KBlog::BlogPost::Created);   // Set the post's status to indicate it has been successfully created.
    qDebug() << "emitting createdPost()"
             << "for" << itemid; // Notify emission to the console
    emit q->createdPost(post);   // Emit the created post
}

void LiveJournalPrivate::slotDeleteFriend(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}

// void LiveJournalPrivate::slotExpireCookie(
//     const QList<QVariant> &result, const QVariant &id )
// {
//   Q_UNUSED( result );
//   Q_UNUSED( id );
//   //TODO
// }

void LiveJournalPrivate::slotError(int number, const QString &errorString, const QVariant &id)
{
    Q_UNUSED(number);
    Q_UNUSED(errorString);
    qCritical() << "XML-RPC error for " << id;
}

void LiveJournalPrivate::slotFetchPost(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}

void LiveJournalPrivate::slotFetchUserInfo(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}
/*
void LiveJournalPrivate::slotGenerateCookie( const QList<QVariant> &result, const QVariant &id )
{
  Q_UNUSED( result );
  Q_UNUSED( id );
  //TODO
}*/

void LiveJournalPrivate::slotListCategories(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}

void LiveJournalPrivate::slotListFriends(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}

void LiveJournalPrivate::slotListFriendsOf(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}

void LiveJournalPrivate::slotListMoods(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}

void LiveJournalPrivate::slotListPictureKeywords(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}

void LiveJournalPrivate::slotListRecentPosts(const QList<QVariant> &result, const QVariant &id)
{
    Q_UNUSED(result);
    Q_UNUSED(id);
    //TODO
}

void LiveJournalPrivate::slotModifyPost(const QList<QVariant> &result, const QVariant &id)
{
    qDebug() << "LiveJournal::slotModifyPost: " << id; // Print method name and id to the console.
    Q_Q(LiveJournal);   // Get access to the q object which allows access to LiveJournal.* from LiveJournalPrivate
    KBlog::BlogPost *post = mCallMap[ id.toInt() ]; // Retrieve the post from the calling map
    mCallMap.remove(id.toInt());   // Remove the post as it is now owned by the signal catcher

    // struct containing String anum, String itemid
    qDebug() << "TOP:" << result[0].typeName();  // Print first return type to the console.
    if (result[0].type() != QVariant::Map) {   // Make sure the only return type is a struct.
        qCritical() << "Could not fetch post's ID out of the result from the server,"
                    << " not a map."; // If not a struct, print error.
        emit q->errorPost(LiveJournal::ParsingError,
                          i18n("Could not read the post ID, result not a map."), post);    // Emit an error signal if we can't get the post ID.
        return;
    }
    QString itemid = result[0].value<QMap<QString, QVariant> >().value("itemid").value<QString>();  // Get post ID from struct.
    post->setPostId(itemid);   // Set the post ID to the anum value from the return struct.
    post->setStatus(KBlog::BlogPost::Created);   // Set the post's status to indicate it has been successfully created.
    qDebug() << "emitting createdPost()"
             << "for" << itemid; // Notify emission to the console
    emit q->createdPost(post);   // Emit the created post
}

void LiveJournalPrivate::slotRemovePost(const QList<QVariant> &result,
                                        const QVariant &id)
{
    qDebug() << "LiveJournal::slotCreatePost: " << id; // Print method name and id to the console.
    Q_Q(LiveJournal);   // Get access to the q object which allows access to LiveJournal.* from LiveJournalPrivate
    KBlog::BlogPost *post = mCallMap[ id.toInt() ]; // Retrieve the post from the calling map
    mCallMap.remove(id.toInt());   // Remove the post as it is now owned by the signal catcher

    // struct containing String anum, String itemid
    qDebug() << "TOP:" << result[0].typeName();  // Print first return type to the console.
    if (result[0].type() != QVariant::Map) {   // Make sure the only return type is a struct.
        qCritical() << "Could not fetch post's ID out of the result from the server,"
                    << "not a map."; // If not a struct, print error.
        emit q->errorPost(LiveJournal::ParsingError,
                          i18n("Could not read the post ID, result not a map."), post);    // Emit an error signal if we can't get the post ID.
        return;
    }
    QString itemid = result[0].value<QMap<QString, QVariant> >().value("itemid").value<QString>();
    if (itemid == post->postId()) {   // Check the post ID matches the anum value from the return struct.
        post->setStatus(KBlog::BlogPost::Removed);   // Set the post's status to indicate it has been successfully removed.
        qDebug() << "emitting createdPost()"
                 << "for" << itemid; // Notify emission to the console
        emit q->removedPost(post);   // Emit the removed post
        return;
    }
    qCritical() << "The returned post ID did not match the sent one."; // If not matching, print error.
    emit q->errorPost(LiveJournal::ParsingError,
                      i18n("The returned post ID did not match the sent one: "), post);    // Emit an error signal if the post IDs don't match.
}

#include "moc_livejournal.cpp"
