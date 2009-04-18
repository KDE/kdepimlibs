/*
    This file is part of the kblog library.

    Copyright (c) 2007 Christian Weilbach <christian_weilbach@web.de>

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

#include "gdata.h"
#include "gdata_p.h"
#include "blogpost.h"
#include "blogcomment.h"

#include <syndication/loader.h>
#include <syndication/item.h>

#include <kio/netaccess.h>
#include <kio/http.h>
#include <kio/job.h>
#include <KDebug>
#include <KLocale>
#include <KDateTime>

#include <QByteArray>
#include <QRegExp>
#include <QDomDocument>

#define TIMEOUT 600

using namespace KBlog;

GData::GData( const KUrl &server, QObject *parent )
  : Blog( server, *new GDataPrivate, parent )
{
  kDebug();
  setUrl( server );
}

GData::~GData()
{
  kDebug();
}

QString GData::interfaceName() const
{
  kDebug();
  return QLatin1String( "Google Blogger Data" );
}

QString GData::fullName() const
{
  kDebug();
  return d_func()->mFullName;
}

void GData::setFullName( const QString &fullName )
{
  kDebug();
  Q_D( GData );
  d->mFullName = fullName;
}

QString GData::profileId() const
{
  kDebug();
  return d_func()->mProfileId;
}

void GData::setProfileId( const QString &pid )
{
  kDebug();
  Q_D( GData );
  d->mProfileId = pid;
}

void GData::fetchProfileId()
{
  kDebug();
  QByteArray data;
  KIO::Job *job = KIO::get( url(), KIO::NoReload, KIO::HideProgressInfo );
  KUrl blogUrl = url();
  connect( job, SIGNAL(data(KIO::Job*,const QByteArray&)),
           this, SLOT(slotFetchProfileIdData(KIO::Job*,const QByteArray&)) );
  connect( job, SIGNAL(result(KJob*)),
           this, SLOT(slotFetchProfileId(KJob*)) );
}

void GData::listBlogs()
{
  kDebug();
  Syndication::Loader *loader = Syndication::Loader::create();
  connect( loader,
           SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
           this,
           SLOT(slotListBlogs(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)) );
  loader->loadFrom( "http://www.blogger.com/feeds/" + profileId() + "/blogs" );
}

void GData::listRecentPosts( const QStringList &labels, int number,
                             const KDateTime &upMinTime, const KDateTime &upMaxTime,
                             const KDateTime &pubMinTime, const KDateTime &pubMaxTime )
{
  kDebug();
  Q_D( GData );
  QString urlString( "http://www.blogger.com/feeds/" + blogId() + "/posts/default" );
  if ( ! labels.empty() ) {
    urlString += "/-/" + labels.join( "/" );
  }
  kDebug() << "listRecentPosts()";
  KUrl url( urlString );

  if ( !upMinTime.isNull() ) {
    url.addQueryItem( "updated-min", upMinTime.toString() );
  }

  if( !upMaxTime.isNull() ) {
    url.addQueryItem( "updated-max", upMaxTime.toString() );
  }

  if( !pubMinTime.isNull() ) {
    url.addQueryItem( "published-min", pubMinTime.toString() );
  }

  if( !pubMaxTime.isNull() ) {
    url.addQueryItem( "published-max", pubMaxTime.toString() );
  }

  Syndication::Loader *loader = Syndication::Loader::create();
  if ( number > 0 ) {
    d->mListRecentPostsMap[ loader ] = number;
  }
  connect( loader,
           SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
           this,
           SLOT(slotListRecentPosts(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)) );
  loader->loadFrom( url.url() );
}

void GData::listRecentPosts( int number )
{
  kDebug();
  listRecentPosts( QStringList(), number );
}

void GData::listComments( KBlog::BlogPost *post )
{
  kDebug();
  Q_D( GData );
  Syndication::Loader *loader = Syndication::Loader::create();
  d->mListCommentsMap[ loader ] = post;
  connect( loader,
           SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
           this,
           SLOT(slotListComments(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)) );
  loader->loadFrom( "http://www.blogger.com/feeds/" + blogId() + '/' +
                    post->postId() + "/comments/default" );
}

void GData::listAllComments()
{
  kDebug();
  Syndication::Loader *loader = Syndication::Loader::create();
  connect( loader,
           SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
           this,
           SLOT(slotListAllComments(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)) );
  loader->loadFrom( "http://www.blogger.com/feeds/" + blogId() + "/comments/default" );
}

void GData::fetchPost( KBlog::BlogPost *post )
{
  kDebug();
  Q_D( GData );

  if ( !post ) {
    kError() << "post is null pointer";
    return;
  }

  kDebug();
  Syndication::Loader *loader = Syndication::Loader::create();
  d->mFetchPostMap[ loader ] = post;
  connect( loader,
           SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
           this,
           SLOT(slotFetchPost(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)) );
  loader->loadFrom( "http://www.blogger.com/feeds/" + blogId() + "/posts/default" );
}

void GData::modifyPost( KBlog::BlogPost *post )
{
  kDebug();
  Q_D( GData );

  if ( !post ) {
    kError() << "post is null pointer";
    return;
  }

  if ( !d->authenticate() ){
    kError() << "Authentication failed.";
    emit errorPost( Atom, i18n( "Authentication failed." ), post );
    return;
  }

  QString atomMarkup = "<entry xmlns='http://www.w3.org/2005/Atom'>";
  atomMarkup += "<id>tag:blogger.com,1999:blog-" + blogId();
  atomMarkup += ".post-" + post->postId() + "</id>";
  atomMarkup += "<published>" + post->creationDateTime().toString() + "</published>";
  atomMarkup += "<updated>" + post->modificationDateTime().toString() + "</updated>";
  atomMarkup += "<title type='text'>" + post->title() + "</title>";
  if( post->isPrivate() ) {
    atomMarkup += "<app:control xmlns:app='http://purl.org/atom/app#'>";
    atomMarkup += "<app:draft>yes</app:draft></app:control>";
  }
  atomMarkup += "<content type='xhtml'>";
  atomMarkup += "<div xmlns='http://www.w3.org/1999/xhtml'>";
  atomMarkup += post->content();
  atomMarkup += "</div></content>";
  QList<QString>::ConstIterator it = post->tags().constBegin();
  QList<QString>::ConstIterator end = post->tags().constEnd();
  for( ; it != end; ++it ){
      atomMarkup += "<category scheme='http://www.blogger.com/atom/ns#' term='" + ( *it ) + "' />";
  }
  atomMarkup += "<author>";
  if ( !fullName().isEmpty() ) {
    atomMarkup += "<name>" + fullName() + "</name>";
  }
  atomMarkup += "<email>" + username() + "</email>";
  atomMarkup += "</author>";
  atomMarkup += "</entry>";
  QByteArray postData;
  QDataStream stream( &postData, QIODevice::WriteOnly );
  stream.writeRawData( atomMarkup.toUtf8(), atomMarkup.toUtf8().length() );

  KIO::TransferJob *job = KIO::http_post(
      KUrl( "http://www.blogger.com/feeds/" + blogId() + "/posts/default/" + post->postId() ),
      postData, KIO::HideProgressInfo );

  Q_ASSERT( job );

  d->mModifyPostMap[ job ] = post;

  job->addMetaData( "content-type", "Content-Type: application/atom+xml; charset=utf-8" );
  job->addMetaData( "ConnectTimeout", "50" );
  job->addMetaData( "UserAgent", userAgent() );
  job->addMetaData( "customHTTPHeader",
                    "Authorization: GoogleLogin auth=" + d->mAuthenticationString +
                    "\r\nX-HTTP-Method-Override: PUT" );

  connect( job, SIGNAL(data(KIO::Job*,const QByteArray&)),
           this, SLOT(slotModifyPostData(KIO::Job*,const QByteArray&)) );
  connect( job, SIGNAL(result(KJob*)),
           this, SLOT(slotModifyPost(KJob*)) );
}

void GData::createPost( KBlog::BlogPost *post )
{
  kDebug();
  Q_D( GData );

  if ( !post ) {
    kError() << "post is null pointer";
    return;
  }

  if ( !d->authenticate() ){
    kError() << "Authentication failed.";
    emit errorPost( Atom, i18n( "Authentication failed." ), post );
    return;
  }

  QString atomMarkup = "<entry xmlns='http://www.w3.org/2005/Atom'>";
  atomMarkup += "<title type='text'>" + post->title() + "</title>";
  if ( post->isPrivate() ) {
    atomMarkup += "<app:control xmlns:app='http://purl.org/atom/app#'>";
    atomMarkup += "<app:draft>yes</app:draft></app:control>";
  }
  atomMarkup += "<content type='xhtml'>";
  atomMarkup += "<div xmlns='http://www.w3.org/1999/xhtml'>";
  atomMarkup += post->content(); // FIXME check for Utf
  atomMarkup += "</div></content>";
  QList<QString>::ConstIterator it = post->tags().constBegin();
  QList<QString>::ConstIterator end = post->tags().constEnd();
  for( ; it != end; ++it ){
    atomMarkup += "<category scheme='http://www.blogger.com/atom/ns#' term='" + ( *it ) + "' />";
  }
  atomMarkup += "<author>";
  if ( !fullName().isEmpty() ) {
    atomMarkup += "<name>" + fullName() + "</name>";
  }
  atomMarkup += "<email>" + username() + "</email>";
  atomMarkup += "</author>";
  atomMarkup += "</entry>";

  QByteArray postData;
  QDataStream stream( &postData, QIODevice::WriteOnly );
  stream.writeRawData( atomMarkup.toUtf8(), atomMarkup.toUtf8().length() );

  KIO::TransferJob *job = KIO::http_post(
    KUrl( "http://www.blogger.com/feeds/" + blogId() + "/posts/default" ),
    postData, KIO::HideProgressInfo );

  Q_ASSERT ( job );
  d->mCreatePostMap[ job ] = post;

  job->addMetaData( "content-type", "Content-Type: application/atom+xml; charset=utf-8" );
  job->addMetaData( "ConnectTimeout", "50" );
  job->addMetaData( "UserAgent", userAgent() );
  job->addMetaData( "customHTTPHeader",
                    "Authorization: GoogleLogin auth=" + d->mAuthenticationString );

  connect( job, SIGNAL(data(KIO::Job*,const QByteArray&)),
           this, SLOT(slotCreatePostData(KIO::Job*,const QByteArray&)) );
  connect( job, SIGNAL(result(KJob*)),
           this, SLOT(slotCreatePost(KJob*)) );
}

void GData::removePost( KBlog::BlogPost *post )
{
  kDebug();
  Q_D( GData );

  if ( !post ) {
    kError() << "post is null pointer";
    return;
  }

  if ( !d->authenticate() ){
    kError() << "Authentication failed.";
    emit errorPost( Atom, i18n( "Authentication failed." ), post );
    return;
  }

  QByteArray postData;

  KIO::TransferJob *job = KIO::http_post(
    KUrl( "http://www.blogger.com/feeds/" + blogId() + "/posts/default/" + post->postId() ),
    postData, KIO::HideProgressInfo );

  d->mRemovePostMap[ job ] = post;

  if ( !job ) {
    kWarning() << "Unable to create KIO job for http://www.blogger.com/feeds/"
               << blogId() << "/posts/default/" + post->postId();
  }

  job->addMetaData( "ConnectTimeout", "50" );
  job->addMetaData( "UserAgent", userAgent() );
  job->addMetaData( "customHTTPHeader",
                    "Authorization: GoogleLogin auth=" + d->mAuthenticationString +
                    "\r\nX-HTTP-Method-Override: DELETE" );

  connect( job, SIGNAL(data(KIO::Job*,const QByteArray&)),
           this, SLOT(slotRemovePostData(KIO::Job*,const QByteArray&)) );
  connect( job, SIGNAL(result(KJob*)),
           this, SLOT(slotRemovePost(KJob*)) );
}

void GData::createComment( KBlog::BlogPost *post, KBlog::BlogComment *comment )
{
  kDebug();

  if ( !comment ) {
    kError() << "comment is null pointer";
    return;
  }

  if ( !post ) {
    kError() << "post is null pointer";
    return;
  }

  Q_D( GData );
  if ( !d->authenticate() ){
    kError() << "Authentication failed.";
    emit errorComment( Atom, i18n( "Authentication failed." ), post, comment );
    return;
  }
  QString atomMarkup = "<entry xmlns='http://www.w3.org/2005/Atom'>";
  atomMarkup += "<title type=\"text\">" + comment->title() + "</title>";
  atomMarkup += "<content type=\"html\">" + comment->content() + "</content>";
  atomMarkup += "<author>";
  atomMarkup += "<name>" + comment->name() + "</name>";
  atomMarkup += "<email>" + comment->email() + "</email>";
  atomMarkup += "</author></entry>";

  QByteArray postData;
  kDebug() <<  postData;
  QDataStream stream( &postData, QIODevice::WriteOnly );
  stream.writeRawData( atomMarkup.toUtf8(), atomMarkup.toUtf8().length() );

  KIO::TransferJob *job = KIO::http_post(
    KUrl( "http://www.blogger.com/feeds/" + blogId() + "/" + post->postId() + "/comments/default" ),
    postData, KIO::HideProgressInfo );

  d->mCreateCommentMap[ job ][post] = comment;

  if ( !job ) {
    kWarning() << "Unable to create KIO job for http://www.blogger.com/feeds/"
               << blogId() << "/" << post->postId() << "/comments/default";
  }

  job->addMetaData( "content-type", "Content-Type: application/atom+xml; charset=utf-8" );
  job->addMetaData( "ConnectTimeout", "50" );
  job->addMetaData( "customHTTPHeader",
                    "Authorization: GoogleLogin auth=" + d->mAuthenticationString );
  job->addMetaData( "UserAgent", userAgent() );

  connect( job, SIGNAL(data(KIO::Job*,const QByteArray&)),
           this, SLOT(slotCreateCommentData(KIO::Job*,const QByteArray&)) );
  connect( job, SIGNAL(result(KJob*)),
           this, SLOT(slotCreateComment(KJob*)) );
}

void GData::removeComment( KBlog::BlogPost *post, KBlog::BlogComment *comment )
{
  kDebug();
  Q_D( GData );
  kDebug();

  if ( !comment ) {
    kError() << "comment is null pointer";
    return;
  }

  if ( !post ) {
    kError() << "post is null pointer";
    return;
  }

  if ( !d->authenticate() ){
    kError() << "Authentication failed.";
    emit errorComment( Atom, i18n( "Authentication failed." ), post, comment );
    return;
  }

  QByteArray postData;

  KIO::TransferJob *job = KIO::http_post(
    KUrl( "http://www.blogger.com/feeds/" + blogId() + "/" + post->postId() +
          "/comments/default/" + comment->commentId() ),
    postData, KIO::HideProgressInfo );
  d->mRemoveCommentMap[ job ][ post ] = comment;

  if ( !job ) {
    kWarning() << "Unable to create KIO job for http://www.blogger.com/feeds/"
               << blogId() << post->postId()
               << "/comments/default/" << comment->commentId();
  }

  job->addMetaData( "ConnectTimeout", "50" );
  job->addMetaData( "UserAgent", userAgent() );
  job->addMetaData( "customHTTPHeader",
                    "Authorization: GoogleLogin auth=" +
                    d->mAuthenticationString + "\r\nX-HTTP-Method-Override: DELETE" );

  connect( job, SIGNAL(data(KIO::Job*,const QByteArray&)),
           this, SLOT(slotRemoveCommentData(KIO::Job*,const QByteArray&)) );
  connect( job, SIGNAL(result(KJob*)),
           this, SLOT(slotRemoveComment(KJob*)) );
}

GDataPrivate::GDataPrivate():mAuthenticationString(), mAuthenticationTime()
{
  kDebug();
}

GDataPrivate::~GDataPrivate()
{
  kDebug();
}

bool GDataPrivate::authenticate()
{
  kDebug();
  Q_Q( GData );
  QByteArray data;
  KUrl authGateway( "https://www.google.com/accounts/ClientLogin" );
  authGateway.addQueryItem( "Email", q->username() );
  authGateway.addQueryItem( "Passwd", q->password() );
  authGateway.addQueryItem( "source", q->userAgent() );
  authGateway.addQueryItem( "service", "blogger" );
  if ( !mAuthenticationTime.isValid() ||
       QDateTime::currentDateTime().toTime_t() - mAuthenticationTime.toTime_t() > TIMEOUT ||
       mAuthenticationString.isEmpty() ) {
    KIO::Job *job = KIO::http_post( authGateway, QByteArray(), KIO::HideProgressInfo );
    if ( KIO::NetAccess::synchronousRun( job, (QWidget*)0, &data, &authGateway ) ) {
      QRegExp rx( "Auth=(.+)" );
      if ( rx.indexIn( data ) != -1 ) {
        kDebug() << "RegExp got authentication string:" << rx.cap(1);
        mAuthenticationString = rx.cap(1);
        mAuthenticationTime = QDateTime::currentDateTime();
        return true;
      }
    }
    return false;
  }
  return true;
}

void GDataPrivate::slotFetchProfileIdData( KIO::Job *job, const QByteArray &data )
{
  kDebug();
  if( !job ){
    kError() << "job is a null pointer.";
    return;
  }
  unsigned int oldSize = mFetchProfileIdBuffer[ job ].size();
  mFetchProfileIdBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mFetchProfileIdBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void GDataPrivate::slotFetchProfileId( KJob *job )
{
  kDebug();
  if( !job ){
    kError() << "job is a null pointer.";
    return;
  }
  Q_Q( GData );
  if ( !job->error() ) {
    QRegExp pid( "http://www.blogger.com/profile/(\\d+)" );
    if ( pid.indexIn( mFetchProfileIdBuffer[ job ] ) != -1 ) {
      q->setProfileId( pid.cap(1) );
      kDebug() << "QRegExp bid( 'http://www.blogger.com/profile/(\\d+)' matches" << pid.cap(1);
      emit q->fetchedProfileId( pid.cap(1) );
    } else {
      kError() << "QRegExp bid( 'http://www.blogger.com/profile/(\\d+)' "
                   << " could not regexp the Profile ID";
      emit q->error( GData::Other, i18n( "Could not regexp the Profile ID." ) );
      emit q->fetchedProfileId( QString() );
    }
  } else {
    kError() << "Could not fetch the homepage data.";
    emit q->error( GData::Other, i18n( "Could not fetch the homepage data." ) );
    emit q->fetchedProfileId( QString() );
  }
  mFetchProfileIdBuffer[ job ].resize( 0 );
  mFetchProfileIdBuffer.remove( job );
}

void GDataPrivate::slotListBlogs( Syndication::Loader *loader,
                                  Syndication::FeedPtr feed,
                                  Syndication::ErrorCode status ) {
  kDebug();
  Q_Q( GData );
  if( !loader ) {
    kError() << "loader is a null pointer.";
    return;
  }
  if ( status != Syndication::Success ) {
    emit q->error( GData::Atom, i18n( "Could not get blogs." ) );
    return;
  }

  QList<QMap<QString,QString> > blogsList;

  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
  for ( ; it != end; ++it ) {
    QRegExp rx( "blog-(\\d+)" );
    QMap<QString,QString> blogInfo;
    if ( rx.indexIn( ( *it )->id() ) != -1 ) {
      kDebug() << "QRegExp rx( 'blog-(\\d+)' matches" << rx.cap(1);
      blogInfo["id"] = rx.cap(1);
      blogInfo["title"] = ( *it )->title();
      blogInfo["summary"] = ( *it )->description(); //TODO fix/add more
      blogsList << blogInfo;
    } else {
      kError() << "QRegExp rx( 'blog-(\\d+)' does not match anything in:"
          << ( *it )->id();
      emit q->error( GData::Other, i18n( "Could not regexp the blog id path." ) );
    }
  }
  kDebug() << "Emitting listedBlogs(); ";
  emit q->listedBlogs( blogsList );
}

void GDataPrivate::slotListComments( Syndication::Loader *loader,
                                     Syndication::FeedPtr feed,
                                     Syndication::ErrorCode status )
{
  kDebug();
  Q_Q( GData );
  if( !loader ) {
    kError() << "loader is a null pointer.";
    return;
  }
  BlogPost *post = mListCommentsMap[ loader ];
  mListCommentsMap.remove( loader );

  if ( status != Syndication::Success ) {
    emit q->errorPost( GData::Atom, i18n( "Could not get comments." ), post );
    return;
  }

  QList<KBlog::BlogComment> commentList;

  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
  for ( ; it != end; ++it ) {
    BlogComment comment;
    QRegExp rx( "post-(\\d+)" );
    if ( rx.indexIn( ( *it )->id() ) == -1 ) {
      kError() << "QRegExp rx( 'post-(\\d+)' does not match" << rx.cap(1);
      emit q->error( GData::Other, i18n( "Could not regexp the comment id path." ) );
    } else {
      comment.setCommentId( rx.cap(1) );
    }
    kDebug() << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
    comment.setTitle( ( *it )->title() );
    comment.setContent( ( *it )->content() );
//  FIXME: assuming UTC for now
    comment.setCreationDateTime(
      KDateTime( QDateTime::fromTime_t( ( *it )->datePublished() ),
                 KDateTime::Spec::UTC() ) );
    comment.setModificationDateTime(
      KDateTime( QDateTime::fromTime_t( ( *it )->dateUpdated() ),
                 KDateTime::Spec::UTC() ) );
    commentList.append( comment );
  }
  kDebug() << "Emitting listedComments()";
  emit q->listedComments( post, commentList );
}

void GDataPrivate::slotListAllComments( Syndication::Loader *loader,
                                        Syndication::FeedPtr feed,
                                        Syndication::ErrorCode status )
{
  kDebug();
  Q_Q( GData );
  if( !loader ) {
    kError() << "loader is a null pointer.";
    return;
  }

  if ( status != Syndication::Success ) {
    emit q->error( GData::Atom, i18n( "Could not get comments." ) );
    return;
  }

  QList<KBlog::BlogComment> commentList;

  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
  for ( ; it != end; ++it ) {
    BlogComment comment;
    QRegExp rx( "post-(\\d+)" );
    if ( rx.indexIn( ( *it )->id() ) == -1 ) {
      kError() << "QRegExp rx( 'post-(\\d+)' does not match"<< rx.cap(1);
      emit q->error( GData::Other, i18n( "Could not regexp the comment id path." ) );
    } else {
      comment.setCommentId( rx.cap(1) );
    }

    kDebug() << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
    comment.setTitle( ( *it )->title() );
    comment.setContent( ( *it )->content() );
//  FIXME: assuming UTC for now
    comment.setCreationDateTime(
      KDateTime( QDateTime::fromTime_t( ( *it )->datePublished() ),
                 KDateTime::Spec::UTC() ) );
    comment.setModificationDateTime(
      KDateTime( QDateTime::fromTime_t( ( *it )->dateUpdated() ),
                 KDateTime::Spec::UTC() ) );
    commentList.append( comment );
  }
  kDebug() << "Emitting listedAllComments()";
  emit q->listedAllComments( commentList );
}

void GDataPrivate::slotListRecentPosts( Syndication::Loader *loader,
                                        Syndication::FeedPtr feed,
                                        Syndication::ErrorCode status ) {
  kDebug();
  Q_Q( GData );
  if( !loader ) {
    kError() << "loader is a null pointer.";
    return;
  }

  if ( status != Syndication::Success ) {
    emit q->error( GData::Atom, i18n( "Could not get posts." ) );
    return;
  }
  int number = 0;

  if ( mListRecentPostsMap.contains( loader ) ) {
    number = mListRecentPostsMap[ loader ];
  }
  mListRecentPostsMap.remove( loader );

  QList<KBlog::BlogPost> postList;

  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
  for ( ; it != end; ++it ) {
    BlogPost post;
    QRegExp rx( "post-(\\d+)" );
    if ( rx.indexIn( ( *it )->id() ) == -1 ) {
      kError() << "QRegExp rx( 'post-(\\d+)' does not match"<< rx.cap(1);
      emit q->error( GData::Other, i18n( "Could not regexp the post id path." ) );
    } else {
      post.setPostId( rx.cap(1) );
    }

    kDebug() << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
    post.setTitle( ( *it )->title() );
    post.setContent( ( *it )->content() );
    post.setLink( ( *it )->link() );
//  FIXME: assuming UTC for now
    post.setCreationDateTime(
      KDateTime( QDateTime::fromTime_t( ( *it )->datePublished() ),
                 KDateTime::Spec::UTC() ) );
    post.setModificationDateTime(
      KDateTime( QDateTime::fromTime_t( ( *it )->dateUpdated() ),
                 KDateTime::Spec::UTC() ) );
    post.setStatus( BlogPost::Fetched );
    postList.append( post );
    if ( number-- == 0 ) {
      break;
    }
  }
  kDebug() << "Emitting listedRecentPosts()";
  emit q->listedRecentPosts( postList );
}

void GDataPrivate::slotFetchPost( Syndication::Loader *loader,
                                  Syndication::FeedPtr feed,
                                  Syndication::ErrorCode status )
{
  kDebug();
  Q_Q( GData );
  if( !loader ) {
    kError() << "loader is a null pointer.";
    return;
  }

  bool success = false;

  BlogPost *post = mFetchPostMap[ loader ];

  if ( status != Syndication::Success ) {
    emit q->errorPost( GData::Atom, i18n( "Could not get posts." ), post );
    return;
  }
  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
  for ( ; it != end; ++it ) {
    QRegExp rx( "post-(\\d+)" );
    if ( rx.indexIn( ( *it )->id() ) != -1 && rx.cap(1) == post->postId() ){
      kDebug() << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
      post->setPostId( rx.cap(1) );
      post->setTitle( ( *it )->title() );
      post->setContent( ( *it )->content() );
      post->setStatus( BlogPost::Fetched );
      post->setLink( ( *it )->link() );
//    FIXME: assuming UTC for now
      post->setCreationDateTime(
        KDateTime( QDateTime::fromTime_t( ( *it )->datePublished() ),
                   KDateTime::Spec::UTC() ) );
      post->setModificationDateTime(
        KDateTime( QDateTime::fromTime_t( ( *it )->dateUpdated() ),
                   KDateTime::Spec::UTC() ) );
      kDebug() << "Emitting fetchedPost( postId=" << post->postId() << ");";
      success = true;
      emit q->fetchedPost( post );
    }
  }
  if ( !success ) {
    kError() << "QRegExp rx( 'post-(\\d+)' does not match"
        << mFetchPostMap[ loader ]->postId() << ".";
    emit q->errorPost( GData::Other, i18n( "Could not regexp the blog id path." ), post );
  }
  mFetchPostMap.remove( loader );
}

void GDataPrivate::slotCreatePostData( KIO::Job *job, const QByteArray &data )
{
  kDebug();
  if( !job ) {
    kError() << "job is a null pointer.";
    return;
  }
  unsigned int oldSize = mCreatePostBuffer[ job ].size();
  mCreatePostBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mCreatePostBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void GDataPrivate::slotCreatePost( KJob *job )
{
  kDebug();
  if( !job ) {
    kError() << "job is a null pointer.";
    return;
  }
  const QString data = QString::fromUtf8( mCreatePostBuffer[ job ].data(),
                                          mCreatePostBuffer[ job ].size() );
  mCreatePostBuffer[ job ].resize( 0 );

  Q_Q( GData );

  KBlog::BlogPost *post = mCreatePostMap[ job ];
  mCreatePostMap.remove( job );

  if ( job->error() != 0 ) {
    kError() << "slotCreatePost error:" << job->errorString();
    emit q->errorPost( GData::Atom, job->errorString(), post );
    return;
  }

  QRegExp rxId( "post-(\\d+)" ); //FIXME check and do better handling, esp the creation date time
  if ( rxId.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the id out of the result:" << data;
    emit q->errorPost( GData::Atom,
                       i18n( "Could not regexp the id out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

  QRegExp rxPub( "<published>(.+)</published>" );
  if ( rxPub.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the published time out of the result:" << data;
    emit q->errorPost( GData::Atom,
                       i18n( "Could not regexp the published time out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

  QRegExp rxUp( "<updated>(.+)</updated>" );
  if ( rxUp.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the update time out of the result:" << data;
    emit q->errorPost( GData::Atom,
                       i18n( "Could not regexp the update time out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);

  post->setPostId( rxId.cap(1) );
  post->setCreationDateTime( KDateTime().fromString( rxPub.cap(1) ) );
  post->setModificationDateTime( KDateTime().fromString( rxUp.cap(1) ) );
  post->setStatus( BlogPost::Created );
  kDebug() << "Emitting createdPost()";
  emit q->createdPost( post );
}

void GDataPrivate::slotModifyPostData( KIO::Job *job, const QByteArray &data )
{
  kDebug();
  if( !job ) {
    kError() << "job is a null pointer.";
    return;
  }
  unsigned int oldSize = mModifyPostBuffer[ job ].size();
  mModifyPostBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mModifyPostBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void GDataPrivate::slotModifyPost( KJob *job )
{
  kDebug();
  if( !job ) {
    kError() << "job is a null pointer.";
    return;
  }
  const QString data = QString::fromUtf8( mModifyPostBuffer[ job ].data(),
                                          mModifyPostBuffer[ job ].size() );
  mModifyPostBuffer[ job ].resize( 0 );

  KBlog::BlogPost *post = mModifyPostMap[ job ];
  mModifyPostMap.remove( job );
  Q_Q( GData );
  if ( job->error() != 0 ) {
    kError() << "slotModifyPost error:" << job->errorString();
    emit q->errorPost( GData::Atom, job->errorString(), post );
    return;
  }

  QRegExp rxId( "post-(\\d+)" ); //FIXME check and do better handling, esp creation date time
  if ( rxId.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the id out of the result:" << data;
    emit q->errorPost( GData::Atom,
                       i18n( "Could not regexp the id out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

  QRegExp rxPub( "<published>(.+)</published>" );
  if ( rxPub.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the published time out of the result:" << data;
    emit q->errorPost( GData::Atom,
                       i18n( "Could not regexp the published time out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

  QRegExp rxUp( "<updated>(.+)</updated>" );
  if ( rxUp.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the update time out of the result:" << data;
    emit q->errorPost( GData::Atom,
                       i18n( "Could not regexp the update time out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);
  post->setPostId( rxId.cap(1) );
  post->setCreationDateTime( KDateTime().fromString( rxPub.cap(1) ) );
  post->setModificationDateTime( KDateTime().fromString( rxUp.cap(1) ) );
  post->setStatus( BlogPost::Modified );
  emit q->modifiedPost( post );
}

void GDataPrivate::slotRemovePostData( KIO::Job *job, const QByteArray &data )
{
  kDebug();
  if( !job ) {
    kError() << "job is a null pointer.";
    return;
  }
  unsigned int oldSize = mRemovePostBuffer[ job ].size();
  mRemovePostBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mRemovePostBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void GDataPrivate::slotRemovePost( KJob *job )
{
  kDebug();
  if( !job ) {
    kError() << "job is a null pointer.";
    return;
  }
  const QString data = QString::fromUtf8( mRemovePostBuffer[ job ].data(),
                                          mRemovePostBuffer[ job ].size() );
  mRemovePostBuffer[ job ].resize( 0 );

  KBlog::BlogPost *post = mRemovePostMap[ job ];
  mRemovePostMap.remove( job );
  Q_Q( GData );
  if ( job->error() != 0 ) {
    kError() << "slotRemovePost error:" << job->errorString();
    emit q->errorPost( GData::Atom, job->errorString(), post );
    return;
  }

  post->setStatus( BlogPost::Removed );
  kDebug() << "Emitting removedPost()";
  emit q->removedPost( post );
}

void GDataPrivate::slotCreateCommentData( KIO::Job *job, const QByteArray &data )
{
  kDebug();
  if( !job ) {
    kError() << "job is a null pointer.";
    return;
  }
  unsigned int oldSize = mCreateCommentBuffer[ job ].size();
  mCreateCommentBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mCreateCommentBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void GDataPrivate::slotCreateComment( KJob *job )
{
  kDebug();
  if( !job ) {
    kError() << "job is a null pointer.";
    return;
  }
  const QString data = QString::fromUtf8( mCreateCommentBuffer[ job ].data(),
                                          mCreateCommentBuffer[ job ].size() );
  mCreateCommentBuffer[ job ].resize( 0 );
  kDebug() << "Dump data: " << data;

  Q_Q( GData );

  KBlog::BlogComment *comment = mCreateCommentMap[ job ].values().first();
  KBlog::BlogPost *post = mCreateCommentMap[ job ].keys().first();
  mCreateCommentMap.remove( job );

  if ( job->error() != 0 ) {
    kError() << "slotCreateComment error:" << job->errorString();
    emit q->errorComment( GData::Atom, job->errorString(), post, comment );
    return;
  }

// TODO check for result and fit appropriately
  QRegExp rxId( "post-(\\d+)" );
  if ( rxId.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the id out of the result:" << data;
    emit q->errorPost( GData::Atom,
                       i18n( "Could not regexp the id out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

  QRegExp rxPub( "<published>(.+)</published>" );
  if ( rxPub.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the published time out of the result:" << data;
    emit q->errorPost( GData::Atom,
                       i18n( "Could not regexp the published time out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

  QRegExp rxUp( "<updated>(.+)</updated>" );
  if ( rxUp.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the update time out of the result:" << data;
    emit q->errorPost( GData::Atom,
                       i18n( "Could not regexp the update time out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);
  comment->setCommentId( rxId.cap(1) );
  comment->setCreationDateTime( KDateTime().fromString( rxPub.cap(1) ) );
  comment->setModificationDateTime( KDateTime().fromString( rxUp.cap(1) ) );
  comment->setStatus( BlogComment::Created );
  kDebug() << "Emitting createdComment()";
  emit q->createdComment( post, comment );
}

void GDataPrivate::slotRemoveCommentData( KIO::Job *job, const QByteArray &data )
{
  kDebug();
  if( !job ) {
    kError() << "job is a null pointer.";
    return;
  }
  unsigned int oldSize = mRemoveCommentBuffer[ job ].size();
  mRemoveCommentBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mRemoveCommentBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void GDataPrivate::slotRemoveComment( KJob *job )
{
  kDebug();
  if( !job ) {
    kError() << "job is a null pointer.";
    return;
  }
  const QString data = QString::fromUtf8( mRemoveCommentBuffer[ job ].data(),
                                          mRemoveCommentBuffer[ job ].size() );
  mRemoveCommentBuffer[ job ].resize( 0 );

  Q_Q( GData );

  KBlog::BlogComment *comment = mRemoveCommentMap[ job ].values().first();
  KBlog::BlogPost *post = mRemoveCommentMap[ job ].keys().first();
  mRemoveCommentMap.remove( job );

  if ( job->error() != 0 ) {
    kError() << "slotRemoveComment error:" << job->errorString();
    emit q->errorComment( GData::Atom, job->errorString(), post, comment );
    return;
  }

  comment->setStatus( BlogComment::Created );
  kDebug() << "Emitting removedComment()";
  emit q->removedComment( post, comment );
}

#include "gdata.moc"
