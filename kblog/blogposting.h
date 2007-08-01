/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006 Christian Weilbach <christian_weilbach@web.de>
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

#ifndef KBLOG_BLOGPOSTING_H
#define KBLOG_BLOGPOSTING_H

#include <kblog/kblog_export.h>

#include <QtCore/QObject>

class QStringList;

class KDateTime;
class KUrl;

namespace KBlog {

  class BlogPostingPrivate;
  class BlogPostingComment;

/**
  @brief
  A class that represents a blog posting on the server.

  @code
  KBlog::BlogPosting *post = new BlogPosting();
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  @endcode

  @author Christian Weilbach \<christian\@whiletaker.homeip.net\>
*/

class KBLOG_EXPORT BlogPosting : public QObject
{
  Q_OBJECT
public:
    /**
      Constructor.

      @param postingId The ID of the posting on the server.
      @param parent Parent object of this BlogPosting
    */
    explicit BlogPosting( const QString &postingId = QString(),
                          QObject *parent = 0 );

    /**
      Virtual default destructor.
    */
    virtual ~BlogPosting();

    /**
      Returns if the posting is published or not.

      @return bool
      @see setPublish()
    */
    bool isPublished() const;

    /**
      Sets the publish value.

      @param publish set this to true, if you want to publish immediately.
      @see publish()
    */
    void setPublished( bool published );

    /**
      Returns the postId. This is for fetched postings.
      @return postingId
      @see setPostingId()
    */
    QString postingId() const;

    /**
      Sets the post id value. This is important for modifying postings.

      @param postingId set this to the post id on the server.
      @see postingId()
    */
    void setPostingId( const QString &postingId );

    /**
      Returns the title.

      @return title
      @see setTitle()
    */
    QString title() const;

    /**
      Sets the title.

      @param title set the title.
      @see title()
    */
    void setTitle( const QString &title );

    /**
      Returns the content.

      @return content
      @see setContent()
    */
    QString content() const;

    /**
      Sets the content.

      @param content set the content.
      @see content()
    */
    void setContent( const QString &content );

    QString abbreviatedContent() const;
    void setAbbreviatedContent( const QString &abbreviatedContent );

    KUrl link() const;
    void setLink( const KUrl &link ) const;

    KUrl permalink() const;
    void setPermalink( const KUrl &permalink ) const;

    bool isCommentAllowed() const;
    void setCommentAllowed( bool commentAllowed );

    bool isTrackBackAllowed() const; // pings in Movable Type
    void setTrackBackAllowed ( bool allowTrackBacks );

    QString summary() const; // excerpts in Movable Type
    void setSummary( const QString &summary );

    QString tags() const; // keywords in Movable Type
    void setTags( const QString &tags );

    QList<KUrl> trackBackUrls() const;
    void setTrackBackUrls( const QList<KUrl> &trackBackUrls );

    QString mood() const;
    void setMood( const QString &mood );

    QString music() const;
    void setMusic( const QString &music );

    /**
      Returns the categories.

      @return categories
      @see setCategories()
    */
    QStringList categories() const;

    /**
      Sets the categories.

      @param categories set the categories.
      @see categories()
    */
    void setCategories( const QStringList &categories );

    /**
      Returns the creation date time.

      @return creationdatetime
      @see setCreationDateTime()
    */
    KDateTime creationDateTime() const;

    /**
      Sets the creation time.

      @param datetime set the time the posting has been created.
      @see creationTime()
    */
    void setCreationDateTime( const KDateTime &datetime );

    /**
      Returns the modification date time.

      @return modificationdatetime
      @see setModificationDateTime(), creationDateTime()
    */
    KDateTime modificationDateTime() const;

    /**
      Sets the modification time.

      @param datetime set the time the posting has been modified.
      @see modificationTime(), setCreationDateTime()
    */
    void setModificationDateTime( const KDateTime &datetime );

    enum Status { New, Fetched, Created, Modified, Removed, Error };

    Status status() const;

    void setStatus( Status status );

    QString error() const;

    void setError( const QString& error );

  Q_SIGNALS:
    void statusChanged( KBlog::BlogPosting::Status status );

    void listedComments( const QList<KBlog::BlogPostingComment*> &comments );

  protected:
    BlogPostingPrivate * const d_ptr;
    BlogPosting( const KUrl &server, BlogPostingPrivate &dd, QObject *parent = 0 );

  private:
    Q_DECLARE_PRIVATE(BlogPosting)
};


} //namespace KBlog

#endif
