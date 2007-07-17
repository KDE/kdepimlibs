/*
    This file is part of the kblog library.

    Copyright (c) 2007 Christian Weilbach <christian@whiletaker.homeip.net>

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

#include <gdata_p.h>

#include <syndication/loader.h>
#include <syndication/item.h>

#include <kdebug.h>
#include <kio/netaccess.h>
#include <kio/http.h>
#include <klocale.h>
#include <kdatetime.h>
#include <kurl.h>

#include <QtCore/QByteArray>
#include <QtCore/QRegExp>
#include <QtGui/QWidget>

#define TIMEOUT 600

using namespace KBlog;

APIGData::APIGDataPrivate::APIGDataPrivate():
  mFetchPostingId(),mAuthenticationString(),mAuthenticationTime(){

}

APIGData::APIGDataPrivate::~APIGDataPrivate(){

}

QString APIGData::APIGDataPrivate::authenticate(){
  QByteArray data;
  KUrl authGateway( "https://www.google.com/accounts/ClientLogin" );
  authGateway.addQueryItem( "Email", parent->username() );
  authGateway.addQueryItem( "Passwd", parent->password() );
  authGateway.addQueryItem( "source" , "KBlog" );
  authGateway.addQueryItem( "service", "blogger" );
  if( !mAuthenticationTime.isValid() || 
      QDateTime::currentDateTime().toTime_t() - mAuthenticationTime.toTime_t() > TIMEOUT || 
      mAuthenticationString.isEmpty() ){
    KIO::Job *job = KIO::http_post( authGateway, QByteArray(), false ); 
    job->addMetaData( "content-length", "0" );
    if ( KIO::NetAccess::synchronousRun( job, (QWidget*)0, &data, &authGateway ) ) {
      kDebug(5323) << "Fetched authentication result for " << authGateway.prettyUrl() << ". " << endl;
      kDebug(5323) << "Authentication response " << data << endl;
      QRegExp rx( "Auth=(.+)" );
      if( rx.indexIn( data )!=-1 ){
        kDebug(5323)<<"RegExp got authentication string: " << rx.cap(1) << endl;
        mAuthenticationString = rx.cap(1);
        mAuthenticationTime = QDateTime::currentDateTime();
        return mAuthenticationString;
      }
    }
    return QString();
  }
  return mAuthenticationString;
}

void APIGData::APIGDataPrivate::slotLoadingBlogsComplete( Syndication::Loader* loader, 
                                          Syndication::FeedPtr feed, Syndication::ErrorCode status ){
  if (status != Syndication::Success){
    emit parent->error( AtomAPI, i18n( "Could not get blogs." ) );
    return;
  }
  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.begin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.end();
  for( ; it!=end; ++it ){
      QRegExp rx( "blog-(\\d+)" );
      QString id, name;
      if( rx.indexIn( ( *it )->id() )!=-1 ){
        kDebug(5323)<<"QRegExp rx( 'blog-(\\d+)' matches "<< rx.cap(1) << endl;
        id = rx.cap(1);
        name = ( *it )->title();
      }
      else{
        emit parent->error( Other, i18n( "Could not regexp the blog id path." ) );
        kDebug(5323)<<"QRegExp rx( 'blog-(\\d+)' does not match anything in: "<< ( *it )->id() << endl;
      }

      if ( !id.isEmpty() && !name.isEmpty() ) {
        emit parent->blogInfoRetrieved( id, name );
        kDebug(5323) << "Emitting blogInfoRetrieved( id=" << id
                 << ", name=" << name << "); " << endl;
      }
  }
}

void APIGData::APIGDataPrivate::slotLoadingPostingsComplete( Syndication::Loader* loader, 
                                          Syndication::FeedPtr feed, Syndication::ErrorCode status ){
  if (status != Syndication::Success){
    emit parent->error( AtomAPI, i18n( "Could not get postings." ) );
    return;
  } 
  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.begin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.end();
  for( ; it!=end; ++it ){
      BlogPosting posting;
      QRegExp rx( "post-(\\d+)" );
      if( rx.indexIn( ( *it )->id() )==-1 ){
        kDebug(5323)<<"QRegExp rx( 'post-(\\d+)' does not match "<< rx.cap(1) << endl; 
        emit parent->error( Other, i18n( "Could not regexp the posting id path." ) );
        return;
      }

      kDebug(5323)<<"QRegExp rx( 'post-(\\d+)' matches "<< rx.cap(1) << endl; 
      posting.setPostingId( rx.cap(1) );
      posting.setTitle( ( *it )->title() );
      posting.setContent( ( *it )->content() );
      // FIXME: assuming UTC for now
      posting.setCreationDateTime( KDateTime( QDateTime::fromTime_t( ( *it )->datePublished() ), KDateTime::Spec::UTC() ) ); 
      posting.setModificationDateTime( KDateTime( QDateTime::fromTime_t( ( *it )->dateUpdated() ), KDateTime::Spec::UTC() ) );

      emit parent->listedPosting( posting );
      kDebug(5323) << "Emitting listedPosting( postingId=" << posting.postingId() << " ); " << endl;
  }
  kDebug(5323) << "Emitting listPostingsFinished()" << endl;
  emit parent->listPostingsFinished();
}

void APIGData::APIGDataPrivate::slotFetchingPostingComplete( Syndication::Loader* loader, 
                                          Syndication::FeedPtr feed, Syndication::ErrorCode status ){
  bool success = false;

  if (status != Syndication::Success){
    emit parent->error( AtomAPI, i18n( "Could not get postings." ) );
    return;
  } 
  QList<Syndication::ItemPtr> items = feed->items();
  QList<Syndication::ItemPtr>::ConstIterator it = items.begin();
  QList<Syndication::ItemPtr>::ConstIterator end = items.end();
  for( ; it!=end; ++it ){
      BlogPosting posting;
      QRegExp rx( "post-(\\d+)" );
      if( rx.indexIn( ( *it )->id() )!=-1 && rx.cap(1)==getFetchPostingId() ){
        kDebug(5323)<<"QRegExp rx( 'post-(\\d+)' matches "<< rx.cap(1) << endl;
        posting.setPostingId( rx.cap(1) );
        posting.setTitle( ( *it )->title() );
        posting.setContent( ( *it )->content() );
        // FIXME: assuming UTC for now
        posting.setCreationDateTime( KDateTime( QDateTime::fromTime_t( ( *it )->datePublished() ),
                                                         KDateTime::Spec::UTC() ) ); 
        posting.setModificationDateTime( KDateTime( QDateTime::fromTime_t( ( *it )->dateUpdated() ), 
                                                              KDateTime::Spec::UTC() ) );
        emit parent->fetchedPosting( posting );
        success = true;
        kDebug(5323) << "Emitting fetchedPosting( postingId=" << posting.postingId() << " ); " << endl;
      }
  }
  if(!success){
    emit parent->error( Other, i18n( "Could not regexp the blog id path." ) );
    kDebug(5323) << "QRegExp rx( 'post-(\\d+)' does not match " << mFetchPostingId << ". " << endl;
  }
  setFetchPostingId( "" );
}

#include "gdata_p.moc"
