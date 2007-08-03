/*
  This file is part of the kblog library.

  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
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

#ifndef KBLOG_BLOGMEDIA_H
#define KBLOG_BLOGMEDIA_H

#include <kblog/kblog_export.h>

#include <QtCore/QObject>

class KUrl;

namespace KBlog {

  class BlogMediaPrivate;
/**
  @brief
  A class that represents a media object on the server.

  @code
  KBlog::BlogMedia *media = new BlogMedia();
  post->setMimetype( "some_mimetype" );
  post->setData( some_qbytestream );
  @endcode

  @author Christian Weilbach \<christian_weilbach\@web.de\>
*/

class KBLOG_EXPORT BlogMedia
{

  public:
    /**
      Default constructor. Creates an empty BlogMedia object.
    */
    explicit BlogMedia();

    BlogMedia( const BlogMedia& media );

    /**
      Virtual default destructor.
    */
    virtual ~BlogMedia();

    /**
      Returns the name. This is most likely the filename on the server side
      (at least with wordpress).

      @return name
      @see setName()
    */
    QString name() const;

    /**
      Sets the name. This will most likely be the filename on the server side
      (at least with wordpress).

      @param title set the name.
      @see name()
    */
    void setName( const QString &title );

    KUrl url() const;

    void setUrl( const KUrl &url );

    /**
      Returns the mimetype.

      @return mimetype of the object
      @see setMimetype()
    */
    QString mimetype() const;

    /**
      Set the mimtype.

      @param mimetype is the mimetype
      @see mimetype()
    */
    void setMimetype( const QString &mimetype );

    /**
      Returns the data of the file.

      @return data
      @see setData()
    */
    QByteArray data() const;

    /**
       Set the data of the file.

       @param data is the data stream
       @see data()
    */
    void setData( const QByteArray &data );

    enum Status { New, Fetched, Created, Modified, Removed, Error };

    Status status() const;

    void setStatus( Status status );

    QString error() const;

    void setError( const QString &error );

    BlogMedia& operator=(const BlogMedia &media );

  protected:
    BlogMediaPrivate * const d_ptr;
    BlogMedia( const KUrl &server, BlogMediaPrivate &dd );
};

} //namespace KBlog

#endif
