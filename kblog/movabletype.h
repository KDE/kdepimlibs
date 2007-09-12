/*
  This file is part of the kblog library.

  Copyright (c) 2007 Christian Weilbach <christian_weilbach@web.de>
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

#ifndef KBLOG_MOVABLETYPE_H
#define KBLOG_MOVABLETYPE_H

#include <kblog/metaweblog.h>

class KUrl;

/**
  @file
  This file is part of the  for accessing Blog Servers
  and defines the MovableType class.

  @author Christian Weilbach \<christian_weilbach\@web.de\>
  @author Mike Arthur \<mike\@mikearthur.co.uk\>
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
  KBlog::BlogPost *post = new BlogPost();
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPosting( posting );
  @endcode

  @author Christian Weilbach \<christian_weilbach\@web.de\>
  @author Mike Arthur \<mike\@mikearthur.co.uk\>
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
    explicit MovableType( const KUrl &server, QObject *parent = 0 );

    /**
      Destroy the object.
    */
    virtual ~MovableType();

    /**
      Create a new posting on server.

      @param posting is send to the server.
    */
    void createPosting( KBlog::BlogPost *posting );

    /**
      Fetch the Posting with postingId.
      @param posting This is the posting with its id set to the
      corresponding posting on the server.

      @see BlogPost::setPostingId( const QString& )
      @see fetchedPosting( KBlog::BlogPost* )
    */
    void fetchPosting( KBlog::BlogPost *posting );

    /**
      Modify a posting on server.

      @param posting The posting to be modified on the
      server. You need to set its id correctly.

      @see BlogPost::setPostingId( const QString& )
      @see modifiedPosting( KBlog::BlogPost* )
    */
    void modifyPosting( KBlog::BlogPost *posting );

    /**
      Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
      List recent postings on the server.

     @param number The number of postings to fetch. Latest first.

      @see     void listedRecentPostings( const QList\<KBlog::BlogPost\>& )
    */
    void listRecentPostings( int number );

    /**
      Get the list of trackback pings from the server.

      @param posting This is the posting to get the trackback pings from.
      You need to set its id correctly.

      @see BlogPost::setPostingId( const QString& )
      @see listedTrackBackPings( KBlog::BlogPost *, const QList\<QMap\<QString,QString\> \>& )

    */
    virtual void listTrackBackPings( KBlog::BlogPost *posting );

  Q_SIGNALS:
    /**
      This signal is emitted when the trackback pings are fetched completely.

      @param posting This is the posting of the trackback ping list.
      @param pings This is the list itself. The map contains the keys: id, url, ip.

      @see listTrackBackPings()
    */
    void listedTrackBackPings( KBlog::BlogPost *posting, const QList<QMap<QString,QString> > &pings );

  protected:
    /**
      Constructor needed for private inheritance.
    */
    MovableType( const KUrl &server, MovableTypePrivate &dd, QObject *parent = 0 );

  private:
    Q_DECLARE_PRIVATE(MovableType)
    Q_PRIVATE_SLOT( d_func(), void slotListTrackBackPings(
    const QList<QVariant>&, const QVariant& ))
};

} //namespace KBlog
#endif
