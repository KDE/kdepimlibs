/*
  This file is part of the kblog library.

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

#ifndef KBLOG_BLOGMEDIA_H
#define KBLOG_BLOGMEDIA_H

#include <kblog_export.h>

#include <QtCore/QtAlgorithms>

class QUrl;

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

    /**
      Copy Constructor needed for list handling.
    */
    BlogMedia( const BlogMedia &media );

    /**
      Virtual default destructor.
    */
    virtual ~BlogMedia();

    /**
      Returns the wished name. This is most likely the filename on the server side
      (at least with wordpress).

      @return The wished name on the server.
      @see setName( const QString& )
    */
    QString name() const;

    /**
      Sets the name. This will most likely be the filename on the server side
      (at least with wordpress).
      @param name The whished name for the object.

      @see name()
    */
    void setName( const QString &name );

    /**
      Returns the server side url.

      @return The url on the server.
      @see setUrl( const KUrl& )
    */
    QUrl url() const;

    /**
      Sets the url of the server side object. Note: You should *not* set this
      on your own normally. It is used internally in MetaWeblog.
      @param url The whished name for the object.

      @see url()
    */
    void setUrl( const QUrl &url );

    /**
      Returns the mimetype.

      @return The mimetype of the object
      @see setMimetype( const QString& )
    */
    QString mimetype() const;

    /**
      Set the mimtype.
      @param mimetype This is the mimetype.

      @see mimetype()
    */
    void setMimetype( const QString &mimetype );

    /**
      Returns the data of the file.
      @return The data.

      @see setData( const QByteArray& )
    */
    QByteArray data() const;

    /**
       Set the data of the file.
       @param data This is the data stream.

       @see data()
    */
    void setData( const QByteArray &data );

    /**
       The different possible status. At the moment you cannot do
       much with media objects.
    */
    enum Status {
       /** Status of freshly constructed media object on the client side. */
       New,
       /** Status of a media object successfully created on the server. */
       Created,
       /** Status when an error on uploading has occurred. */
       Error
    };

    /**
       Returns the status.
       @return This is the status.

       @see setStatus( Status )
    */
    Status status() const;

    /**
       Set the status. Note: You should *not* set this on your own
       it is used mostly internally.
       @param status This is the status.

       @see status()
    */
    void setStatus( Status status );

    /**
       Returns the error string.
       @return The error string.
       @see setError( const QString& )
    */
    QString error() const;

    /**
       Set the error of the object.
       @param error This is the error string.

       @see error()
    */
    void setError( const QString &error );

    /**
       Overloaded for QList handling.
       @param media The media file to copy.
    */
    BlogMedia &operator=( const BlogMedia &media );

    /**
      The swap operator.
    */
    void swap( BlogMedia &other ) {
        qSwap( this->d_ptr, other.d_ptr );
    }

  private:
    BlogMediaPrivate *d_ptr; //krazy:exclude=dpointer can't constify due to bic and swap being declared inline
};

} //namespace KBlog

#endif
