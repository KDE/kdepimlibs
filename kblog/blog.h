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
#ifndef API_BLOG_H
#define API_BLOG_H

#include <kblog.h>
#include <kurl.h>
#include <kio/job.h>
#include <ktimezones.h>
#include <kdatetime.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>

/**
This is the main interface for blog backends
@author Ian Reinhart Geiser, Reinhold Kainhofer, Christian Weilbach
*/

namespace KBlog {

/**
  @file

  This file is part of the API for accessing Blog Servers
  and defines the #BlogPosting, #BlogMedia, and #APIBlog class.

  @author Reinhold Kainhofer <reinhold@kainhofer.com>
  @author Christian Weilbach <christian@whiletaker.homeip.net>

  \par Maintainer: Christian Weilbach <christian@whiletaker.homeip.net>
 */

/**
  @brief
  A class that represents a blog posting on the server.

  @code
    KBlog::BlogPosting *post = new BlogPosting();
    post->setUserId( "some_user_id" );
    post->setBlogId( "some_blog_id" );
    post->setTitle( "This is the title." );
    post->setContent( "Here is some the content..." );
  @endcode

  @author Christian Weilbach <christian@whiletaker.homeip.net>
 */


class KBLOG_EXPORT BlogPosting
{
public:
  /**
    Default constructor. Creates an empty BlogPosting object.
  */
  BlogPosting();

  /**
    Constructor for convenience.

    @param title
    @param content
    @param category
    @param publish
  */
  BlogPosting( const QString& title, const QString& content, 
               const QString& category = QString::null, 
	       const bool publish = true );
  
  /**
    Virtual default destructor.
  */
  virtual ~BlogPosting();

  /**
    Returns if the posting is published or not.

    @result bool
    @see setPublish( const bool publish )
  */
  bool publish() const;

  /**
    Set the publish value.

    @param publish set this to true, if you want to publish immediately.
    @see publish()
  */
  void setPublish( const bool publish );

  /**
    Returns the userId.

    @result userId
    @see setUserId( const QString &userId )
  */
  QString userId() const;
   
  /**
    Set the userId value.

    @param userId set this to the user id on the server.
    @see userId()
  */ 
  void setUserId( const QString &userId );

  /**
    Returns the blogId.

    @result blogId
    @see setBlogId( const QString &blogId )
  */
  QString blogId() const;

  /**
    Set the blogId value.

    @param blogId set this to the blod id on the server.
    @see blogId()
  */
  void setBlogId( const QString &blogId );

  /**
    Returns the postId.

    @result postId
    @see setPostId( const QString &postId )
  */
  QString postId() const;

  /**
    Set the post id value.

    @param postId set this to the post id on the server.
    @see postId()
  */
  void setPostId( const QString &postId );

  /**
    Returns the title.

    @result title
    @see setTitle( const QString &title )
  */
  QString title() const;

  /**
    Set the title.

    @param title set the title.
    @see title()
  */
  void setTitle( const QString &title );

  /**
    Returns the content.

    @result content
    @see setContent( const QString &content )
  */
  QString content() const;

  /**
    Set the content.

    @param content set the content.
    @see content()
  */
  void setContent( const QString &content );

  /**
    Returns the category.

    @result category
    @see setCategory( const QString &category )
  */
  QString category() const;

  /**
    Set the category.

    @param category set the category.
    @see category()
  */
  void setCategory( const QString &category );

  /**
    Returns the creation date time.

    @result creationdatetime
    @see setCreationDateTime( const QString &datetime )
  */
  KDateTime creationDateTime() const;

  /**
    Set the creation time.

    @param datetime set the time the posting has been created.
    @see creationTime()
  */
  void setCreationDateTime( const KDateTime &datetime );
  
  /**
    Returns the modification date time.

    @result modificationdatetime
    @see setModificationDateTime( const QString &datetime )
    @see creationDateTime()
  */
  KDateTime modificationDateTime() const;

  /**
    Set the modification time.

    @param datetime set the time the posting has been modified.
    @see modificationTime()
    @see setCreationDateTime( const KDateTime &datetime )
  */
  void setModificationDateTime( const KDateTime &datetime );
  
  /**
    Returns if the post has been deleted on the server.

    @result deleted
    @see setDeleted( const bool deleted )
  */
  bool deleted() const;
  
  /**
    Set when the posting has been deleted on the server.

    @param deleted set to the status of the posting.
    @see deleted()
  */
  void setDeleted( const bool deleted );
  
  /**
    Returns if the post has been uploaded to the server.

    @result uploaded
    @see setUploaded( const bool uploaded )
  */
  bool uploaded() const;

