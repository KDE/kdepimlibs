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

  \par Maintainer: Christian Weilbach \<christian_weilbach\@web.de\>
 */

namespace KBlog {

  class GDataPrivate;
  class BlogPostingComment;

/**
  @brief
  A class that can be used for access to GData blogs. The new blogspot.com
  accounts ( August 2007 ) exclusively support GData API which is a standard
  based on Atom API. Compared to Blogger 1.0, which is based on Xml-Rpc and
  less secure, it adds new functionality like titles and comments.

  @code
  Blog* myblog = new GData("http://myblogspot.account.com");
  myblog->setProfileId( "2039484587348593945823" ); // can be fetched via fetchProfileId()
  KBlog::BlogPosting *post = new BlogPosting();
  post->setUsername( "your_email@address.com" );
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPosting( posting );
  @endcode

  @author Christian Weilbach \<christian_weilbach\@web.de\>
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
      Sets the user's name for the blog. Username is only the E-Mail
      address of the user. This is used in createPosting and modifyPosting.
      @param fullName is a QString containing the blog username.

      @see username()
      @see createPosting( KBlog::BlogPosting* )
      @see modifiyPosting( KBlog::BlogPosting* )
    */
    virtual void setFullName( const QString &fullName );

    /**
       Returns the full name of user of the blog.
       @see setFullName()
    */
    QString fullName() const;

    /**
        Returns the profile id of the blog. This is used for rss paths internally.

        @param id This is nummeric id.

        @see setProfileId( QString& id )
    */
    QString profileId() const;

    /**
        Get the profile's id of the blog.

        @return The profile id.

        @see profileId()
    */
    virtual void setProfileId( const QString &pid );

    /**
        Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
        Get information about the user from the blog.

        @see void fetchedUserInfo( const QMap\<QString,QString\>& )
    */
    void fetchProfileId();

    /**
        List the blogs available for this authentication on the server.

        @see void listedBlogs( const QList\<QMap\<QString,QString\>\>& )
    */
    virtual void listBlogs();


    /**
        List the comments available for this posting on the server.

        @see void listedComments( KBlog::BlogPosting*, const QList\<KBlog::BlogPostingComment\>& )
    */
    virtual void listComments( KBlog::BlogPosting *posting );


    /**
        List the all comments available for this authentication on the server.

        @see void listedAllComments( const QList\<KBlog::BlogPostingComment\>& )
    */
    virtual void listAllComments();

    /**
        List recent postings on the server.

        @param number The number of postings to fetch. The order is newest first.

        @see     void listedPostings( const QList\<KBlog::BlogPosting\>& )
        @see     void fetchPosting( KBlog::BlogPosting* )
    */
    void listRecentPostings( int number );


    /**
        List recent postings on the server depending on meta information about the posting.

        @param label The lables of postings to fetch.
        @param number The number of postings to fetch. The order is newest first.
        @param upMinTime The oldest upload time of the postings to fetch.
        @param upMaxTime The newest upload time of the postings to fetch.
        @param pubMinTime The oldest publication time of the postings to fetch.
        @param pubMaxTime The newest publication time of the postings to fetch.

        @see     void listedPostings( const QList\<KBlog::BlogPosting\>& )
        @see     void fetchPosting( KBlog::BlogPosting* )
    */
    virtual void listRecentPostings( const QStringList &label=QStringList(), int number=0, 
                const KDateTime &upMinTime=KDateTime(), 
                const KDateTime &upMaxTime=KDateTime(), 
                const KDateTime &pubMinTime=KDateTime(), 
                const KDateTime &pubMaxTime=KDateTime() );


    /**
        Fetch the Posting with a specific id.
        @param posting This is the posting with its id set correctly.

        @see BlogPosting::setPostingId( const QString& )
        @see fetchedPosting( KBlog::BlogPosting *posting )
    */
    void fetchPosting( KBlog::BlogPosting *posting );

    /**
        Modify a posting on server.

        @param posting This is used to send the modified posting including the
          correct id.
    */
    void modifyPosting( KBlog::BlogPosting *posting );

    /**
        Create a new posting on server.

        @param posting This is send to the server.

        @see createdPosting( KBlog::BlogPosting *posting )
    */
    void createPosting( KBlog::BlogPosting *posting );

    /**
        Remove a posting from the server.

        @param posting This is the posting whith its id set correctly.

        @see BlogPosting::setPostingId( const QString& )
        @see removedPosting( KBlog::BlogPosting* )
    */
    void removePosting( KBlog::BlogPosting *posting );


    /**
        Create a comment on the server.

        @param posting This is the posting whith its id set correctly.
        @param comment This is the comment to create.

        @see BlogPosting::setPostingId( const QString& )
        @see createdComment( KBlog::BlogPosting*, KBlog::BlogPostingComment*  )
    */
    virtual void createComment( KBlog::BlogPosting *posting, KBlog::BlogPostingComment *comment );


    /**
        Remove a comment from the server.

        @param posting This is the posting whith its id set correctly.
        @param comment This is the comment to remove.

        @see BlogPosting::setPostingId( const QString& )
        @see removedComment( KBlog::BlogPosting*, KBlog::BlogPostingComment*  )
    */
    virtual void removeComment( KBlog::BlogPosting *posting, KBlog::BlogPostingComment *comment );

  Q_SIGNALS:

    /**
      This signal is emitted when a list of blogs has been fetched
      from the blogging server.

      @param blogsList The list of blogs.

      @see listBlogs()
    */
    void listedBlogs( const QList<QMap<QString,QString> >& blogsList );

    /**
      This signal is emitted when a list of all comments has been 
      fetched from the blogging server.

      @param commentsList The list of comments.

      @see listAllComments()
    */
    void listedAllComments( const QList<KBlog::BlogPostingComment> &commentsList );


    /**
      This signal is emitted when a list of comments has been fetched 
      from the blogging server.

      @param posting This is the corresponding posting.
      @param comments The list of comments.

      @see listComments( KBlog::BlogPosting* )
    */
    void listedComments( KBlog::BlogPosting *posting, const QList<KBlog::BlogPostingComment> &comments );


    /**
      This signal is emitted when a comment has been created
      on the blogging server.

      @param posting This is the corresponding posting.
      @param comment This is the created comment.

      @see createComment( KBlog::BlogPosting *posting, KBlog::BlogPostingComment *comment )
    */
    void createdComment( const KBlog::BlogPosting *posting, const KBlog::BlogPostingComment *comment );


    /**
      This signal is emitted when a comment has been removed
      from the blogging server.

      @param posting This is the corresponding posting.
      @param comment This is the removed comment.

      @see removeComment( KBlog::BlogPosting *posting, KBlog::BlogPostingComment *comment )
    */
    void removedComment( const KBlog::BlogPosting *posting, const KBlog::BlogPostingComment *comment );

    /**
      This signal is emitted when the profile id has been
      fetched.

      @param profileId This is the fetched id. On error it is QString()

      @see fetchProfileId()
    */
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
