/*
    This file is part of the kblog library.

    Copyright (c) 2007 Christian Weilbach <christian@whiletaker.homeip.net>

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
#ifndef API_GDATA_H
#define API_GDATA_H

#include <kblog/blog.h>

#include <QtCore/QString>

class KUrl;

/**
  @file

  This file is part of the API for accessing Blog Servers
  and defines the APIGData class.

  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>

  \par Maintainer: Christian Weilbach \<christian\@whiletaker.homeip.net\>
 */

namespace KBlog {
/**
  @brief
  A class that can be used for access to GData API blogs. Almost every blog server
  supports GData API . Compared to Blogger API 1.0 it is a superset of functions added to the its definition. GData API is much more functional, but has some drawbacks, e.g. security when compared to Blogger API 2.0 which is based on GData API and quite new.

  @code
  APIBlog* myblog = new APIGData( "http://example.com/xmlrpc/gateway.php" );
  KBlog::BlogPosting *post = new BlogPosting();
  post->setUserId( "some_user_id" );
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPosting( posting );
  @endcode

  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
 */
class KBLOG_EXPORT APIGData : public APIBlog
{
  Q_OBJECT
  public:
    /**
         Create an object for GData API

         @param server is the url for the xmlrpc gateway.
    */
    explicit APIGData( const KUrl &server, QObject *parent = 0 );
    virtual ~APIGData();

    /**
        Returns the API of the inherited object.
    */
    QString interfaceName() const;

    /**
        Sets the blogId accordingly to the HTML website.

        @see KBlog::APIBlog::setBlogId( QString& )
    */
    virtual void getIntrospection();

    /**
        Get information about the user from the blog. Note: This is not supported on the server side.
	@see void userInfoRetrieved( const QString &nickname, const QString &userid, const QString &email )
    */
    virtual void userInfo();

    /**
        List the blogs available for this authentication on the server. Note: This is not supported on the server side.
	@see void blogInfoRetrieved( const QString &id, const QString &name )
    */
    virtual void listBlogs();

    /**
        List recent postings on the server..
	@see     void listedPosting( KBlog::BlogPosting &posting )
        @see     void fetchedPosting( KBlog::BlogPosting &posting )
        @see     void listPostingsFinished()
    */
    virtual void listPostings();

    /**
        List the categories of the blog.
	@see  void categoryInfoRetrieved( const QString &name, const QString &description )
        @see  void listCategoriesFinished()
    */
    virtual void listCategories();

    /**
        Fetch the Posting with postingId.
        @param postingId is the id of the posting on the server.

        @see  void fetchedPosting( KBlog::BlogPosting &posting )
    */
    virtual void fetchPosting( KBlog::BlogPosting *posting );

    /**
        Modify a posting on server.

        @param posting is used to send the modified posting including the correct postingId from it to the server.
    */
    virtual void modifyPosting( KBlog::BlogPosting *posting );

    /**
        Create a new posting on server.

        @param posting is send to the server.
    */
    virtual void createPosting( KBlog::BlogPosting *posting );

    /**
        Create a new media object, e.g. picture, on server.

        @param media is send to the server.
    */
    virtual void createMedia( KBlog::BlogMedia *media );

    /**
        Remove a posting from the server.

        @param postingId is the id of the posting to remove.

        @see void removePosting( KBlog::BlogPosting *posting )
    */
    virtual void removePosting( KBlog::BlogPosting *posting );

    /**
        Get the E-Mail of the user.

        @return email

        @see setEmail( QString& email )
    */
    virtual QString email();

    /**
        Set the E-Mail of the user. This is used for authentication.

        @param email is the mail address of the user

        @see email()
    */
    virtual void setEmail( const QString& email );

private:
    class APIGDataPrivate;
    APIGDataPrivate* const d;
};

}
#endif
