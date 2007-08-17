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
  post->setPrivate( true ); // false on default
  connect( backend, createdPosting( KBlog::BlogPosting* ),
                 this, createdPosting( KBlog::BlogPosting* );
  backend->createPosting( post );
  ...
  void createdPosting( KBlog::BlogPosting* post )
  {
    setMyFancyGUIPostingId( post->postingId() );
    setMyFancyGUIPermaLink( post->permaLink() );
  }
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
    */
    explicit BlogPosting( const QString &postingId = QString() );

    /** Constructor to create a blog posting from a KCal Journal.

      @param journal The journal to use to create the posting
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

//     QString abbreviatedContent() const; // TODO check if necessary
//     void setAbbreviatedContent( const QString &abbreviatedContent );

    /**
      Returns the link path.

      @return link
      @see setLink()
    */
    KUrl link() const;

    /**
      Set the link path.

      @param link The path to set.
      @see link()
    */
    void setLink( const KUrl &link ) const;

    /**
      Returns the perma link path.

      @return permaLink
      @see setPermaLink()
    */
    KUrl permaLink() const;

    /**
      Set the perma link path.

      @param permalink The path to set.
      @see permaLink()
    */
    void setPermaLink( const KUrl &permalink ) const;

    /**
      Returns whether comments should be allowed.

      @return commentAllowed
      @see setCommentAllowed()
    */
    bool isCommentAllowed() const;

    /**
      Set whether comments should be allowed.

      @param commentAllowed
      @see isCommentAllowed()
    */
    void setCommentAllowed( bool commentAllowed );

    /**
      Returns whether track back should be allowed.

      @return trackBackAllowed
      @see setTrackBackAllowed()
    */
    bool isTrackBackAllowed() const; // pings in Movable Type

    /**
      Set whether track back should be allowed.

      @param allowTrackBacks
      @see isTrackBackAllowed()
    */ 
   void setTrackBackAllowed ( bool allowTrackBacks );

    /**
      Returns the summary.

      @return summary
      @see setSummary()
    */
    QString summary() const; // excerpts in Movable Type

    /**
      Set the summary.

      @param summary
      @see summary()
    */
    void setSummary( const QString &summary );

    /**
      Returns the tags list as a QStringList.

      @return tags list
      @see setTags()
    */
    QStringList tags() const; // keywords in Movable Type

    /**
      Set the tags list.

      @param tags The tags list.
      @see tags()
    */
    void setTags( const QStringList &tags );

//     QList<KUrl> trackBackUrls() const; // TODO check if necessary
//     void setTrackBackUrls( const QList<KUrl> &trackBackUrls );

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

    /**
      The enumartion of the different posting status, reflecting the status changes
      on the server.
    */
    enum Status { 
      /** Status of a freshly constructed posting on the client. */
      New, 
      /** Status of a successfully fetched posting. 
      @see Blog::fetchPosting( KBlog::BlogPosting* ) */
      Fetched, 
      /** Status of a successfully created posting. 
      @see Blog::createPosting( KBlog::BlogPosting* ) */
      Created, 
      /** Status of a successfully modified posting. 
      @see Blog::modifyPosting( KBlog::BlogPosting* ) */
      Modified, 
      /** Status of a successfully removed posting. 
      @see Blog::removePosting( KBlog::BlogPosting* ) */
      Removed, 
      /** Status when an error on the server side occured. 
      @see error() */
      Error 
    };

    /**
      Returns the status on the server.

      @return status
      @see setStatus(), Status
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

      @returns error
      @see setError(), Error
    */
    QString error() const;

    /**
      Sets the error.

      @param error The error string.
      @see error(), Error
    */
    void setError( const QString& error );

    /**
      The overloaed = operator.
    */
    BlogPosting& operator=( const BlogPosting &posting );
  protected:
    /**
      Constructor needed for private inheritance.
    */
    BlogPosting( const QString &postingId, BlogPostingPrivate &dd );
    /**
      Constructor needed for private inheritance.
    */
    BlogPosting( const KCal::Journal &journal, BlogPostingPrivate &dd );
  private:
    BlogPostingPrivate * const d_ptr;
};


} //namespace KBlog

#endif
