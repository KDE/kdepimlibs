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

#include <metaweblog.h>
#include <kxmlrpcclient/client.h>
#include <kdebug.h>
#include <klocale.h>

#include <metaweblog_p.h>

#include <QtCore/QList>

using namespace KBlog;

APIMetaWeblog::APIMetaWeblogPrivate::APIMetaWeblogPrivate()
{
  mXmlRpcClient = 0;
}

APIMetaWeblog::APIMetaWeblogPrivate::~APIMetaWeblogPrivate()
{
  delete mXmlRpcClient;  
}

QList<QVariant> APIMetaWeblog::APIMetaWeblogPrivate::defaultArgs( const QString &id )
{
  QList<QVariant> args;
//  args << QVariant( QString( "0123456789ABCDEF" ) ); TODO remove
  if ( !id.isNull() ) {
    args << QVariant( id );
  }
  args << QVariant( parent->username() )
       << QVariant( parent->password() );
  return args;
}

void APIMetaWeblog::APIMetaWeblogPrivate::slotListCategories( const QList<QVariant> &result, const QVariant &id ){ 
  kDebug() << "APIMetaWeblogPrivate::slotListCategories" << endl;
  kDebug () << "TOP: " << result[0].typeName() << endl;
  if( result[ 0 ].type()!=8 ){
    kDebug () << "Could not list categories out of the result from the server." << endl;
    emit parent->error( i18n("Could not list categories out of the result from the server." ) );
  }
  else {
    const QMap<QString, QVariant> categories = result[0].toMap();
    const QList<QString> categoryNames = categories.keys();

    QList<QString>::ConstIterator it = categoryNames.begin();
    QList<QString>::ConstIterator end = categoryNames.end();
    for ( ; it != end; ++it ) {
      kDebug () << "MIDDLE: " << ( *it ) << endl;
      const QString name( *it );
      const QMap<QString, QVariant> category = categories[ *it ].toMap();
      const QString description( category["description"].toString() );
      if (  !name.isEmpty() ) {
        emit parent->categoryInfoRetrieved( name, description );
        kDebug()<< "Emitting categorieInfoRetrieved( name=" << name << " description=" << description << " ); " << endl;
      }
    }
  }
  kDebug() << "Emitting fetchingCategoriesFinished()" << endl;
  emit parent->listCategoriesFinished();
}

void APIMetaWeblog::APIMetaWeblogPrivate::slotListPostings( const QList<QVariant> &result, const QVariant &id )
{
  kDebug(5800)<<"APIMetaWeblog::slotListPostings"<<endl;
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;
  if( result[ 0 ].type()!=9 ){
    kDebug () << "Could not fetch list of postings out of the result from the server." << endl;
    emit parent->error( i18n("Could not fetch list of postings out of the result from the server." ) );
  }
  else {
    const QList<QVariant> postReceived = result[ 0 ].toList();
    QList<QVariant>::ConstIterator it = postReceived.begin();
    QList<QVariant>::ConstIterator end = postReceived.end();
    for ( ; it != end; ++it ) {
      BlogPosting posting;
      kDebug () << "MIDDLE: " << ( *it ).typeName() << endl;
      const QMap<QString, QVariant> postInfo = ( *it ).toMap();
      if ( readPostingFromMap( &posting, postInfo ) ) {
        kDebug() << "Emitting listedPosting( posting.postingId()=" <<posting.postingId() << "); " << endl;
        emit parent->listedPosting( posting ); // KUrl( posting.postingId() ) );
      } else {
        kDebug() << "d->readPostingFromMap failed! " << endl;
        emit parent->error( i18n("Couldn't read posting.") );
      }
    }
  } //FIXME should we emit here? (see below, too)
  kDebug() << "Emitting listPostingsFinished()" << endl;
  emit parent->listPostingsFinished();
}

void APIMetaWeblog::APIMetaWeblogPrivate::slotFetchPosting( const QList<QVariant> &result, const QVariant &id )
{
  kDebug(5800)<<"APIMetaWeblog::slotFetchPosting"<<endl;
  //array of structs containing ISO.8601 dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;
  if( result[ 0 ].type()!=8 ){
    kDebug () << "Could not fetch posting out of the result from the server." << endl;
    emit parent->error( i18n("Could not fetch posting out of the result from the server." ) );
  }
  else {
//     const QList<QVariant> postReceived = result[ 0 ].toList();
//     QList<QVariant>::ConstIterator it = postReceived.begin();
    BlogPosting posting;
    const QMap<QString, QVariant> postInfo = result[ 0 ].toMap();
    if ( readPostingFromMap( &posting, postInfo ) ) {
      kDebug() << "Emitting fetchedPosting( posting.postingId()=" <<posting.postingId() << "); " << endl;
      emit parent->fetchedPosting( posting ); // KUrl( posting.posingtId() ) );
    } else {
      kDebug() << "d->readPostingFromMap failed! " << endl;
      emit parent->error( i18n("Could not read posting.") );
    }
  }
}

