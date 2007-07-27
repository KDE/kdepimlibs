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

#ifndef KBLOG_GDATA_H
#define KBLOG_GDATA_H

#include <kblog/blog.h>

class KUrl;
class GDataPrivate;


/**
  @file

  This file is part of the  for accessing Blog Servers
  and defines the GData class.

  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>

  \par Maintainer: Christian Weilbach \<christian\@whiletaker.homeip.net\>
 */

namespace KBlog {

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
        Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
        Get information about the user from the blog. Note: This is not
        supported on the server side.
        @see void fetchedUserInfo( const QString &nickname,
                const QString &userid, const QString &email )
    */
    void fetchUserInfo();

    /**
        List the blogs available for this authentication on the server.
        Note: This is not supported on the server side.
        @see void blogInfoRetrieved( const QString &id, const QString &name )
    */
    void listBlogs();

    /**
        List recent postings on the server..
        @see     void listedPosting( KBlog::BlogPosting &posting )
        @see     void fetchedPosting( KBlog::BlogPosting &posting )
        @see     void listRecentPostingsFinished()
    */
    void listRecentPostings( int number );

    void listComments( KBlog::BlogPosting *posting );

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

    void createComment( KBlog::BlogPosting *posting, KBlog::BlogPostingComment *comment );

    /**
      Sets the user's name for the blog.
      @param fullName is a QString containing the blog username.

      @see username()
    */
    void setFullName( const QString &fullName );

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
    void setProfileId( const QString &pid );
  protected:
    GData( const KUrl &server, GDataPrivate &dd, QObject *parent = 0 );
  private:
    Q_DECLARE_PRIVATE(GData)
//     Q_PRIVATE_SLOT(d, void slotData( KIO::Job *, const QByteArray& ))
};

} //namespace KBlog
#endif
