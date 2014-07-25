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

#ifndef KBLOG_BLOG_H
#define KBLOG_BLOG_H

#include <kblog_export.h>

#include <QtCore/QObject>

template <class T, class S> class QMap;

class KTimeZone;
class QUrl;

/**
  This is the main interface for blogging APIs.
  It's methods represent the core functionality of a blogging API.
  @author Reinhold Kainhofer, Christian Weilbach and Mike McQuaid.
*/

/**
  @file
  This file is part of the library for accessing blogs and defines the
  Blog class.

  @author Christian Weilbach \<christian_weilbach\@web.de\>
  @author Mike McQuaid \<mike\@mikemcquaid.com\>
  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
*/

/** Namespace for blog related classes. */
namespace KBlog
{

class BlogPost;
class BlogComment;
class BlogMedia;
class BlogPrivate;

/**
  @brief
  A class that provides methods to call functions on a supported blog
  web application.
  This is the main interface to the blogging client library.

  @author Christian Weilbach \<christian_weilbach\@web.de\>
  @author Mike McQuaid \<mike\@mikemcquaid.com\>
  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
*/

class KBLOG_EXPORT Blog : public QObject
{
    Q_OBJECT
public:
    /**
      Constructor used by the remote interface implementations.

      @param server URL for the blog's remote interface.
      @param parent the parent of this object, defaults to null.
      @param applicationName the client application's name to use in the
      HTTP user agent string, defaults to KBlog's own.
      @param applicationVersion the client application's version to use in the
      HTTP user agent string, defaults to KBlog's own.
    */
    explicit Blog(const QUrl &server, QObject *parent = 0,
                  const QString &applicationName = QString(),
                  const QString &applicationVersion = QString());

    /**
      Destroys the Blog object.
    */
    virtual ~Blog();

    /**
      Enumeration for possible errors.
    */
    enum ErrorType {
        /** An error in the XML-RPC client. */
        XmlRpc,
        /** An error in the syndication client. */
        Atom,
        /** A parsing error. */
        ParsingError,
        /** An error on authentication. */
        AuthenticationError,
        /** An error where the method called is not supported by this object. */
        NotSupported,
        /** Any other miscellaneous error. */
        Other
    };

    /**
      Returns the HTTP user agent string used to make the HTTP requests.
    */
    QString userAgent() const;

    /**
    Sets the HTTP user agent string used to make the HTTP requests.

    @param applicationName the client application's name to use in the
    HTTP user agent string.
    @param applicationVersion the client application's version to use in the
    HTTP user agent string.
    @see userAgent()
    */
    void setUserAgent(const QString &applicationName,
                      const QString &applicationVersion);

    /**
      Returns the name of the blogging API this object implements.
    */
    virtual QString interfaceName() const = 0;

    /**
      Sets the unique ID for the specific blog on the server.
      @param blogId the ID of the blog to send/receive from.
      @see blogId();
    */
    virtual void setBlogId(const QString &blogId);

    /**
      Returns the unique ID for the specific blog on the server.
      @see setBlogId( const QString &blogId );
    */
    QString blogId() const;

    /**
      Sets the password used in blog authentication.
      @param password the blog's password.

      @see password();
    */
    virtual void setPassword(const QString &password);

    /**
      Returns the password of the blog.
      @see setPassword( const QString & );
    */
    QString password() const;

    /**
      Sets the username used in blog authentication.
      @param username the blog's username.
      @see username()
    */
    virtual void setUsername(const QString &username);

    /**
      Returns the username used in blog authentication.

      @see setUsername( const QString & )
    */
    QString username() const;

    /**
      Sets the URL for the blog's XML-RPC interface.

      @param url the blog's XML-RPC URL.
      @see url()
    */
    virtual void setUrl(const QUrl &url);

    /**
      Get the URL for the blog's XML-RPC interface.

      @see setUrl( const QUrl & )
    */
    QUrl url() const;

    /**
      Sets the time zone of the blog's server.

      @param timeZone the time zone of the server.
      @see timeZone()
    */
    virtual void setTimeZone(const KTimeZone &timeZone);

    /**
      Get the time zone of the blog's server.

      @see void setTimeZone()
    */
    KTimeZone timeZone();

    /**
      List a number of recent posts from the server.
      The posts are returned in descending chronological order.

      @param number the number of posts to fetch.
      @see listedRecentPosts( const QList<KBlog::BlogPost>& posts )
    */
    virtual void listRecentPosts(int number) = 0;

