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

#include <blogger2_p.h>
#include <kdebug.h>

#include <kio/netaccess.h>
#include <kio/http.h>

#include <QtCore/QByteArray>
#include <QtGui/QWidget>
#include <QDomDocument>

using namespace KBlog;

APIBlogger2::APIBlogger2Private::APIBlogger2Private(){

}

APIBlogger2::APIBlogger2Private::~APIBlogger2Private(){

}

void APIBlogger2::APIBlogger2Private::getIntrospection(){
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
    QDomNodeList links = homepage.elementsByTagName( QString( "link" ) );
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
    }
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

void APIBlogger2::APIBlogger2Private::slotLoadingPostingsComplete( Syndication::Loader* loader, 
                                          Syndication::FeedPtr feed, Syndication::ErrorCode status ){
  if (status != Syndication::Success)
         return;

//  emit parent->
}

#include "blogger2_p.moc"