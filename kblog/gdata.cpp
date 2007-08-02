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
#include "blogposting.h"
#include "blogpostingcomment.h"

#include <syndication/loader.h>
#include <syndication/item.h>
#include <syndication/atom/entry.h>

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
  Q_D(GData);
  setUrl( server );
}

GData::~GData()
{
}

QString GData::interfaceName() const
{
  return QLatin1String( "Google Blogger Data" );
}

QString GData::fullName() const
{
  return d_func()->mFullName;
}

void GData::setFullName( const QString &fullName )
{
  Q_D(GData);
  d->mFullName = fullName;
}

QString GData::profileId() const
{
  return d_func()->mProfileId;
}

void GData::setProfileId( const QString& pid )
{
  Q_D(GData);
  d->mProfileId = pid;
}

void GData::fetchProfileId()
{
  Q_D(GData);
  kDebug() << "fetchProfileId()";
  QByteArray data;
  KIO::Job *job = KIO::get( url(), false, false );
  KUrl blogUrl = url();
  connect( job, SIGNAL(data(KIO::Job*,const QByteArray&)),
                  this,SLOT(slotFetchProfileIdData(KIO::Job*,const QByteArray&)));
  connect( job, SIGNAL(result(KJob*)),
                  this,SLOT(slotFetchProfileId(KJob*)));
}

void GData::listBlogs()
{
  Q_D(GData);
  kDebug() << "listBlogs()";
  Syndication::Loader *loader = Syndication::Loader::create();
  connect( loader, SIGNAL(loadingComplete(Syndication::Loader*,
                          Syndication::FeedPtr, Syndication::ErrorCode)),
                          this, SLOT(slotListBlogs(Syndication::Loader*,
                  Syndication::FeedPtr, Syndication::ErrorCode)) );
  loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + profileId()
      + QString( "/blogs" ) );
}

void GData::listRecentPostings( const QStringList &labels, const int number, 
                const KDateTime &minTime, const KDateTime &maxTime, 
                const listRecentPostingsOptions &opts )
{

}

void GData::listRecentPostings( const int number )
{
  Q_D(GData);
  kDebug() << "listRecentPostings()";
  Syndication::Loader *loader = Syndication::Loader::create();
  connect( loader, SIGNAL(loadingComplete(Syndication::Loader*,
                          Syndication::FeedPtr, Syndication::ErrorCode)),
                          this, SLOT(slotListRecentPostings(
                                  Syndication::Loader*,
                  Syndication::FeedPtr, Syndication::ErrorCode)) );
  loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + blogId()
      + QString( "/posts/default" ) );
}

void GData::listComments( KBlog::BlogPosting *posting )
{
  Q_D(GData);
  kDebug() << "listComments()";
  Syndication::Loader *loader = Syndication::Loader::create();
  connect( loader, SIGNAL(loadingComplete(Syndication::Loader*,
                          Syndication::FeedPtr, Syndication::ErrorCode)),
                          this, SLOT(slotListComments(
                                  Syndication::Loader*,
                  Syndication::FeedPtr, Syndication::ErrorCode)) );
  loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + blogId()
      + posting->postingId() + QString( "/comments/default" ) );
}

void GData::listAllComments()
{
  Q_D(GData);
  kDebug() << "listRecentPostings()";
  Syndication::Loader *loader = Syndication::Loader::create();
  connect( loader, SIGNAL(loadingComplete(Syndication::Loader*,
                          Syndication::FeedPtr, Syndication::ErrorCode)),
                          this, SLOT(slotListAllComments(
                                  Syndication::Loader*,
                  Syndication::FeedPtr, Syndication::ErrorCode)) );
  loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + blogId()
      + QString( "/comments/default" ) );
}

