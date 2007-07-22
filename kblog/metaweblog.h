/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006 Christian Weilbach <christian@whiletaker.homeip.net>

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
#ifndef API_METAWEBLOG_H
#define API_METAWEBLOG_H

#include <kblog/blog.h>
#include <kblog/blogger.h>

#include <kurl.h>

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QList>

/**
  @file
  This file is part of the API for accessing Blog Servers
  and defines the APIMetaWeblog class.

  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
*/

namespace KBlog {
/**
  @brief
  A class that can be used for access to MetaWeblog API blogs. Almost every
  blog server supports MetaWeblog API . Compared to Blogger API 1.0 it is a
  superset of functions added to the its definition. MetaWeblog API is much
  more functional, but has some drawbacks, e.g. security when compared to
  Blogger API 2.0 which is based on Atom API and quite new.

  @code
  APIBlog* myblog = new APIMetaWeblog( "http://example.com/xmlrpc/gateway.php" );
  KBlog::BlogPosting *post = new BlogPosting();
  post->setUserId( "some_user_id" );
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPosting( posting );
  @endcode

  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
*/
class KBLOG_EXPORT APIMetaWeblog : public APIBlogger
{
  Q_OBJECT
  public:
    /**
      Create an object for MetaWeblog API

      @param server is the url for the xmlrpc gateway.
      @param parent is the parent object.
    */
    explicit APIMetaWeblog( const KUrl &server, QObject *parent = 0 );

    /**
      Destroy the object.
    */
    virtual ~APIMetaWeblog();

    /**
      Returns the API of the inherited object.
    */
    QString interfaceName() const;

    /**
      Set the Url of the server.

      @param server is the server url.
    */
    void setUrl( const KUrl &server );

    /**
      List recent postings on the server.
      @see     void listedPosting( KBlog::BlogPosting &posting )

      @see     void listPostingsFinished()
    */
    virtual bool listPostings();

    /**
      List the categories of the blog.
      @see  void categoryInfoRetrieved( const QString &, const QString & )
      @see  void listCategoriesFinished()
    */
    virtual bool listCategories();

    /**
      Fetch the Posting with postingId.
      @param postingId is the id of the posting on the server.

      @see  void fetchedPosting( KBlog::BlogPosting &posting )
    */
    virtual bool fetchPosting( const QString &postingId );

    /**
      Modify a posting on server.

      @param posting is used to send the modified posting including the
      correct postingId from it to the server.
    */
    virtual bool modifyPosting( KBlog::BlogPosting *posting );

    /**
      Create a new posting on server.

      @param posting is send to the server.
    */
    virtual bool createPosting( KBlog::BlogPosting *posting );

    /**
      Create a new media object, e.g. picture, on server.

      @param media is send to the server.
    */
    virtual bool createMedia( KBlog::BlogMedia *media );

  private:
    class APIMetaWeblogPrivate;
    APIMetaWeblogPrivate *const d;
};

}
#endif
