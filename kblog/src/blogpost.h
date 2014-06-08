/*
  This file is part of the kblog library.

  Copyright (c) 2007 Christian Weilbach <christian_weilbach@web.de>
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

#ifndef KBLOG_BLOGPOSTING_H
#define KBLOG_BLOGPOSTING_H

#include <kblog_export.h>

#include <QUrl>
#include <kcalcore/journal.h>
#include <QtCore/QtAlgorithms>

class QStringList;

class KDateTime;
class QUrl;


namespace KBlog {
  class Blog;
  class BlogPostPrivate;

/**
  @brief
  A class that represents a blog post on the server.

  @code
  KBlog::BlogPost *post = new BlogPost();
  post->setTitle( "This is the title." );
  post->setContent( "Here is some the content..." );
  post->setPrivate( true ); // false on default
  connect( backend, createdPost( KBlog::BlogPost* ),
                 this, createdPost( KBlog::BlogPost* );
  backend->createPost( post );
  ...
  void createdPost( KBlog::BlogPost* post )
  {
    setMyFancyGUIPostId( post->postId() );
    setMyFancyGUIPermaLink( post->permaLink() );
  }
  @endcode

  @author Christian Weilbach \<christian_weilbach\@web.de\>
*/

class KBLOG_EXPORT BlogPost
{

public:

    /**
      Constructor.
    */
    BlogPost( const KBlog::BlogPost &post );

    /**
      Constructor.
      @param postId The ID of the post on the server.
    */
    explicit BlogPost( const QString &postId = QString() );

    /** Constructor to create a blog post from a KCalCore Journal.
      @param journal The journal to use to create the post. Must not be null.
     */
    explicit BlogPost( const KCalCore::Journal::Ptr &journal );

    /**
      Virtual default destructor.
    */
    virtual ~BlogPost();

    /**
      Returns a KCalCore journal from the blog post owned by the caller.
      @param blog The blog object to convert.
      @return journal
     */
    KCalCore::Journal::Ptr journal( const Blog &blog ) const;

    /**
      Returns the ID used by the journal in creation, if created from a journal.
      @return journal ID
    */
    QString journalId() const;

    /**
      Returns if the post is published or not.
      @return bool

      @see setPrivate()
    */
    bool isPrivate() const;

    /**
      Sets the post to private viewings only.
      @param privatePost set this to false, if you don't want to publish
      the blog post immediately.

      @see isPrivate()
    */
    void setPrivate( bool privatePost );

    /**
      Returns the postId. This is for fetched posts.
      @return postId

      @see setPostId()
    */
    QString postId() const;

    /**
      Sets the post id value. This is important for modifying posts.
      @param postId set this to the post id on the server.

      @see postId()
    */
    void setPostId( const QString &postId );

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
    Returns the additional content, (mt_text_more of MovableType API)
     @return additional content

     @see setAdditionalContent()
    */
    QString additionalContent() const;

    /**
     Sets the additional content, (mt_text_more of MovableType API)
     @param additionalContent set the additional content

     @see additionalContent()
    */
    void setAdditionalContent( const QString &additionalContent );

    /**
     Returns the Wordpress posts Slug (or permalink will use for post)
     Currently just wordpress supports this!
     @return wordpress slug

     @see setSlug()
    */
    QString slug() const;

    /**
     Sets the Wordpress slug property! (will use to set post's permalink)
     Currently just wordpress supports this!
     @param slug wordpress slug

     @see slug()
    */
    void setSlug( const QString &slug );
    /**
      Returns the link path.
      @return link

      @see setLink()
    */
    QUrl link() const;

    /**
      Set the link path.
      @param link The path to set.

      @see link()
    */
    void setLink( const QUrl &link ) const;

    /**
      Returns the perma link path.
      @return permaLink

      @see setPermaLink()
    */
    QUrl permaLink() const;

    /**
      Set the perma link path.
      @param permalink The path to set.

      @see permaLink()
    */
    void setPermaLink( const QUrl &permalink ) const;

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

    /**
      Returns the mood.
      @return mood

      @see setMood()
    */
    QString mood() const;

    /**
      Set the mood list.
      @param mood The mood.

      @see mood()
    */
    void setMood( const QString &mood );

    /**
      Returns the music.
      @return music

      @see setMusic()
    */
    QString music() const;

    /**
      Set the music.
      @param music The music.

      @see music()
    */
    void setMusic( const QString &music );

    /**
      Returns the categories.
      @return categories

      @see setCategories()
    */
    QStringList categories() const;

    /**
      Sets the categories. The first one is used as the primary
      category if possible.
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
      Sets the creation time. This is used by most Blogs and is generally
      the shown date. Set it if you want to change the shown date.
      @param datetime set the time the post has been created.

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
      @param datetime set the time the post has been modified.

      @see modificationTime(), setCreationDateTime()
    */
    void setModificationDateTime( const KDateTime &datetime );

    /**
      The enumartion of the different post status, reflecting the status changes
      on the server.
    */
    enum Status {
      /** Status of a freshly constructed post on the client. */
      New,
      /** Status of a successfully fetched post.
      @see Blog::fetchPost( KBlog::BlogPost* ) */
      Fetched,
      /** Status of a successfully created post.
      @see Blog::createPost( KBlog::BlogPost* ) */
      Created,
      /** Status of a successfully modified post.
      @see Blog::modifyPost( KBlog::BlogPost* ) */
      Modified,
      /** Status of a successfully removed post.
      @see Blog::removePost( KBlog::BlogPost* ) */
      Removed,
      /** Status when an error on the server side occurred.
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
    void setError( const QString &error );

    /**
      The overloaed = operator.
    */
    BlogPost &operator=( const BlogPost &post );

    /**
      The swap operator.
    */
    void swap( BlogPost &other ) {
        qSwap( this->d_ptr, other.d_ptr );
    }

  private:
    BlogPostPrivate *d_ptr;  //krazy:exclude=dpointer can't constify due to bic and swap being declared inline
};

} //namespace KBlog

#endif
