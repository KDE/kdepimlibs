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

#ifndef KBLOG_METAWEBLOG_H
#define KBLOG_METAWEBLOG_H

#include <blogger1.h>

class QUrl;

/**
  @file
  This file is part of the  for accessing Blog Servers
  and defines the MetaWeblog class.

  @author Reinhold Kainhofer \<reinhold\@kainhofer.com\>
  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
*/

namespace KBlog
{

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
  myblog->setUsername( "some_user_id" );
  myblog->setPassword( "YouRFuNNYPasSwoRD" );
  myblog->setBlogId( "1" ); // can be caught by listBlogs()
  KBlog::BlogPost *post = new BlogPost();
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPost( post );
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
    explicit MetaWeblog(const QUrl &server, QObject *parent = Q_NULLPTR);

    /**
      Destroy the object.
    */
    virtual ~MetaWeblog();

    /**
      Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
      List the categories of the blog.

      @see listedCategories( const QList\<QMap\<QString,QString\> \>& )
    */
    virtual void listCategories();

    /**
      Create a new media object, e.g. picture, on server.

      @param media The media to send.
    */
    virtual void createMedia(KBlog::BlogMedia *media);

Q_SIGNALS:

    /**
      This signal is emitted when a media has been created
      on the server.

      @param media The created media.

      @see createMedia( KBlog::BlogMedia *media )
    */
    void createdMedia(KBlog::BlogMedia *media);

    /**
      This signal is emitted when the last category of the listCategories()
      job has been fetched.

      @param categories This list contains the categories. Each map has the keys:
      name, description, htmlUrl, rssUrl.

      @see listCategories()
    */
    void listedCategories(const QList<QMap<QString, QString> > &categories);

protected:
    /**
      Constructor needed for private inheritance.
    */
    MetaWeblog(const QUrl &server, MetaWeblogPrivate &dd, QObject *parent = Q_NULLPTR);

private:
    Q_DECLARE_PRIVATE(MetaWeblog)
    Q_PRIVATE_SLOT(d_func(),
                   void slotListCategories(const QList<QVariant> &, const QVariant &))
    Q_PRIVATE_SLOT(d_func(),
                   void slotCreateMedia(const QList<QVariant> &, const QVariant &))
};

} //namespace KBlog
#endif