  /**
    Set when the posting has been uploaded to the server.

    @param uploaded set the status of the posting.
    @see uploaded()
  */
  void setUploaded( const bool deleted);
//FIXME:   virtual void error( int /*code*/, const QString &/*error*/ ) {}

protected:
  // Override this method to detect the new postId assigned when adding a new post
  virtual void assignPostId( const QString &/*postId*/ ) {}
private:
  class Private;
  Private* const d;
};

/**
  @brief
  A class that represents a media object on the server.

  @code
    KBlog::BlogMedia *media = new BlogMedia();
    ost->setMimetype( "some_mimetype" );
    post->setData( some_qbytestream );
  @endcode

  @author Christian Weilbach <christian@whiletaker.homeip.net>
 */


class KBLOG_EXPORT BlogMedia : public BlogPosting {
public:
  /**
    Default constructor. Creates an empty BlogMedia object.
  */
  BlogMedia();

  /**
    Virtual default destructor.
  */
  virtual ~BlogMedia();
  
  /**
    Return the mimetype.
    
    @result mimetype of the object
    @see setMimetype()
  */
  QString mimetype();
  void setMimetype( const QString& mimetype );
  
  /**
    Return the data of the file.

    @result data
    @see setData()
  */
  QByteArray data();
  void setData( const QByteArray& data );

private:
  class Private;
  Private* const d;
};

/**
  @brief
  A virtual basis class that represents a connection to a blog server.
  This is the main interface to the blog client library.

  @author Christian Weilbach <christian@whiletaker.homeip.net>
  @author Reinhold Kainhofer <reinhold@kainhofer.com>
 */

class KBLOG_EXPORT APIBlog : public QObject
{
  Q_OBJECT
  public:
      /**
                        Construtor used by the API implementations.

      @param server the server url of the server.
      @param parent the parent of this object, defaults to NULL.
      @param name  the name of the instance.
     */
    APIBlog( const KUrl &server, QObject *parent = 0L, const char *name = 0L );

     /**
       Destroys the APIBlog object.
     */
    virtual ~APIBlog();

     /**
        Returns the API of the inherited object.
     */
    virtual QString interfaceName() const = 0;

     /**
        Set the blod id of the Server.

        @param blogId
     */
    void setBlogId( const QString &blogId );

     /**
        Returns the blog id.

        @result blogId
     */
    QString blogId() const;

     /**
        Sets the password for the blog.
        @see password();
     */
    void setPassword( const QString &pass );
     /**
        Returns the password of the blog.
        @see setPassword();
    */
    QString password() const;

    /**
       Sets the username for the blog.
       @see username()
    */
    void setUsername( const QString &uname );

    /**
       Get the username of the blog.
       @see setUsername()
    */
    QString username() const;

    /**
        Sets the URL for the blog.
        @see url()
    */
    void setUrl( const KUrl& url );

    /**
        Get the URL for the blog.
        @see setUrl()
    */
    KUrl url() const;

    /**
        Sets the time zone of the blog server.
	@param tz time zone of the server
	@see timezone()
    */
    void setTimezone( const KTimeZone& tz );

    /**
        Get the time zone of the blog server.
	@see void setTimezone( const KTimeZone& tz )
    */
    KTimeZone timezone();

    // TODO once again, do we need this?
    void setDownloadCount( int nr );
    int downloadCount() const;

    enum blogFunctions {
      blogGetUserInfo,
      blogGetUsersBlogs,
      blogGetCategories,
      blogGetRecentPosts,
      blogNewPost,
      blogNewMedia,
      blogEditPost,
      blogDeletePost,
      blogGetPost,
      blogGetTemplate, // not implemented yet
      blogSetTemplate  // not implemented yet
    };

    /**
        Returns the function name for the corresponding blog backend.

        @result function name
	@param type the type of the blogFunction
    */
    virtual QString getFunctionName( blogFunctions type ) = 0;

    /**
        Returns the default Arguments for the blog.
        
	@result list of QVariants with the default args
	@param id of the blog.
    */
    virtual QList<QVariant> defaultArgs( const QString &id = QString::null );


    virtual void userInfo() = 0;
    virtual void listBlogs() = 0;
    virtual void listPostings() = 0;
    virtual void listCategories() = 0;
    virtual void fetchPosting( const QString &postId ) = 0;
    void fetchPosting( KBlog::BlogPosting *posting );
    virtual void modifyPosting( KBlog::BlogPosting *posting ) = 0;
    virtual void createPosting( KBlog::BlogPosting *posting ) = 0;
    virtual void createMedia( KBlog::BlogMedia *media ) = 0;
    virtual void removePosting( const QString &postId ) = 0;
    void removePosting( KBlog::BlogPosting *posting );

  signals:
    void userInfoRetrieved( const QString &nickname, const QString &userid, const QString &email );
    void folderInfoRetrieved( const QString &id, const QString &name );
    void categoryInfoRetrieved( const QString &name, const QString &description );
    void mediaInfoRetrieved( const QString &url );

    void itemOnServer( KBlog::BlogPosting &posting );
    void error( const QString &errorMessage );
    void uploadPostId( const int );
    void fetchingPostsFinished();
    void fetchingCategoriesFinished();

  private:
    class Private;
    Private* const d;
};

}
#endif