void APIMetaWeblog::APIMetaWeblogPrivate::slotCreatePosting( const QList<QVariant> &result, const QVariant &id )
{
  kDebug()<<"APIMetaWeblog::slotCreatePosting"<<endl;
  //array of structs containing ISO.8601 dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;
  if( result[ 0 ].type()!=2 ){
    kDebug () << "Invalid XML format in response from server. Not an integer." << endl;
    emit parent->error( i18n( "Invalid XML format in response from server. Not an integer." ) );
  }
  else {
    emit parent->createdPosting( result[ 0 ].toInt() );
    kDebug() << "emitting createdPosting( " << result[ 0 ].toInt() << " )" << endl;
  }
}

void APIMetaWeblog::APIMetaWeblogPrivate::slotModifyPosting( const QList<QVariant> &result, const QVariant &id )
{
  kDebug()<<"APIMetaWeblog::slotModifyPosting"<<endl;
  //array of structs containing ISO.8601 dateCreated, String userid, String postid, String content;
  // TODO: Time zone for the dateCreated!
  kDebug () << "TOP: " << result[ 0 ].typeName() << endl;
  if( result[ 0 ].type()!=1 ){
    kDebug () << "Invalid XML format in response from server. Not a boolean." << endl;
    emit parent->error( i18n( "Invalid XML format in response from server. Not a boolean." ) );
  }
  else {
    emit parent->modifiedPosting( result[ 0 ].toBool() );
    kDebug() << "emitting modifiedPosting( " << result[ 0 ].toBool() << " )" << endl;
  }
}

void APIMetaWeblog::APIMetaWeblogPrivate::slotCreateMedia( const QList<QVariant> &result, const QVariant &id ){
  kDebug() << "APIMetaWeblogPrivate::slotCreateMedia, no error!" << endl;
  kDebug () << "TOP: " << result[0].typeName() << endl;
  if( result[ 0 ].type()!=8 ){
    kDebug () << "Invalid XML format in response from server. Not a map." << endl;
    emit parent->error( i18n("Invalid XML format in response from server. Not a map." ) );
  }
  else {
  const QMap<QString, QVariant> resultStruct = result[0].toMap();
  const QString url = resultStruct["url"].toString();
  kDebug() << "APIMetaWeblog::slotCreateMedia url="<< url << endl;

  if (  !url.isEmpty() ) {
    emit parent->mediaInfoRetrieved( url );
    kDebug()<< "Emitting mediaInfoRetrieved( url=" << url  << " ); " << endl;
  }
}
}

void APIMetaWeblog::APIMetaWeblogPrivate::faultSlot( int number, const QString& errorString, const QVariant& id )
{
  emit parent->error( errorString );
}

bool APIMetaWeblog::APIMetaWeblogPrivate::readPostingFromMap( BlogPosting *post, const QMap<QString, QVariant> &postInfo )
{
 // FIXME: integrate error handling
  if ( !post ) return false;
  QStringList mapkeys = postInfo.keys();
  kDebug() << endl << "Keys: " << mapkeys.join(", ") << endl << endl;
  
  KDateTime dt = KDateTime( postInfo[ "dateCreated" ].toDateTime() );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt );
  }
  dt = KDateTime( postInfo[ "lastModified" ].toDateTime() );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt );
  }

  post->setPostingId( postInfo[ "postid" ].toString() );

  QString title( postInfo[ "title" ].toString() );
  QString description( postInfo[ "description" ].toString() );
  QList<QVariant> categories( postInfo[ "categories" ].toList() );

  post->setTitle( title );
  post->setContent( description );
  if ( !categories.isEmpty() ){
    QString category = ( *categories.begin() ).toString();
    kDebug() << "Category: " <<  category  << endl;
    post->setCategory( category );
  }
  return true;
}

#include "metaweblog_p.moc"