void GData::fetchPosting( KBlog::BlogPosting *posting )
{
  Q_D(GData);
  kDebug() << "fetchPosting()";
  Syndication::Loader *loader = Syndication::Loader::create();
  d->mFetchPostingMap[ loader ] = posting;
  connect( loader, SIGNAL(loadingComplete(Syndication::Loader*,
                   Syndication::FeedPtr, Syndication::ErrorCode)),
                   this, SLOT(slotFetchPosting(Syndication::Loader*,
                   Syndication::FeedPtr, Syndication::ErrorCode)));
  loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + blogId()
      + QString( "/posts/default" ) );
}

void GData::modifyPosting( KBlog::BlogPosting* posting )
{
  Q_D(GData);
    kDebug() << "modifyPosting()";
    if ( d->authenticate().isEmpty() ){
      kDebug(5323) << "Authentication failed.";
      emit error( Atom, "Authentication failed." );
      return;
    }

    QString atomMarkup = "<entry xmlns='http://www.w3.org/2005/Atom'>";
    atomMarkup += "<id>tag:blogger.com,1999:blog-"+blogId();
    atomMarkup += ".post-"+posting->postingId()+"</id>";
    atomMarkup += "<published>"+posting->creationDateTime().toString() +"</published>";
    atomMarkup += "<updated>"+posting->modificationDateTime().toString()+"</updated>";
    atomMarkup += "<title type='text'>"+posting->title().toUtf8() +"</title>";
    if( !posting->isPublished() )
    {
      atomMarkup += "<app:control xmlns:app=*http://purl.org/atom/app#'>";
      atomMarkup += "<app:draft>yes</app:draft></app:control>";
    }
    atomMarkup += "<content type='xhtml'>";
    atomMarkup += "<div xmlns='http://www.w3.org/1999/xhtml'>";
    atomMarkup += posting->content().toUtf8();
    atomMarkup += "</div></content>";
    atomMarkup += "<author>";
    atomMarkup += "<name>" + fullName().toUtf8() + "</name>";
    atomMarkup += "<email>" + username().toUtf8() + "</email>";
    atomMarkup += "</author>";
    atomMarkup += "</entry>";

    QByteArray postData;
    QDataStream stream( &postData, QIODevice::WriteOnly );
    stream.writeRawData( atomMarkup.toUtf8(), atomMarkup.toUtf8().length() );

    KIO::TransferJob *job = KIO::http_post(
        KUrl( "http://www.blogger.com/feeds/" + blogId() + "/posts/default/"+posting->postingId() ),
        postData, false );

    d->mCreatePostingMap[ job ] = posting;

    if ( !job ) {
      kWarning() << "Unable to create KIO job for http://www.blogger.com/feeds/"
          << blogId() <<"/posts/default/" << posting->postingId()  << endl;
    }


  job->addMetaData( "content-type", "Content-Type: application/atom+xml; charset=utf-8" );
  job->addMetaData( "ConnectTimeout", "50" );
  job->addMetaData( "customHTTPHeader", "Authorization: GoogleLogin auth=" + d->mAuthenticationString +
                                   "\r\nX-HTTP-Method-Override: PUT" );

    connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             this, SLOT( slotModifyPostingData( KIO::Job *, const QByteArray & ) ) );
    connect( job, SIGNAL( result( KJob * ) ),
             this, SLOT( slotModifyPosting( KJob * ) ) );
}


