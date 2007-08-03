/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
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

#ifndef KBLOG_BLOG_H
#define KBLOG_BLOG_H

#include <kblog/kblog_export.h>

#include <QtCore/QObject>

template <class T,class S> class QMap;

class KTimeZone;
class KUrl;

/**
  This is the main interface for blog backends
  @author Ian Reinhart Geiser, Reinhold Kainhofer, Christian Weilbach
*/

/**
  @file
  This file is part of the  for accessing Blog Servers
  and defines the BlogPosting, BlogMedia, and Blog class.

  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
  @author Christian Weilbach \<christian_weilbach\@web.de\>
*/

/** Namespace for blog related classes. */
namespace KBlog {

class BlogPosting;
class BlogMedia;
class BlogPrivate;

/**
  @brief
  A virtual basis class that represents a connection to a blog server.
  This is the main interface to the blog client library.

  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
*/

class KBLOG_EXPORT Blog : public QObject
{
  Q_OBJECT
  public:
    /**
    Construtor used by the  implementations.

    @param server the gateway url of the server.
    @param parent the parent of this object, defaults to NULL.
    @param applicationName the client application's name to use in the
    user agent, defaults to empty string.
    @param applicationVersion the client application's version to use in the
    user agent, defaults to empty string.
     */
    explicit Blog( const KUrl &server, QObject *parent = 0,
                   const QString &applicationName = QString(),
                   const QString &applicationVersion = QString() );

    /**
      Destroys the Blog object.
    */
    virtual ~Blog();

    /**
      Enumeration for possible errors.
    */
    enum ErrorType {
      XmlRpc,
      Atom,
      ParsingError,
      AuthenticationError,
      NotSupported,
      Other
    };

    /**
      Returns user agent used in requests.
    */
    QString userAgent() const;

    /**
      Returns the  of the inherited object.
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
    QString blogId() const;

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
    QString password() const;

    /**
      Sets the user's authentication name for the blog.
      @param username is a QString containing the blog username.

      @see userId()
    */
    virtual void setUsername( const QString &userName );

    /**
       Returns the user's id of the blog.
       @see setUserId()
    */
    QString username() const;

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
    KUrl url() const;

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
    KTimeZone timeZone();

    /**
      List recent postings on the server.
      @see listedPosting()
      @see fetchedPosting()
      @see listRecentPostingsFinished()
    */
    virtual void listRecentPostings( int number ) = 0;

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
      This signal is emitted when a listRecentPostings() job fetches a posting
      from the blogging server.

      @param posting is the fetched posting.

      @see listRecentPostings()
    */
    void listedRecentPostings(
        const QList<KBlog::BlogPosting>& postings );

    void createdPosting( KBlog::BlogPosting *posting );

    void fetchedPosting( KBlog::BlogPosting *posting );

    void modifiedPosting( KBlog::BlogPosting *posting );

    void removedPosting( KBlog::BlogPosting *posting );

    /**
      All xml parsing and all structural problems will emit an error.

      @see ErrorType
    */
    void error( KBlog::Blog::ErrorType type,
                        const QString &errorMessage, KBlog::BlogPosting* = 0 );

  protected:
    BlogPrivate * const d_ptr;
    Blog( const KUrl &server, BlogPrivate &dd, QObject *parent = 0,
          const QString &applicationName = QString(),
          const QString &applicationVersion = QString() );

  private:
    void setUserAgent( const QString &applicationName,
                       const QString &applicationVersion );
    Q_DECLARE_PRIVATE(Blog)
};

} //namespace KBlog
#endif
