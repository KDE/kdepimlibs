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

#ifndef KBLOG_BLOGCOMMENT_H
#define KBLOG_BLOGCOMMENT_H

#include <kblog/kblog_export.h>

#include <QtCore/QString>
#include <QtCore/QtAlgorithms>

class KDateTime;
class KUrl;

namespace KBlog {

    class BlogCommentPrivate;
/**
  @brief
  A class that represents a blog comment on the blog post.

  @code
  KBlog::BlogComment *comment = new BlogComment();
  comment->setTitle( "This is the title." );
  comment->setContent( "Here is some the content..." );
  @endcode

  @author Mike Arthur \<mike\@mikearthur.co.uk\>
*/

class KBLOG_EXPORT BlogComment
{
  public:
  /**
    Copy Constructor for list handling.
    @param comment The comment to copy.
  */
  BlogComment( const BlogComment &comment );

   /**
    Constructor.
    @param commentId The ID of the comment on the server.
  */
  explicit BlogComment( const QString &commentId = QString() );

  /**
    Virtual default destructor.
  */
  virtual ~BlogComment();

  /**
    Returns the title.
    @return The title.

    @see setTitle( const QString& )
  */
  QString title() const;

  /**
    Sets the title.
    @param title This is the title.

    @see title()
  */
  void setTitle( const QString &title );

  /**
    Returns the content.
    @return The content.

    @see setContent( const QString& )
  */
  QString content() const;

  /**
    Sets the content.
    @param content This is the content.

    @see content()
  */
  void setContent( const QString &content );

  /**
    Returns the comment's id.
    @return The comment's id

    @see setCommentId( const QString& )
  */
  QString commentId() const;

  /**
    Sets the comment's id.
    @param id The comment's id.

    @see commentId()
  */
  void setCommentId( const QString &id );

  /**
    Returns the E-Mail address of the commentator.
    @return The E-Mail.

    @see setEmail( const QString& )
  */
  QString email() const;

  /**
    Sets the E-Mail.
    @param email This is the E-Mail address of the commentator.

    @see email()
  */
  void setEmail( const QString &email );

  /**
    Returns the commentator's name.
    @return The name.

    @see setName()
  */
  QString name() const;

  /**
    Sets the name of the commentator.
    @param name This is the commenator's name.

    @see name()
  */
  void setName( const QString &name );

  /**
    Returns the commentator's homepage URL.
    @return The url of the commentator's homepage

    @see setUrl( const KUrl& )
  */
  KUrl url() const;

  /**
    Sets the commentator's homepage URL.
    @param url The commentator's homepage url.

    @see url()
  */
  void setUrl( const KUrl &url );

  /**
    Returns the modification date-time.
    @return The modification date-time.

    @see setModificationDateTime( const KDateTime& )
  */
  KDateTime modificationDateTime() const;

  /**
    Sets the modification date-time.
    @param datetime The date-time the comment has been modified.

    @see modificationDateTime( const KDateTime& )
  */
  void setModificationDateTime( const KDateTime &datetime );

  /**
    Returns the creation date-time.
    @return The creation date-time.

    @see setCreationDateTime( const KDateTime& )
  */
  KDateTime creationDateTime() const;

  /**
    Sets the creation date-time.
    @param datetime The date-time the comment has been created.

    @see creationDateTime()
  */
  void setCreationDateTime( const KDateTime &datetime );

  /**
    The enumartion of the different post status, reflecting the status changes
    on the server.
  */
  enum Status {
    /** Status of a freshly constructed comment on the client. */
    New,
    /** Status of a successfully fetched comment. */
    Fetched,
    /** Status of a successfully created comment.
    @see GData::createComment( BlogPost*, BlogComment* ) */
    Created,
    /** Status of a successfully removed comment.
    @see GData::removeComment( BlogPost*, BlogComment* ) */
    Removed,
    /** Status when an error has occurred on the server side.
    @see error() */
    Error
  };

  /**
    Returns the status on the server.
    @return The status.

    @see setStatus( Status ), Status
  */
  Status status() const;

  /**
    Sets the status.
    @param status The status on the server.

    @see status(), Status
  */
  void setStatus( Status status );

  /**
    Returns the last error.
    @returns The last error string.

    @see setError( const QString& ), Error
  */
  QString error() const;

  /**
    Sets the error.
    @param error The error string.

    @see error(), Error
  */
  void setError( const QString &error );

  /**
    Overloaded for QList handling.
  */
  BlogComment &operator=( const BlogComment &comment );

  /**
    The swap operator.
  */
  void swap( BlogComment &other ) { qSwap( this->d_ptr, other.d_ptr ); }

  private:
    BlogCommentPrivate *d_ptr; //krazy:exclude=dpointer can't constify due to bic and swap being declared inline
};

} //namespace KBlog

#endif