    /**
      Fetch a blog post from the server with a specific ID.
      The ID of the existing post must be retrieved using getRecentPosts
      and then be modified and provided to this method or a new BlogPost
      created with the existing ID.

      @param post a blog post with the ID identifying the blog post to fetch.
      @see fetchedPost()
      @see listedRecentPosts( int number )
    */
    virtual void fetchPost(KBlog::BlogPost *post) = 0;

    /**
      Modify an existing blog post on the server.
      The ID of the existing post must be retrieved using getRecentPosts
      and then be modified and provided to this method or a new BlogPost
      created with the existing ID.

      @param post the new blog post.
      @see modifiedPost()
      @see listedRecentPosts( int number )
    */
    virtual void modifyPost(KBlog::BlogPost *post) = 0;

    /**
      Create a new blog post on the server.

      @param post the blog post to create.
      @see createdPost()
    */
    virtual void createPost(KBlog::BlogPost *post) = 0;

    /**
      Remove an existing blog post from the server.
      The BlogPost object representing the existing post must be retrieved
      using getRecentPosts and then provided to this method.

      @param post* the blog post to remove.
      @see removedPost()
      @see listedRecentPosts( int number )
    */
    virtual void removePost(KBlog::BlogPost *post) = 0;

Q_SIGNALS:
    /**
      This signal is emitted when a listRecentPosts() job fetches a post
      from the blogging server.

      @param posts the list of posts.
      @see listRecentPosts()
    */
    void listedRecentPosts(
        const QList<KBlog::BlogPost> &posts);

    /**
      This signal is emitted when a createPost() job creates a new blog post
      on the blogging server.

      @param post the created post.
      @see createPost()
    */
    void createdPost(KBlog::BlogPost *post);

    /**
      This signal is emitted when a fetchPost() job fetches a post
      from the blogging server.

      @param post the fetched post.
      @see fetchPost()
    */
    void fetchedPost(KBlog::BlogPost *post);

    /**
      This signal is emitted when a modifyPost() job modifies a post
      on the blogging server.

      @param post the modified post.
      @see modifyPost()
    */
    void modifiedPost(KBlog::BlogPost *post);

    /**
      This signal is emitted when a removePost() job removes a post
      from the blogging server.

      @param post the removed post.
      @see removePost()
    */
    void removedPost(KBlog::BlogPost *post);

    /**
      This signal is emitted when an error occurs with XML parsing or a
      structural problem.

      @param type the type of the error.
      @param errorMessage the error message.
      @see ErrorType
    */
    void error(KBlog::Blog::ErrorType type, const QString &errorMessage);

    /**
      This signal is emitted when an error occurs with XML parsing or a
      structural problem in an operation involving a blog post.

      @param type the type of the error.
      @param errorMessage the error message.
      @param post the post that caused the error.
      @see ErrorType
    */
    void errorPost(KBlog::Blog::ErrorType type,
                   const QString &errorMessage, KBlog::BlogPost *post);

    /**
      This signal is emitted when an error occurs with XML parsing or a
      structural problem in an operation involving some blog media.

      @param type the type of the error.
      @param errorMessage the error message.
      @param media the media that caused the error.
      @see ErrorType
    */
    void errorMedia(KBlog::Blog::ErrorType type,
                    const QString &errorMessage, KBlog::BlogMedia *media);

    /**
      This signal is emitted when an error occurs with XML parsing or a
      structural problem in an operation involving a blog post's comment.

      @param type the type of the error.
      @param errorMessage the error message.
      @param post the post that caused the error.
      @param comment the comment that caused the error.
      @see ErrorType
    */
    void errorComment(KBlog::Blog::ErrorType type,
                      const QString &errorMessage, KBlog::BlogPost *post,
                      KBlog::BlogComment *comment);

protected:
    /** A pointer to the corresponding 'Private' class */
    BlogPrivate *const d_ptr;

    /**
      Constructor needed to allow private inheritance of 'Private' classes.

      @param server URL for the blog's XML-RPC interface.
      @param dd URL for the corresponding private class.
      @param parent the parent of this object, defaults to null.
      @param applicationName the client application's name to use in the
      HTTP user agent string, defaults to KBlog's own.
      @param applicationVersion the client application's version to use in the
      HTTP user agent string, defaults to KBlog's own.
    */
    Blog(const QUrl &server, BlogPrivate &dd, QObject *parent = 0,
         const QString &applicationName = QString(),
         const QString &applicationVersion = QString());

private:
    Q_DECLARE_PRIVATE(Blog)
};

} //namespace KBlog
#endif
