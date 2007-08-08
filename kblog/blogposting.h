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

#ifndef KBLOG_BLOGPOSTING_H
#define KBLOG_BLOGPOSTING_H

#include <kblog/kblog_export.h>

#include <kurl.h>

class QStringList;

class KDateTime;
class KUrl;

namespace KCal {
  class Journal;
}

namespace KBlog {
  class Blog;
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

  @author Christian Weilbach \<christian_weilbach\@web.de\>
*/

class KBLOG_EXPORT BlogPosting
{

public:

    /**
      Constructor.
    */
    BlogPosting( const KBlog::BlogPosting& posting );

    /**
      Constructor.

      @param postingId The ID of the posting on the server.
      @param parent Parent object of this BlogPosting
    */
    explicit BlogPosting( const QString &postingId = QString() );

    /** Constructor to create a blog posting from a KCal Journal.

      @param journal The journal to use to create the posting
      @param parent Parent object of this BlogPosting
     */
    explicit BlogPosting( const KCal::Journal &journal );

    /**
      Virtual default destructor.
    */
    virtual ~BlogPosting();

    /**
      Returns a KCal journal from the blog posting owned by the caller.

      @return journal
     */
    KCal::Journal* journal( const Blog &blog ) const;

    /**
      Returns the ID used by the journal in creation, if created from a journal.

      @return journal ID
    */
    QString journalId() const;

    /**
      Returns if the posting is published or not.

      @return bool
      @see setPrivate()
    */
    bool isPrivate() const;

    /**
      Sets the posting to private viewings only.

      @param privatePosting set this to false, if you don't want to publish
      the blog posting immediately.
      @see isPrivate()
    */
    void setPrivate( bool privatePosting );

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

    KUrl permaLink() const;
    void setPermaLink( const KUrl &permalink ) const;

    bool isCommentAllowed() const;
    void setCommentAllowed( bool commentAllowed );

    bool isTrackBackAllowed() const; // pings in Movable Type
    void setTrackBackAllowed ( bool allowTrackBacks );

    QString summary() const; // excerpts in Movable Type
    void setSummary( const QString &summary );

    QStringList tags() const; // keywords in Movable Type
    void setTags( const QStringList &tags );

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

    BlogPosting& operator=( const BlogPosting &posting );

  protected:

    BlogPosting( const QString &postingId, BlogPostingPrivate &dd );
    BlogPosting( const KCal::Journal &journal, BlogPostingPrivate &dd );
  private:
    BlogPostingPrivate * const d_ptr;
};


} //namespace KBlog

#endif
