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

#ifndef KBLOG_LIVEJOURNAL_H
#define KBLOG_LIVEJOURNAL_H

#include <kblog/blog.h>

#include <QColor>

class KHttpCookie;
class KUrl;

/**
  @file
  This file is part of the  for accessing Blog Servers
  and defines the LiveJournal class.

  @author Mike Arthur \<mike\@mikearthur.co.uk\>
*/
namespace KBlog {

    class LiveJournalPrivate;

/**
  @brief
  A class that can be used for access to Movable Type  blogs. Almost every
  blog server supports Movable Type . Blah blah
  @code
  Blog* myblog = new LiveJournal("http://example.com/xmlrpc/gateway.php");
  KBlog::BlogPosting *post = new BlogPosting();
  post->setUserId( "some_user_id" );
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPosting( posting );
  @endcode

  @author Mike Arthur \<mike\@mikearthur.co.uk\>
*/
class KBLOG_EXPORT LiveJournal : public Blog
{
  Q_OBJECT
  public:
    /**
      Create an object for Movable Type 

      @param server is the url for the xmlrpc gateway.
      @param parent is the parent object.
    */
    explicit LiveJournal( const KUrl &server, QObject *parent = 0 );

    /**
      Destroy the object.
    */
    ~LiveJournal();

    void addFriend( const QString &username, int group,
                    const QColor &fgcolor = QColor( "#000000" ),
                    const QColor &bgcolor = QColor( "#FFFFFF" ) );

    void assignFriendToCategory( const QString &username, int category );

    /**
      Create a new posting on server.

      @param posting is send to the server.
    */
    void createPosting( KBlog::BlogPosting *posting );

    void deleteFriend( const QString &username );

    void expireCookie( const QString &cookie );

    void expireAllCookies();

    /**
      Fetch the Posting with postingId.
      @param postingId is the id of the posting on the server.

      @see  void fetchedPosting( KBlog::BlogPosting &posting )
    */
    void fetchPosting( KBlog::BlogPosting *posting );

    void fetchUserInfo();

    QString fullName() const;

    enum GenerateCookieOption {
      LongExpiriation = 0x01,
      FixedIP = 0x02
    };
    Q_DECLARE_FLAGS(GenerateCookieOptions,
                                        GenerateCookieOption)

    void generateCookie( const GenerateCookieOptions& options );

    /**
      Returns the  of the inherited object.
    */
    QString interfaceName() const;

    void listCategories();

    void listFriends();

    void listFriendsOf();

    void listMoods();

    void listPictureKeywords();

    /**
      List recent postings on the server.
      @see     void listedPosting( KBlog::BlogPosting &posting )

      @see     void listRecentPostingsFinished()
    */
    void listRecentPostings( int number );

    /**
      Modify a posting on server.

      @param posting is used to send the modified posting including the
      correct postingId from it to the server.
    */
    void modifyPosting( KBlog::BlogPosting *posting );

    void removePosting( KBlog::BlogPosting *posting );
    /**
      Set the Url of the server.

      @param server is the server url.
    */
    void setUrl( const KUrl &server );

    QString serverMessage() const;

    QString userId() const;

  Q_SIGNALS:
    void addedFriend();
    void assignedFriendToCategory();
    void deletedFriend();
    void expiredCookie();
    void expiredAllCookies();
    void generatedCookie( const QString &cookie );
    void listedCategories( const QMap<QString, QString> &categories );
    void listedFriends( const QMap<QString, QMap<QString, QString> > &friends );
    void listedFriendsOf( const QMap<QString,
                          QMap<QString, QString> > &friendsOf );
    void listedMoods( const QMap<int, QString> &moods );
    void listedPictureKeywords( const QMap<QString, KUrl> &pictureKeywords );
    void fetchedUserInfo();

  protected:
    LiveJournal( const KUrl &server, LiveJournalPrivate &dd, QObject *parent = 0 );
  private:
    Q_DECLARE_PRIVATE(LiveJournal)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(LiveJournal::GenerateCookieOptions)

} //namespace KBlog
#endif
