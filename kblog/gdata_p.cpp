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
#include <QDomDocument>
#include <QtGui/QWidget>

using namespace KBlog;

APIGData::APIGDataPrivate::APIGDataPrivate(){

}

APIGData::APIGDataPrivate::~APIGDataPrivate(){

}

void APIGData::APIGDataPrivate::getIntrospection(){
  // fetch the introspection file synchronously and parse it
  QByteArray data;
  QString parsingError;
  int parsingErrorLine;
  QDomDocument homepage;
  bool mCreatePostingsPathSuccess = false;
  bool mFetchPostingsPathSuccess = false;
  KIO::Job *job = KIO::get( parent->url(), false, false ); 
  if ( KIO::NetAccess::synchronousRun( job, (QWidget*)0, &data, &parent->url() ) 
        && homepage.setContent( data, false, &parsingError, &parsingErrorLine ) ) {
    kDebug() << "Fetched Homepage data: " << data.data() << endl;
/*    QDomNodeList links = homepage.elementsByTagName( QString( "link" ) );
    for( int i=0; i<links.count(); i++){
      QDomNamedNodeMap attributes = links.item(i).toElement().attributes();
      if( attributes.namedItem( "rel" ).nodeValue() == QString( "service.post" ) ) {
        mCreatePostingsPath = attributes.namedItem( "href" ).nodeValue();
        kDebug() << "CreatePostingsPath: " << mCreatePostingsPath << endl;
        mCreatePostingsPathSuccess = true;
      }
      else if( attributes.namedItem( "type" ).nodeValue() == QString( "application/atom+xml" ) 
                    && attributes.namedItem( "rel" ).nodeValue() == QString( "alternate" )) {
        kDebug() << "FetchPostingsPath: " << mFetchPostingsPath << endl;
        mFetchPostingsPath = attributes.namedItem( "href" ).nodeValue();
        mFetchPostingsPath = true;
      }

    }*/
//     if( !mCreatePostingsPathSuccess || !mFetchPostingsPathSuccess )
      // TODO emit a fitting error
  }
  else{
    kDebug() << "Could not get and parse the dom document." << endl;
    kDebug() << "Fetched unparsable data: " << data.data() << endl;
    kDebug() << "Parsing error at line " << QString( "%1" ).arg( parsingErrorLine ) << ": " << parsingError << endl;
    // TODO emit a fitting error
  }
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
      rx.indexIn( ( *it )->id() );
      QString id = rx.cap(1);
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
      rx.indexIn( ( *it )->id() );
      kDebug(5323)<<"QRegExp rx( 'post-(\\d+)' matches "<< rx.cap(1) << endl;
      posting.setPostingId( rx.cap(1) );
      posting.setTitle( ( *it )->title() );
      posting.setContent( ( *it )->content() );
      // FIXME: assuming UTC for now
      posting.setCreationDateTime( KDateTime( QDateTime::fromTime_t( ( *it )->datePublished() ), KDateTime::Spec::UTC() ) ); 
      posting.setModificationDateTime( KDateTime( QDateTime::fromTime_t( ( *it )->dateUpdated() ), KDateTime::Spec::UTC() ) );

      emit parent->listedPosting( posting );
      kDebug(5323) << "Emitting listedPosting( postingId=" << posting.postingId() << "); " << endl;
  }
  kDebug(5323) << "Emitting listPostingsFinished()" << endl;
  emit parent->listPostingsFinished();
}

#include "gdata_p.moc"