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

#include <QtCore/QByteArray>
#include <QtCore/QRegExp>
#include <QtGui/QWidget>

using namespace KBlog;

APIGData::APIGDataPrivate::APIGDataPrivate(){

}

APIGData::APIGDataPrivate::~APIGDataPrivate(){

}

void APIGData::APIGDataPrivate::getIntrospection(){
  // fetch the introspection file synchronously and parse it
  QByteArray data;
  KIO::Job *job = KIO::get( parent->url(), false, false ); 
  if ( KIO::NetAccess::synchronousRun( job, (QWidget*)0, &data, &parent->url() ) ) {
    kDebug() << "Fetched Homepage data." << endl;
    QRegExp pp( "<link.+rel=\"service.post\".+href=\"(.+)\".*/>" );
    if( pp.indexIn( data )!=-1 )
       mCreatePostingsPath = pp.cap(1);
    else
       emit parent->error( Other, i18n( "Could not regexp the service.post path." ) );
    kDebug(5323)<<"QRegExp pp( \"<link.+rel=\"service.post\".+href=\"(.+)\".*/>\" ) matches "<< pp.cap(1) << endl;
  }

  QRegExp bid( "<link.+blogID=(\\d+).*/>" );
  if( bid.indexIn( data )!=-1 )
     parent->setBlogId( bid.cap(1) );
  else
    emit parent->error( Other, i18n( "Could not regexp the blogID path." ) );
  kDebug(5323)<<"QRegExp bid( '<link.+blogID=(\\d+).*/>' matches "<< bid.cap(1) << endl;
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
      QString id;
      if( rx.indexIn( ( *it )->id() )!=-1 )
         id = rx.cap(1);
      else
         emit parent->error( Other, i18n( "Could not regexp the blog id path." ) );
      kDebug(5323)<<"QRegExp rx( 'blog-(\\d+)' matches "<< rx.cap(1) << endl;
      QString name = ( *it )->title();

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
      if( rx.indexIn( ( *it )->id() )!=-1 )
         posting.setPostingId( rx.cap(1) );
      else
         emit parent->error( Other, i18n( "Could not regexp the blog id path." ) );
      kDebug(5323)<<"QRegExp rx( 'post-(\\d+)' matches "<< rx.cap(1) << endl;
      posting.setTitle( ( *it )->title() );
      posting.setContent( ( *it )->content() );
      // FIXME: assuming UTC for now
      posting.setCreationDateTime( KDateTime( QDateTime::fromTime_t( ( *it )->datePublished() ), KDateTime::Spec::UTC() ) ); 
      posting.setModificationDateTime( KDateTime( QDateTime::fromTime_t( ( *it )->dateUpdated() ), KDateTime::Spec::UTC() ) );

      emit parent->fetchedPosting( posting );
      kDebug(5323) << "Emitting fetchedPosting( postingId=" << posting.postingId() << " ); " << endl;
  }
  kDebug(5323) << "Emitting listPostingsFinished()" << endl;
  emit parent->listPostingsFinished();
}

void APIGData::APIGDataPrivate::slotFetchingPostingComplete( Syndication::Loader* loader, 
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
      if( rx.indexIn( ( *it )->id() )!=-1 && rx.cap(1)==mFetchPostingId ){
         posting.setPostingId( rx.cap(1) );
         posting.setTitle( ( *it )->title() );
         posting.setContent( ( *it )->content() );
         // FIXME: assuming UTC for now
         posting.setCreationDateTime( KDateTime( QDateTime::fromTime_t( ( *it )->datePublished() ),
                                                          KDateTime::Spec::UTC() ) ); 
         posting.setModificationDateTime( KDateTime( QDateTime::fromTime_t( ( *it )->dateUpdated() ), 
                                                               KDateTime::Spec::UTC() ) );
         mFetchPostingId=QString(); //HACK
      }
      else
         emit parent->error( Other, i18n( "Could not regexp the blog id path." ) );
      kDebug(5323)<<"QRegExp rx( 'post-(\\d+)' matches "<< rx.cap(1) << endl;

      emit parent->listedPosting( posting );
      kDebug(5323) << "Emitting listedPosting( postingId=" << posting.postingId() << " ); " << endl;
  }
}

#include "gdata_p.moc"