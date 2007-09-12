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

#include <QtGui/QColor>

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
  KBlog::BlogPost *post = new BlogPost();
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
    virtual ~LiveJournal();

    virtual void addFriend( const QString &username, int group,
                    const QColor &fgcolor = QColor( "#000000" ),
                    const QColor &bgcolor = QColor( "#FFFFFF" ) );

    virtual void assignFriendToCategory( const QString &username, int category );

    /**
      Create a new posting on server.

      @param posting is send to the server.
    */
    void createPosting( KBlog::BlogPost *posting );

    virtual void deleteFriend( const QString &username );

    virtual void expireCookie( const QString &cookie );

    virtual void expireAllCookies();

    /**
      Fetch the Posting with postingId.
      @param postingId is the id of the posting on the server.

      @see  void fetchedPosting( KBlog::BlogPost &posting )
    */
    void fetchPosting( KBlog::BlogPost *posting );

    virtual void fetchUserInfo();

    QString fullName() const;

    enum GenerateCookieOption {
      LongExpiriation = 0x01,
      FixedIP = 0x02
    };
    Q_DECLARE_FLAGS(GenerateCookieOptions,
                                        GenerateCookieOption)

    virtual void generateCookie( const GenerateCookieOptions& options );

    /**
      Returns the  of the inherited object.
    */
    QString interfaceName() const;

    void listCategories();

    virtual void listFriends();

    virtual void listFriendsOf();

    virtual void listMoods();

    virtual void listPictureKeywords();

    /**
      List recent postings on the server.
      @see     void listedPosting( KBlog::BlogPost &posting )

      @see     void listRecentPostingsFinished()
    */
    void listRecentPostings( int number );

    /**
      Modify a posting on server.

      @param posting is used to send the modified posting including the
      correct postingId from it to the server.
    */
    void modifyPosting( KBlog::BlogPost *posting );

    void removePosting( KBlog::BlogPost *posting );
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
    Q_PRIVATE_SLOT(d_func(), void slotAddFriend(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotAssignFriendToCategory(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotCreatePosting(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotDeleteFriend(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotExpireCookie(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotExpireAllCookies(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotFetchPosting(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotFetchUserInfo(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotGenerateCookie(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotListCategories(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotListFriends(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotListFriendsOf(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotListMoods(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotListPictureKeywords(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotListRecentPostings(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotModifyPosting(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotRemovePosting(
                    const QList<QVariant>&, const QVariant& ))
    Q_PRIVATE_SLOT(d_func(), void slotError( int ,
                    const QString&, const QVariant& ))
};
Q_DECLARE_OPERATORS_FOR_FLAGS(LiveJournal::GenerateCookieOptions)

} //namespace KBlog
#endif
