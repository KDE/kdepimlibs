/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006 Christian Weilbach <christian@whiletaker.homeip.net>
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
#ifndef API_BLOG_H
#define API_BLOG_H

#include <kblog/kblog_export.h>

#include <QtCore/QObject>

class QString;
template <class T> class QList;

class KDateTime;
class KTimeZone;
class KUrl;

namespace KIO {
    class Job;
}

/**
  This is the main interface for blog backends
  @author Ian Reinhart Geiser, Reinhold Kainhofer, Christian Weilbach
*/

/**
  @file
  This file is part of the API for accessing Blog Servers
  and defines the BlogPosting, BlogMedia, and APIBlog class.

  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
*/

/** Namespace for blog related classes. */
namespace KBlog {

class BlogPosting;
class BlogMedia;

/**
  @brief
  A virtual basis class that represents a connection to a blog server.
  This is the main interface to the blog client library.

  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
*/

class KBLOG_EXPORT APIBlog : public QObject
{
  Q_OBJECT
  public:
    /**
      Construtor used by the API implementations.

      @param server the gateway url of the server.
      @param parent the parent of this object, defaults to NULL.
    */
    explicit APIBlog( const KUrl &server, QObject *parent = 0 );

    /**
      Destroys the APIBlog object.
    */
    virtual ~APIBlog();

    /**
      Enumeration for possible errors.
    */
    enum errorType {
      XmlRpc,
      AtomAPI,
      ParsingError,
      AuthenticationError,
      NotSupported,
      Other
    };

    /**
      Returns the API of the inherited object.
    */
    virtual QString interfaceName() const = 0;

    /**
      Sets the blog id of the Server.

      @param blogId
    */
    virtual void setBlogId( const QString &blogId );

    /**
      Returns the blog id.

      @return blogId
    */
    virtual QString blogId() const;

    /**
      Sets the password for the blog.
      @param pass is a QString containing the blog password.

      @see password();
    */
    virtual void setPassword( const QString &pass );

    /**
      Returns the password of the blog.
      @see setPassword();
    */
    virtual QString password() const;

    /**
      Sets the user's id for the blog.
      @param uid is a QString containing the blog username.

      @see userId()
    */
    virtual void setUserId( const QString &uid );

    /**
       Returns the user's id of the blog.
       @see setUserId()
    */
    virtual QString userId() const;
    /**
      Sets the user's name for the blog.
      @param uname is a QString containing the blog username.

      @see userName()
    */
    virtual void setUserName( const QString &uname );

    /**
       Returns the user's name of the blog.
       @see setUserName()
    */
    virtual QString userName() const;

    /**
        Get the E-Mail of the user.

        @return email

        @see setEmail( QString& email )
    */
    virtual QString email() const;

    /**
        Set the E-Mail of the user. This is used for authentication.

        @param email is the mail address of the user

        @see email()
    */
    virtual void setEmail( const QString& email );

    /**
      Sets the URL for the blog.
      @param url is the blog URL.

      @see url()
    */
    virtual void setUrl( const KUrl &url );

    /**
      Get the URL for the blog.
      @see setUrl()
    */
    virtual KUrl url() const;

    /**
      Sets the time zone of the blog server.
      @param tz time zone of the server
      @see timeZone()
    */
    virtual void setTimeZone( const KTimeZone &tz );

    /**
      Get the time zone of the blog server.
      @see void setTimeZone()
    */
    virtual KTimeZone timeZone();

    /**
      Sets the number of blog posts to be downloaded.
      @param nr number of posts to download. '0' gets all posts.
      @see listPostings()
      @see downloadCount()
    */
    virtual void setDownloadCount( int nr );

    /**
      Gets the number of blog posts to be downloaded.
      @see listPostings()
      @see setDownloadCount()
    */
    virtual int downloadCount() const;

    /**
      Get information about the user from the blog.
      @see userInfoRetrieved()
    */
    virtual void userInfo() = 0;

    /**
      List the blogs available for this authentication on the server.
      @see blogInfoRetrieved()
    */
    virtual void listBlogs() = 0;

    /**
      List recent postings on the server.
      @see listedPosting()
      @see fetchedPosting()
      @see listPostingsFinished()
    */
    virtual void listPostings() = 0;

    /**
      Fetch the Posting with postingId.
      @param postingId is the id of the posting on the server.

      @see fetchedPosting()
    */
    virtual void fetchPosting( KBlog::BlogPosting *posting ) = 0;

    /**
      Modify a posting on server.

      @param posting is used to send the modified posting including the
      correct postingId from it to the server.
    */
    virtual void modifyPosting( KBlog::BlogPosting *posting ) = 0;

    /**
      Create a new posting on server.

      @param posting is send to the server.
    */
    virtual void createPosting( KBlog::BlogPosting *posting ) = 0;

    /**
      Remove a posting from the server.

      @param posting* is the BlogPosting to remove.
    */
    virtual void removePosting( KBlog::BlogPosting *posting ) = 0;

  Q_SIGNALS:
    /**
      This signal is emitted when a userInfo() job fetches the user
      information from the blogging server.

'     @see userInfo()
    */
    virtual void userInfoRetrieved();

    /**
      This signal is emitted when a listBlogs() job fetches the blog
      information from the blogging server.

      @see listBlogs()
    */
    virtual void blogInfoRetrieved();

    /**
      This signal is emitted when a listPostings() job fetches a posting
      from the blogging server.

      @param posting is the fetched posting.

      @see listPostings()
    */
    virtual void listedPosting( KBlog::BlogPosting &posting );

    /**
      This signal is emitted when a fetchPosting() job fetches a posting
      from the blogging server.

      @param posting is the fetched posting.

      @see fetchPosting(KBlog::BlogPosting*)
    */
//     virtual void fetchedPosting( KBlog::BlogPosting &posting );

    /**
      This signal is emitted when a createPosting() job successfully creates
      a posting on the server.

      @param id is the id the posting has on the server.

      @see createPosting( KBlog::BlogPosting* )
    */
//     virtual void createdPosting( const QString &id );

    /**
      This signal is emitted when a createMedia() job successfully creates
      a posting on the server.

      @param url is the url of the posting on the server. This, depending
      on the server, can only be an id string, too.

      @see createMedia( KBlog::BlogMedia* )
    */
//     virtual void createdMedia( const QString &url );

    /**
      This signal is emitted when a modifyPosting() job modifies a posting
      on the server.

      @param modified shows the success of the modification.

      @see modifyPosting( KBlog::BlogPosting* )
    */
//     virtual void modifiedPosting( void modified );

    /**
      This signal is emitted when the last posting of the listPostings()
      job has been fetched.

      @see listPostings()
    */
    virtual void listPostingsFinished();

    /**
      This signal is emitted when the last category of the listCategories()
      job has been fetched.

      @see listCategories()
    */
    virtual void listCategoriesFinished();

    /**
      All xml parsing and all structural problems will emit an error.

      @see errorType
    */
    virtual void error( const errorType &type, const QString &errorMessage );

  protected:
    class APIBlogPrivate;
    APIBlogPrivate *const d;
};

}
#endif
