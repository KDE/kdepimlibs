/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>

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

#ifndef KBLOG_METAWEBLOG_H
#define KBLOG_METAWEBLOG_H

#include <kblog/blogger1.h>

class KUrl;

/**
  @file
  This file is part of the  for accessing Blog Servers
  and defines the MetaWeblog class.

  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
*/

namespace KBlog {

    class MetaWeblogPrivate;
/**
  @brief
  A class that can be used for access to MetaWeblog  blogs. Almost every
  blog server supports MetaWeblog. Compared to Blogger 1.0 it is a
  superset of functions added to the its definition. MetaWeblog  is much
  more functional, but has some drawbacks, e.g. security when compared to
  GData which is based on Atom API and is quite new.

  @code
  Blog* myblog = new MetaWeblog("http://example.com/xmlrpc/gateway.php");
  KBlog::BlogPosting *post = new BlogPosting();
  post->setUsername( "some_user_id" );
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPosting( posting );
  @endcode

  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
*/
class KBLOG_EXPORT MetaWeblog : public Blogger1
{
  Q_OBJECT
  public:
    /**
      Create an object for MetaWeblog

      @param server is the url for the xmlrpc gateway.
      @param parent is the parent object.
    */
    explicit MetaWeblog( const KUrl &server, QObject *parent = 0 );

    /**
      Destroy the object.
    */
    virtual ~MetaWeblog();

    /**
      Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
      List recent postings on the server.

      @param number The number of postings to fetch. Latest first.

      @see     void listedRecentPostings( QList\<KBlog::BlogPosting\>& )
    */
    void listRecentPostings( int number );

    /**
      List the categories of the blog.
      @see listedCategories( const QList\<QMap\<QString,QString\> \>& )
    */
    virtual void listCategories();

    /**
      Fetch the Posting with postingId.
      @param posting is the posting with its id set to get the corresponding
      posting. 

      @see BlogPosting::setPostingId( const QString& )
      @see fetchedPosting( KBlog::BlogPosting *posting )
    */
    void fetchPosting( KBlog::BlogPosting *posting );

    /**
      Modify a posting on server.

      @param posting is used to send the modified posting including the
      correct postingId from it to the server.

      @see modifiedPosting( KBlog::BlogPosting *posting )
    */
    void modifyPosting( KBlog::BlogPosting *posting );

    /**
      Create a new posting on server.

      @param posting This posting to send.

      @see createdPosting( KBlog::BlogPosting *posting )
    */
    void createPosting( KBlog::BlogPosting *posting );

    /**
      Create a new media object, e.g. picture, on server.

      @param media The media to send.
    */
    virtual void createMedia( KBlog::BlogMedia *media );

  Q_SIGNALS:

    /**
      This signal is emitted when a media has been created 
      on the server.

      @param media The created media.

      @see createMedia( KBlog::BlogMedia *media )
    */
    void createdMedia( KBlog::BlogMedia *media );

    /**
      This signal is emitted when the last category of the listCategories()
      job has been fetched.

      @param categories This list contains the categories. Each map has the keys: 
      name, description, htmlUrl, rssUrl.

      @see listCategories()
    */
    void listedCategories( const QList<QMap<QString,QString> >& categories );

  protected:
    /**
      Constructor needed for private inheritance.
    */
    MetaWeblog( const KUrl &server, MetaWeblogPrivate &dd, QObject *parent = 0 );

  private:
    Q_DECLARE_PRIVATE(MetaWeblog)
    Q_PRIVATE_SLOT(d_func(), void slotListCategories( 
    const QList<QVariant>&, const QVariant& ))
};

} //namespace KBlog
#endif
