/*
  This file is part of the kblog library.

  Copyright (c) 2007-2009 Christian Weilbach <christian_weilbach@web.de>
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

#ifndef KBLOG_MOVABLETYPE_H
#define KBLOG_MOVABLETYPE_H

#include <metaweblog.h>

class QUrl;

/**
  @file
  This file is part of the  for accessing Blog Servers
  and defines the MovableType class.

  @author Christian Weilbach \<christian_weilbach\@web.de\>
  @author Mike McQuaid \<mike\@mikemcquaid.com\>
*/

namespace KBlog {

    class MovableTypePrivate;
/**
  @brief
  A class that can be used for access to Movable Type blogs. Almost every
  blog server supports Movable Type.
  @code
  Blog* myblog = new MovableType("http://example.com/xmlrpc/gateway.php");
  myblog->setUsername( "some_user_id" );
  myblog->setPassword( "YoURFunnyPAsSwoRD" );
  myblog->setBlogId( "1" ); // can be caught by listBlogs()
  KBlog::BlogPost *post = new BlogPost();
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPost( post );
  @endcode

  @author Christian Weilbach \<christian_weilbach\@web.de\>
  @author Mike McQuaid \<mike\@mikemcquaid.com\>
*/
class KBLOG_EXPORT MovableType : public MetaWeblog
{
  Q_OBJECT
  public:
    /**
      Create an object for Movable Type

      @param server is the url for the xmlrpc gateway.
      @param parent is the parent object.
    */
    explicit MovableType( const QUrl &server, QObject *parent = 0 );

    /**
      Destroy the object.
    */
    virtual ~MovableType();

    /**
      Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
      List recent posts on the server. The status of the posts will be Fetched.

     @param number The number of posts to fetch. Latest first.

      @see     void listedRecentPosts( const QList\<KBlog::BlogPost\>& )
      @see     BlogPost::Status
    */
    void listRecentPosts( int number );

    /**
      Get the list of trackback pings from the server.

      @param post This is the post to get the trackback pings from.
      You need to set its id correctly.

      @see BlogPost::setPostId( const QString& )
      @see listedTrackBackPings( KBlog::BlogPost *, const QList\<QMap\<QString,QString\> \>& )

    */
    virtual void listTrackBackPings( KBlog::BlogPost *post );

    void createPost( KBlog::BlogPost *post );

    void modifyPost( KBlog::BlogPost *post );

    void fetchPost( KBlog::BlogPost *post );

  Q_SIGNALS:
    /**
      This signal is emitted when the trackback pings are fetched completely.

      @param post This is the post of the trackback ping list.
      @param pings This is the list itself. The map contains the keys: id, url, ip.

      @see listTrackBackPings()
    */
    void listedTrackBackPings( KBlog::BlogPost *post, const QList<QMap<QString,QString> > &pings );

  protected:
    /**
      Constructor needed for private inheritance.
    */
    MovableType( const QUrl &server, MovableTypePrivate &dd, QObject *parent = 0 );

  private:
    Q_DECLARE_PRIVATE( MovableType )
    Q_PRIVATE_SLOT( d_func(),
                    void slotListTrackBackPings( const QList<QVariant> &, const QVariant & ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotCreatePost( const QList<QVariant> &, const QVariant & ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotModifyPost( const QList<QVariant> &, const QVariant & ) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotGetPostCategories(const QList<QVariant>&,const QVariant&) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotSetPostCategories(const QList<QVariant>&,const QVariant&) )
    Q_PRIVATE_SLOT( d_func(),
                    void slotTriggerCreatePost() )
    Q_PRIVATE_SLOT( d_func(),
                    void slotTriggerModifyPost() )
    Q_PRIVATE_SLOT( d_func(),
                    void slotTriggerFetchPost() )
};

} //namespace KBlog
#endif