void GData::createPosting( KBlog::BlogPosting* posting )
{
  Q_D(GData);
    kDebug() << "createPosting()";
    if ( d->authenticate().isEmpty() ){
      kDebug(5323) << "Authentication failed.";
      emit error( Atom, "Authentication failed." );
      return;
    }

    QString atomMarkup = "<entry xmlns='http://www.w3.org/2005/Atom'>";
    atomMarkup += "<title type='text'>"+posting->title().toUtf8() +"</title>";
    if( !posting->isPublished() )
    {
      atomMarkup += "<app:control xmlns:app=*http://purl.org/atom/app#'>";
      atomMarkup += "<app:draft>yes</app:draft></app:control>";
    }
    atomMarkup += "<content type='xhtml'>";
    atomMarkup += "<div xmlns='http://www.w3.org/1999/xhtml'>";
    atomMarkup += posting->content().toUtf8(); // FIXME check for Utf
    atomMarkup += "</div></content>";
    atomMarkup += "<author>";
    atomMarkup += "<name>" + fullName().toUtf8() + "</name>";
    atomMarkup += "<email>" + username().toUtf8() + "</email>";
    atomMarkup += "</author>";
    atomMarkup += "</entry>";

    QByteArray postData;
    QDataStream stream( &postData, QIODevice::WriteOnly );
    stream.writeRawData( atomMarkup.toUtf8(), atomMarkup.toUtf8().length() );

    KIO::TransferJob *job = KIO::http_post(
        KUrl( "http://www.blogger.com/feeds/" + blogId() + "/posts/default" ),
        postData, false );

    d->mCreatePostingMap[ job ] = posting;

    if ( !job ) {
      kWarning() << "Unable to create KIO job for http://www.blogger.com/feeds/"
          << blogId() <<"/posts/default" << endl;
    }

  job->addMetaData( "content-type", "Content-Type: application/atom+xml; charset=utf-8" );
  job->addMetaData( "ConnectTimeout", "50" );
  job->addMetaData( "customHTTPHeader", "Authorization: GoogleLogin auth=" + d->mAuthenticationString );

    connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             this, SLOT( slotCreatePostingData( KIO::Job *, const QByteArray & ) ) );
    connect( job, SIGNAL( result( KJob * ) ),
             this, SLOT( slotCreatePosting( KJob * ) ) );
}

void GData::removePosting( KBlog::BlogPosting *posting )
{
  Q_D(GData);
    Q_UNUSED( posting );
    kDebug() << "deletePosting()";
    d->authenticate();
}

void GData::createComment( KBlog::BlogPosting *posting, KBlog::BlogPostingComment *comment )
{
  Q_D(GData);
  return; //FIXME
}

void GData::deleteComment( KBlog::BlogPosting *posting, KBlog::BlogPostingComment *comment )
{
  Q_D(GData);
  return; //FIXME
}

GDataPrivate::GDataPrivate():
  mAuthenticationString(),mAuthenticationTime(){

}

GDataPrivate::~GDataPrivate(){

}

QString GDataPrivate::authenticate(){
  Q_Q(GData);
  QByteArray data;
  KUrl authGateway( "https://www.google.com/accounts/ClientLogin" );
  authGateway.addQueryItem( "Email", q->username() );
  authGateway.addQueryItem( "Passwd", q->password() );
  authGateway.addQueryItem( "source" , q->userAgent() );
  authGateway.addQueryItem( "service", "blogger" );
  if( !mAuthenticationTime.isValid() ||
      QDateTime::currentDateTime().toTime_t() - mAuthenticationTime.toTime_t()
       > TIMEOUT || mAuthenticationString.isEmpty() ){
    KIO::Job *job = KIO::http_post( authGateway, QByteArray(), false );
    if ( KIO::NetAccess::synchronousRun(
         job, (QWidget*)0, &data, &authGateway ) ) {
      kDebug(5323) << "Fetched authentication result for" <<
          authGateway.prettyUrl() << ".";
      kDebug(5323) << "Authentication response:" << data;
      QRegExp rx( "Auth=(.+)" );
      if( rx.indexIn( data )!=-1 ){
        kDebug(5323)<<"RegExp got authentication string:" << rx.cap(1);
        mAuthenticationString = rx.cap(1);
        mAuthenticationTime = QDateTime::currentDateTime();
        return mAuthenticationString;
      }
    }
    return QString();
  }
  return mAuthenticationString;
}

