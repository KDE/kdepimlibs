/*
  This file is part of the kblog library.

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
  KBlog::BlogPosting *post = new BlogPosting();
  post->setUserId( "some_user_id" );
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  myblog->createPosting( posting );
  @endcode

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
    ~MovableType();

    /**
      Create a new posting on server.

      @param posting is send to the server.
    */
    void createPosting( KBlog::BlogPosting *posting );

    /**
      Fetch the Posting with postingId.
      @param postingId is the id of the posting on the server.

      @see  void fetchedPosting( KBlog::BlogPosting &posting )
    */
    void fetchPosting( KBlog::BlogPosting *posting );

    /**
      Returns the  of the inherited object.
    */
    QString interfaceName() const;

    /**
      List recent postings on the server.
      @see     void listedPosting( KBlog::BlogPosting &posting )

      @see     void listRecentPostingsFinished()
    */
    virtual void listRecentPostings( int number );

    /**
      TODO
      @param postingId is the id of the posting on the server.

      @see  void fetchedPosting( KBlog::BlogPosting &posting )
    */
    virtual void listTrackbackPings( KBlog::BlogPosting *posting );

    /**
      Modify a posting on server.

      @param posting is used to send the modified posting including the
      correct postingId from it to the server.
    */
    void modifyPosting( KBlog::BlogPosting *posting );

    /**
      Set the Url of the server.

      @param server is the server url.
    */
    void setUrl( const KUrl &server );

  Q_SIGNALS:
    /**
      TODO

      @see listTrackbackPings()
    */
    void listedTrackbackPings( const QMap<QString, QString> &pings );

  protected:
    MovableType( const KUrl &server, MovableTypePrivate &dd, QObject *parent = 0 );

  private:
    Q_DECLARE_PRIVATE(MovableType)
    Q_PRIVATE_SLOT( d_func(), void slotListTrackbackPings(
    const QList<QVariant>&, const QVariant& ))
};

} //namespace KBlog
#endif
