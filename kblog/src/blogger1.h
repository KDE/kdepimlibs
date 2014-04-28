/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  Copyright (c) 2007 Mike McQuaid <mike@mikemcquaid.com>

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

#ifndef KBLOG_BLOGGER1_H
#define KBLOG_BLOGGER1_H

#include <blog.h>

class KUrl;

/**
  @file
  This file is part of the  for accessing Blog Servers
  and defines the Blogger1 class.

  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
  @author Christian Weilbach \<christian_weilbach\@web.de\>
*/

namespace KBlog {

class Blogger1Private;

/**
   @brief
   A class that can be used for access to Blogger  1.0 blogs.
   Almost every blog server supports Blogger  1.0. Compared to
   MetaWeblog  it is not as functional and is obsolete on blogspot.com
   compared to GData which uses Atom instead of Xml-Rpc.

   @code
   Blog* myblog = new Blogger1("http://example.com/xmlrpc/gateway.php");
   myblog->setUsername( "some_user_id" );
   myblog->setPassword( "YoUrFunnYPasSword" );
   myblog->setBlogId( "1" ); // can be caught by listBlogs()
   KBlog::BlogPost *post = new BlogPost();
   post->setTitle( "This is the title." );
   post->setContent( "Here is some the content..." );
   myblog->createPost( post );
   @endcode

   @author Christian Weilbach \<christian_weilbach\@web.de\>
   @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
*/
class KBLOG_EXPORT Blogger1 : public Blog
{
  Q_OBJECT
  public:
    /**
      Create an object for Blogger 1.0

      @param server is the url for the xmlrpc gateway.
      @param parent the parent object.
    */
    explicit Blogger1( const KUrl &server, QObject *parent = 0 );

    /**
       Destroy the object.
    */
    virtual ~Blogger1();

    /**
      Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
       Set the Url of the server.

       @param server is the server Url.
    */
    void setUrl( const KUrl &server );

    /**
        Get information about the user from the blog. Note: This is not
        supported on the server side.
        @see void fetchedUserInfo( const QMap\<QString,QString\>& )
    */
    virtual void fetchUserInfo();

    /**
      List the blogs available for this authentication on the server.
      @see void listedBlogs( const QList\<QMap\<QString,QString\> \>& )
    */
    virtual void listBlogs();

    /**
      List recent posts on the server. The status of the posts will be Fetched.

     @param number The number of posts to fetch. Latest first.

      @see     void listedRecentPosts( QList\<KBlog::BlogPost> & )
      @see     void fetchPost( KBlog::BlogPost *post )
      @see     BlogPost::Status
    */
    void listRecentPosts( int number );

    /**
      Fetch a post from the server.

      @param post is the post. Note: Its id has to be set
      appropriately.

      @see BlogPost::setPostId( const QString& )
      @see fetchedPost( KBlog::BlogPost *post )
    */
    void fetchPost( KBlog::BlogPost *post );

    /**
      Modify a post on server.

      @param post is used to send the modified post including
      the correct postId from it to the server.

      @see  void modifiedPost( KBlog::BlogPost *post )
    */
    void modifyPost( KBlog::BlogPost *post );

    /**
      Create a new post on server.

      @param post is sent to the server.

      @see createdPost( KBlog::BlogPost *post )
    */
    void createPost( KBlog::BlogPost *post );

    /**
      Remove a post from the server.

      @param post is the post. Note: Its id has to be set
      appropriately.

      @see BlogPost::setPostId( const QString& )
      @see removedPost( KBlog::BlogPost *post )
    */
    void removePost( KBlog::BlogPost *post );

  Q_SIGNALS:

    /**
      This signal is emitted when a listBlogs() job fetches the blog
      information from the blogging server.

      @param blogsList The list of maps, in which each maps corresponds to
      a blog on the server. Each map has the keys id and name.

      @see listBlogs()
    */
    void listedBlogs( const QList<QMap<QString,QString> >& blogsList );

    /**
      This signal is emitted when a fetchUserInfo() job fetches the blog
      information from the blogging server.

      @param userInfo The map with the keys: nickname,
      userid, url, email, lastname, firstname. Note: Not all keys are
      supported by all servers.

      @see fetchUserInfo()
    */
    void fetchedUserInfo( const QMap<QString,QString> &userInfo );

  protected:
    /**
      Constructor needed for private inheritance.
    */
    Blogger1( const KUrl &server, Blogger1Private &dd, QObject *parent = 0 );

  private:
    Q_DECLARE_PRIVATE( Blogger1 )
    Q_PRIVATE_SLOT( d_func(),
                    void slotFetchUserInfo( const QList<QVariant> &, const QVariant & ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotListBlogs( const QList<QVariant> &, const QVariant & ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotListRecentPosts( const QList<QVariant> &, const QVariant & ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotFetchPost( const QList<QVariant> &, const QVariant & ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotCreatePost( const QList<QVariant> &, const QVariant & ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotModifyPost( const QList<QVariant> &, const QVariant & ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotRemovePost( const QList<QVariant> &, const QVariant & ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotError( int, const QString &, const QVariant & ) )
};

} //namespace KBlog
#endif