void GDataPrivate::slotFetchProfileIdData( KIO::Job *job, const QByteArray &data )
{
  unsigned int oldSize = mFetchProfileIdBuffer[ job ].size();
  mFetchProfileIdBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mFetchProfileIdBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void GDataPrivate::slotFetchProfileId(KJob* job)
{
  Q_Q(GData);
  if ( !job->error() ) {
    QRegExp pid( "http://www.blogger.com/profile/(\\d+)" );
    if( pid.indexIn( mFetchProfileIdBuffer[ job ] )!=-1 ){
       q->setProfileId( pid.cap(1) );
       emit q->fetchedProfileId( pid.cap(1) );
    }
    else
      emit q->error( GData::Other, i18n( "Could not regexp the Profile ID." ) );
      emit q->fetchedProfileId( QString() );
    kDebug(5323)<<"QRegExp bid( 'http://www.blogger.com/profile/(\\d+)' matches"
        << pid.cap(1);
  }
  else {
    emit q->error( GData::Other, i18n( "Could not fetch the homepage data." ) );
    emit q->fetchedProfileId( QString() );
    kDebug(5323)<< "Could not fetch the homepage data.";
  }
  mFetchProfileIdBuffer[ job ].resize( 0 );
  mFetchProfileIdBuffer.remove( job );
}

void GDataPrivate::slotListBlogs(
    Syndication::Loader* loader, Syndication::FeedPtr feed,
    Syndication::ErrorCode status ) {
  Q_Q(GData);
  Q_UNUSED( loader );
  if (status != Syndication::Success){
    emit q->error( GData::Atom, i18n( "Could not get blogs." ) );
    return;
  }

  QMap <QString,QMap<QString,QString> > blogMap;

  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.begin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.end();
  for( ; it!=end; ++it ){
      QRegExp rx( "blog-(\\d+)" );
      if( rx.indexIn( ( *it )->id() )!=-1 ){
        kDebug(5323)<<"QRegExp rx( 'blog-(\\d+)' matches"<< rx.cap(1);
        blogMap[ rx.cap(1) ]["title"] = ( *it )->title();
        blogMap[ rx.cap(1) ]["summary"] = ( *it )->description(); //TODO fix/add more
      }
      else{
        emit q->error( GData::Other,
                            i18n( "Could not regexp the blog id path." ) );
        kDebug(5323)<<"QRegExp rx( 'blog-(\\d+)' does not match anything in:"
            << ( *it )->id();
      }

    }
    emit q->listedBlogs( blogMap );
    kDebug(5323) << "Emitting listedBlogs(); ";
}


void GDataPrivate::slotListComments(
    Syndication::Loader* loader, Syndication::FeedPtr feed,
    Syndication::ErrorCode status )
{

}

void GDataPrivate::slotListAllComments(
    Syndication::Loader* loader, Syndication::FeedPtr feed,
    Syndication::ErrorCode status )
{
  Q_Q(GData);
  Q_UNUSED( loader );

  if (status != Syndication::Success){
    emit q->error( GData::Atom, i18n( "Could not get comments." ) );
    return;
  }

  QList<KBlog::BlogPostingComment*> commentList;

  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.begin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.end();
  for( ; it!=end; ++it ){
      BlogPostingComment* comment = new BlogPostingComment;
      QRegExp rx( "post-(\\d+)" );
      if( rx.indexIn( ( *it )->id() )==-1 ){
        kDebug(5323)<<
        "QRegExp rx( 'post-(\\d+)' does not match"<< rx.cap(1);
        emit q->error( GData::Other,
        i18n( "Could not regexp the comment id path." ) );
      }
      else {
        comment->setCommentId( rx.cap(1) );
      }

      kDebug(5323)<<"QRegExp rx( 'post-(\\d+)' matches"<< rx.cap(1);
      comment->setTitle( ( *it )->title() );
      comment->setContent( ( *it )->content() );
//       FIXME: assuming UTC for now
      comment->setCreationDateTime( KDateTime( QDateTime::fromTime_t(
  ( *it )->datePublished() ), KDateTime::Spec::UTC() ) );
      comment->setModificationDateTime( KDateTime( QDateTime::fromTime_t(
  ( *it )->dateUpdated() ), KDateTime::Spec::UTC() ) );
      commentList.append( comment );
  }
  kDebug(5323) << "Emitting listedRecentPostings()";
  emit q->listedAllComments( commentList );
}

void GDataPrivate::slotListRecentPostings(
    Syndication::Loader* loader, Syndication::FeedPtr feed,
    Syndication::ErrorCode status ) {
  Q_Q(GData);
  Q_UNUSED( loader );

  if (status != Syndication::Success){
    emit q->error( GData::Atom, i18n( "Could not get postings." ) );
    return;
  }

  QList<KBlog::BlogPosting*> postingList;

  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.begin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.end();
  for( ; it!=end; ++it ){
      BlogPosting* posting = new BlogPosting;
      QRegExp rx( "post-(\\d+)" );
      if( rx.indexIn( ( *it )->id() ) ==-1 ){
        kDebug(5323)<<
        "QRegExp rx( 'post-(\\d+)' does not match"<< rx.cap(1);
        emit q->error( GData::Other,
        i18n( "Could not regexp the posting id path." ) );
      }
      else {
        posting->setPostingId( rx.cap(1) );
      }

      kDebug(5323)<<"QRegExp rx( 'post-(\\d+)' matches"<< rx.cap(1);
      posting->setTitle( ( *it )->title() );
      posting->setContent( ( *it )->content() );
//       FIXME: assuming UTC for now
      posting->setCreationDateTime( KDateTime( QDateTime::fromTime_t(
  ( *it )->datePublished() ), KDateTime::Spec::UTC() ) );
      posting->setModificationDateTime( KDateTime( QDateTime::fromTime_t(
  ( *it )->dateUpdated() ), KDateTime::Spec::UTC() ) );
      postingList.append( posting );
  }
  kDebug(5323) << "Emitting listedRecentPostings()";
  emit q->listedRecentPostings( postingList );
}

void GDataPrivate::slotFetchPosting(
    Syndication::Loader* loader, Syndication::FeedPtr feed,
    Syndication::ErrorCode status ){
  kDebug(5323) << "slotFetchPosting";
  Q_Q(GData);

  bool success = false;

  if (status != Syndication::Success){
    emit q->error( GData::Atom, i18n( "Could not get postings." ) );
    return;
  }
  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.begin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.end();
  for( ; it!=end; ++it ){
      BlogPosting* posting = new BlogPosting;
      QRegExp rx( "post-(\\d+)" );
      if( rx.indexIn( ( *it )->id() )!=-1 && rx.cap(1)==mFetchPostingMap[ loader ]->postingId() ){
        kDebug(5323)<<"QRegExp rx( 'post-(\\d+)' matches"<< rx.cap(1);
        posting->setPostingId( rx.cap(1) );
        posting->setTitle( ( *it )->title() );
        posting->setContent( ( *it )->content() );
//         FIXME: assuming UTC for now
        posting->setCreationDateTime( KDateTime( QDateTime::fromTime_t(
                                     ( *it )->datePublished() ),
                                        KDateTime::Spec::UTC() ) );
        posting->setModificationDateTime( KDateTime( QDateTime::fromTime_t(
                                         ( *it )->dateUpdated() ),
                                            KDateTime::Spec::UTC() ) );
        emit q->fetchedPosting( posting );
        success = true;
        kDebug(5323) << "Emitting fetchedPosting( postingId="
            << posting->postingId() << ");";
      }
  }
  if(!success){
    emit q->error( GData::Other, i18n( "Could not regexp the blog id path." ) );
    kDebug(5323) << "QRegExp rx( 'post-(\\d+)' does not match"
        << mFetchPostingMap[ loader ]->postingId() << ".";
  }
  mFetchPostingMap.remove( loader );
}

void GDataPrivate::slotCreatePostingData( KIO::Job *job, const QByteArray &data )
{
  kDebug(5323) << "slotCreatePostingData()";
  unsigned int oldSize = mCreatePostingBuffer[ job ].size();
  mCreatePostingBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mCreatePostingBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void GDataPrivate::slotCreatePosting( KJob *job )
{
  kDebug(5323) << "slotCreatePosting()";  
  const QString data = QString::fromUtf8( mCreatePostingBuffer[ job ].data(), mCreatePostingBuffer[ job ].size() );
//   Syndication::Atom::Entry entry( data.documentElement() );
  Q_Q(GData);
  if ( job->error() != 0 ) {
    kDebug(5323) << "slotCreatePosting error:" << job->errorString();
    emit q->error( GData::Atom, job->errorString() );
    mCreatePostingBuffer[ job ].resize( 0 );
    mCreatePostingBuffer.remove( job );
    return;
  }

  KBlog::BlogPosting* posting = mCreatePostingMap[ job ];
  mCreatePostingMap.remove( job );

  QRegExp rxId( "post-(\\d+)" ); //FIXME check that and do better handling, especially the creation date time
  if( rxId.indexIn( data )==-1 ){
    kDebug(5323) << "Could not regexp the id out of the result:" << data;
    return;
  }
  kDebug(5323) << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

  QRegExp rxPub( "<published>(.+)</published>" ); 
  if( rxPub.indexIn( data )==-1 ){
    kDebug(5323) << "Could not regexp the published time out of the result:" << data;
    return;
  }
  kDebug(5323) << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

  QRegExp rxUp( "<updated>(.+)</updated>" );
  if( rxUp.indexIn( data )==-1 ){
    kDebug(5323) << "Could not regexp the update time out of the result:" << data;
    return;
  }
  kDebug(5323) << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);
  posting->setPostingId( rxId.cap(1) );
  posting->setCreationDateTime( KDateTime().fromString( rxPub.cap(1) ) );
  posting->setModificationDateTime( KDateTime().fromString( rxUp.cap(1) ) );
  emit q->createdPosting( posting );
  mCreatePostingBuffer[ job ].resize( 0 );
  mCreatePostingBuffer.remove( job );
}

void GDataPrivate::slotModifyPostingData( KIO::Job *job, const QByteArray &data )
{
  kDebug(5323) << "slotModifyPostingData()";
  kDebug(5323) << "Dump modify data: " << data;
  unsigned int oldSize = mModifyPostingBuffer[ job ].size();
  mModifyPostingBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mModifyPostingBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void GDataPrivate::slotModifyPosting( KJob *job )
{
  kDebug(5323) << "slotModifyPosting()";  
  const QString data = QString::fromUtf8( mModifyPostingBuffer[ job ].data(), mModifyPostingBuffer[ job ].size() );
//   Syndication::Atom::Entry entry( data.documentElement() );
  Q_Q(GData);
  if ( job->error() != 0 ) {
    kDebug(5323) << "slotModifyPosting error: " << job->errorString();
    emit q->error( GData::Atom, job->errorString() );
    mModifyPostingBuffer[ job ].resize( 0 );
    mModifyPostingBuffer.remove( job );
    return;
  }

  KBlog::BlogPosting* posting = mModifyPostingMap[ job ];
  mModifyPostingMap.remove( job );

  QRegExp rxId( "post-(\\d+)" ); //FIXME check that and do better handling, especially the creation date time
  if( rxId.indexIn( data )==-1 ){
    kDebug(5323) << "Could not regexp the id out of the result: " << data;
    return;
  }
  kDebug(5323) << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

  QRegExp rxPub( "<published>(.+)</published>" ); 
  if( rxPub.indexIn( data )==-1 ){
    kDebug(5323) << "Could not regexp the published time out of the result:" << data;
    return;
  }
  kDebug(5323) << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

  QRegExp rxUp( "<updated>(.+)</updated>" );
  if( rxUp.indexIn( data )==-1 ){
    kDebug(5323) << "Could not regexp the update time out of the result:" << data;
    return;
  }
  kDebug(5323) << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);
  posting->setPostingId( rxId.cap(1) );
  posting->setCreationDateTime( KDateTime().fromString( rxPub.cap(1) ) );
  posting->setModificationDateTime( KDateTime().fromString( rxUp.cap(1) ) );
  posting->setStatus( BlogPosting::Modified );
  emit q->modifiedPosting( posting );
  mModifyPostingBuffer[ job ].resize( 0 );
  mModifyPostingBuffer.remove( job );
}

#include "gdata.moc"
