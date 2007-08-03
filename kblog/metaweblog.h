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
  post->setUserId( "some_user_id" );
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
      @see     void listedPosting( KBlog::BlogPosting &posting )

      @see     void listRecentPostingsFinished()
    */
    void listRecentPostings( int number );

    /**
      List the categories of the blog.
      @see  void categoryInfoRetrieved( const QString &, const QString & )
      @see  void listCategoriesFinished()
    */
    virtual void listCategories();

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
      Create a new media object, e.g. picture, on server.

      @param media is send to the server.
    */
    virtual void createMedia( KBlog::BlogMedia *media );

  Q_SIGNALS:

    void createdMedia( KBlog::BlogMedia *media );

    /**
      This signal is emitted when the last category of the listCategories()
      job has been fetched.

      @see listCategories()
    */
    void listedCategories( const QMap<QString,QMap<QString,QString> >& categories );

  protected:
    MetaWeblog( const KUrl &server, MetaWeblogPrivate &dd, QObject *parent = 0 );

  private:
    Q_DECLARE_PRIVATE(MetaWeblog)
    Q_PRIVATE_SLOT(d_func(), void slotListCategories( 
    const QList<QVariant>&, const QVariant& ))
};

} //namespace KBlog
#endif
