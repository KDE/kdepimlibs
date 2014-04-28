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

#include <blog.h>
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
  class BlogComment;

/**
  @brief
  A class that can be used for access to GData blogs. The new blogspot.com
  accounts ( August 2007 ) exclusively support GData API which is a standard
  based on Atom API. Compared to Blogger 1.0, which is based on Xml-Rpc and
  less secure, it adds new functionality like titles and comments.

  @code
  Blog* myblog = new GData("http://myblogspot.account.com");
  myblog->setProfileId( "2039484587348593945823" ); // can be fetched via fetchProfileId()
  myblog->setBlogId( "1" ); // can be caught by listBlogs()
  myblog->setUsername( "your_email@address.com" );
  myblog->setPassword( "yOuRFuNnYPasSword" );
  KBlog::BlogPost *post = new BlogPost();
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPost( post );
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
      @param server The server url for the xmlrpc gateway.
      @param parent The parent object, inherited from QObject.
    */
    explicit GData( const KUrl &server, QObject *parent = 0 );

    /**
      Destructor.
    */
    ~GData();

    /**
      Sets the user's name for the blog. Username is only the E-Mail
      address of the user. This is used in createPost and modifyPost.
      @param fullName is a QString containing the blog username.

      @see username()
      @see createPost( KBlog::BlogPost* )
      @see modifiyPost( KBlog::BlogPost* )
    */
    virtual void setFullName( const QString &fullName );

    /**
      Returns the full name of user of the blog.
      @see setFullName()
    */
    QString fullName() const;

    /**
      Returns the profile id of the blog. This is used for rss paths internally.
      @return The profile id.

      @see setProfileId( const QString& )
    */
    QString profileId() const;

    /**
      Get the profile's id of the blog.
      @param pid This is nummeric id.

      @see profileId()
    */
    virtual void setProfileId( const QString &pid );

    /**
      Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
      Get information about the profile from the blog.
      Sets the profileId automatically for the blog it is called from.

      @see setProfileId( const QString& )
      @see void fetchedProfileId( const QString& )
    */
    void fetchProfileId();

    /**
      List the blogs available for this authentication on the server.

      @see void listedBlogs( const QList\<QMap\<QString,QString\>\>& )
    */
    virtual void listBlogs();

    /**
      List the comments available for this post on the server.
      @param post The post, which posts should be listed.

      @see void listedComments( KBlog::BlogPost*, const QList\<KBlog::BlogComment\>& )
    */
    virtual void listComments( KBlog::BlogPost *post );

    /**
      List the all comments available for this authentication on the server.

      @see void listedAllComments( const QList\<KBlog::BlogComment\>& )
    */
    virtual void listAllComments();

    /**
      List recent posts on the server. The status of the posts will be Fetched.
      @param number The number of posts to fetch. The order is newest first.

      @see     void listedPosts( const QList\<KBlog::BlogPost\>& )
      @see     void fetchPost( KBlog::BlogPost* )
      @see     BlogPost::Status
    */
    void listRecentPosts( int number );

    /**
      List recent posts on the server depending on meta information about the post.
      @param label The lables of posts to fetch.
      @param number The number of posts to fetch. The order is newest first.
      @param upMinTime The oldest upload time of the posts to fetch.
      @param upMaxTime The newest upload time of the posts to fetch.
      @param pubMinTime The oldest publication time of the posts to fetch.
      @param pubMaxTime The newest publication time of the posts to fetch.

      @see     void listedPosts( const QList\<KBlog::BlogPost\>& )
      @see     void fetchPost( KBlog::BlogPost* )
    */
    virtual void listRecentPosts( const QStringList &label=QStringList(), int number=0,
                                  const KDateTime &upMinTime=KDateTime(),
                                  const KDateTime &upMaxTime=KDateTime(),
                                  const KDateTime &pubMinTime=KDateTime(),
                                  const KDateTime &pubMaxTime=KDateTime() );

    /**
      Fetch the Post with a specific id.
      @param post This is the post with its id set correctly.

      @see BlogPost::setPostId( const QString& )
      @see fetchedPost( KBlog::BlogPost *post )
    */
    void fetchPost( KBlog::BlogPost *post );

    /**
      Modify a post on server.
      @param post This is used to send the modified post including the correct id.
    */
    void modifyPost( KBlog::BlogPost *post );

    /**
      Create a new post on server.
      @param post This is send to the server.

      @see createdPost( KBlog::BlogPost *post )
    */
    void createPost( KBlog::BlogPost *post );

    /**
      Remove a post from the server.
      @param post This is the post with its id set correctly.

      @see BlogPost::setPostId( const QString& )
      @see removedPost( KBlog::BlogPost* )
    */
    void removePost( KBlog::BlogPost *post );

    /**
      Create a comment on the server.
      @param post This is the post with its id set correctly.
      @param comment This is the comment to create.

      @see BlogPost::setPostId( const QString& )
      @see createdComment( KBlog::BlogPost*, KBlog::BlogComment*  )
    */
    virtual void createComment( KBlog::BlogPost *post, KBlog::BlogComment *comment );

    /**
      Remove a comment from the server.
      @param post This is the post with its id set correctly.
      @param comment This is the comment to remove.

      @see BlogPost::setPostId( const QString& )
      @see removedComment( KBlog::BlogPost*, KBlog::BlogComment*  )
    */
    virtual void removeComment( KBlog::BlogPost *post, KBlog::BlogComment *comment );

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
    void listedAllComments( const QList<KBlog::BlogComment> &commentsList );

    /**
      This signal is emitted when a list of comments has been fetched
      from the blogging server.
      @param post This is the corresponding post.
      @param comments The list of comments.

      @see listComments( KBlog::BlogPost* )
    */
    void listedComments( KBlog::BlogPost *post, const QList<KBlog::BlogComment> &comments );

    /**
      This signal is emitted when a comment has been created
      on the blogging server.
      @param post This is the corresponding post.
      @param comment This is the created comment.

      @see createComment( KBlog::BlogPost *post, KBlog::BlogComment *comment )
    */
    void createdComment( const KBlog::BlogPost *post, const KBlog::BlogComment *comment );

    /**
      This signal is emitted when a comment has been removed
      from the blogging server.
      @param post This is the corresponding post.
      @param comment This is the removed comment.

      @see removeComment( KBlog::BlogPost *post, KBlog::BlogComment *comment )
    */
    void removedComment( const KBlog::BlogPost *post, const KBlog::BlogComment *comment );

    /**
      This signal is emitted when the profile id has been
      fetched.
      @param profileId This is the fetched id. On error it is QString()

      @see fetchProfileId()
    */
    void fetchedProfileId( const QString &profileId );

  protected:
    /**
      Overloaded for private inheritance handling.
    */
    GData( const KUrl &server, GDataPrivate &dd, QObject *parent = 0 );

  private:
    Q_DECLARE_PRIVATE( GData )
    Q_PRIVATE_SLOT( d_func(),
                    void slotFetchProfileId( KJob * ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotListBlogs( Syndication::Loader *,
                                        Syndication::FeedPtr, Syndication::ErrorCode ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotListComments( Syndication::Loader *,
                                           Syndication::FeedPtr, Syndication::ErrorCode ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotListAllComments( Syndication::Loader *,
                                              Syndication::FeedPtr, Syndication::ErrorCode ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotListRecentPosts( Syndication::Loader *,
                                              Syndication::FeedPtr, Syndication::ErrorCode ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotFetchPost( Syndication::Loader *,
                                        Syndication::FeedPtr, Syndication::ErrorCode ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotCreatePost( KJob * ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotModifyPost( KJob * ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotRemovePost( KJob * ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotCreateComment( KJob * ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotRemoveComment( KJob * ) )
};

} //namespace KBlog
#endif
