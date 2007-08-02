/*
  This file is part of the kblog library.

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

#ifndef KBLOG_GDATA_H
#define KBLOG_GDATA_H

#include <kblog/blog.h>
#include <kdatetime.h>

#include <QtCore/QStringList>

class KUrl;

/**
  @file

  This file is part of the  for accessing Blog Servers
  and defines the GData class.

  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>

  \par Maintainer: Christian Weilbach \<christian\@whiletaker.homeip.net\>
 */

namespace KBlog {

  class GDataPrivate;
  class BlogPostingComment;

/**
  @brief
  A class that can be used for access to GData  blogs. Almost every blog
  server supports GData  . Compared to Blogger1  1.0 it is a superset of
  functions added to the its definition. GData  is much more functional, but
  has some drawbacks, e.g. security when compared to Blogger1  2.0 which is
  based on GData  and quite new.

  @code
  Blog* myblog = new GData("http://example.com/xmlrpc/gateway.php");
  KBlog::BlogPosting *post = new BlogPosting();
  post->setUserId( "some_user_id" );
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPosting( posting );
  @endcode

  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
 */
class KBLOG_EXPORT GData : public Blog
{
  Q_OBJECT
  public:
    /**
         Create an object for GData 

         @param server is the url for the xmlrpc gateway.
    */
    explicit GData( const KUrl &server, QObject *parent = 0 );
    ~GData();

    /**
      Sets the user's name for the blog.
      @param fullName is a QString containing the blog username.

      @see username()
    */
    virtual void setFullName( const QString &fullName );

    /**
       Returns the user's name of the blog.
       @see setUsername()
    */
    QString fullName() const;

    /**
        Set the ProfileId of the blog. This is used for authentication.

        @param email is the mail address of the user

        @see setProfileId( QString& id )
    */
    QString profileId() const;

    /**
        Get the profile's id of the blog.

        @return email

        @see profileId()
    */
    virtual void setProfileId( const QString &pid );

    /**
        Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
        Get information about the user from the blog. Note: This is not
        supported on the server side.
        @see void fetchedUserInfo( const QString &nickname,
                const QString &userid, const QString &email )
    */
    void fetchProfileId();

    /**
        List the blogs available for this authentication on the server.
        Note: This is not supported on the server side.
        @see void blogInfoRetrieved( const QString &id, const QString &name )
    */
    virtual void listBlogs();

    virtual void listComments( KBlog::BlogPosting *posting );

    virtual void listAllComments();

    /**
        List recent postings on the server..
        @see     void listedPosting( KBlog::BlogPosting &posting )
        @see     void fetchedPosting( KBlog::BlogPosting &posting )
        @see     void listRecentPostingsFinished()
    */
    void listRecentPostings( const int number );

    enum listRecentPostingsOption {
      updated = 0x01,
      published = 0x02
    };
    Q_DECLARE_FLAGS(listRecentPostingsOptions,
                                        listRecentPostingsOption)
    virtual void listRecentPostings( const QStringList &label=QStringList(), const int number=0, 
                const KDateTime &minTime=KDateTime(), 
                const KDateTime &maxTime=KDateTime(), 
                const listRecentPostingsOptions &opts = updated );


    /**
        Fetch the Posting with postingId.
        @param postingId is the id of the posting on the server.

        @see  void fetchedPosting( KBlog::BlogPosting &posting )
    */
    void fetchPosting( KBlog::BlogPosting *posting );

    /**
        Modify a posting on server.

        @param posting is used to send the modified posting including the
          correct postingId from it to the server.
    */
    void modifyPosting( KBlog::BlogPosting *posting );

    /**
        Create a new posting on server.

        @param posting is send to the server.
    */
    void createPosting( KBlog::BlogPosting *posting );

    /**
        Remove a posting from the server.

        @param postingId is the id of the posting to remove.

        @see void removePosting( KBlog::BlogPosting *posting )
    */
    void removePosting( KBlog::BlogPosting *posting );

    virtual void createComment( KBlog::BlogPosting *posting, KBlog::BlogPostingComment *comment );

    virtual void removeComment( KBlog::BlogPosting *posting, KBlog::BlogPostingComment *comment );

  Q_SIGNALS:

    /**
      This signal is emitted when a listBlogs() job fetches the blog
      information from the blogging server.

      @see listBlogs()
    */
    void listedBlogs( const QMap<QString,QMap<QString,QString> >& blogsInfo );

    void listedAllComments( const QList<KBlog::BlogPostingComment> &comments );

    void listedComments( const QList<KBlog::BlogPostingComment> &comments, KBlog::BlogPosting *posting );

    void createdComment( const KBlog::BlogPosting *posting, const KBlog::BlogPostingComment *comment );

    void removedComment( const KBlog::BlogPosting *posting, const KBlog::BlogPostingComment *comment );

    void fetchedProfileId( const QString &profileId );

  protected:
    GData( const KUrl &server, GDataPrivate &dd, QObject *parent = 0 );
  private:
    Q_DECLARE_PRIVATE(GData)
    Q_PRIVATE_SLOT(d_func(), void slotFetchProfileId(KJob*))
    Q_PRIVATE_SLOT(d_func(), void slotFetchProfileIdData(KIO::Job*,const QByteArray&))
    Q_PRIVATE_SLOT(d_func(), void slotListBlogs(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode))
    Q_PRIVATE_SLOT(d_func(), void slotListComments(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode))
    Q_PRIVATE_SLOT(d_func(), void slotListAllComments(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode))
    Q_PRIVATE_SLOT(d_func(), void slotListRecentPostings(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode))
    Q_PRIVATE_SLOT(d_func(), void slotFetchPosting(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode))
    Q_PRIVATE_SLOT(d_func(), void slotCreatePosting(KJob*))
    Q_PRIVATE_SLOT(d_func(), void slotCreatePostingData(KIO::Job *,const QByteArray&))
    Q_PRIVATE_SLOT(d_func(), void slotModifyPosting(KJob*))
    Q_PRIVATE_SLOT(d_func(), void slotModifyPostingData(KIO::Job *,const QByteArray&))
    Q_PRIVATE_SLOT(d_func(), void slotRemovePosting(KJob*))
    Q_PRIVATE_SLOT(d_func(), void slotRemovePostingData(KIO::Job *,const QByteArray&))
    Q_PRIVATE_SLOT(d_func(), void slotCreateComment(KJob*))
    Q_PRIVATE_SLOT(d_func(), void slotCreateCommentData(KIO::Job *,const QByteArray&))
    Q_PRIVATE_SLOT(d_func(), void slotRemoveComment(KJob*))
    Q_PRIVATE_SLOT(d_func(), void slotRemoveCommentData(KIO::Job *,const QByteArray&))
};

} //namespace KBlog
#endif
