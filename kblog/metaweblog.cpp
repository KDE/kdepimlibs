/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
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

#include "metaweblog.h"
#include "metaweblog_p.h"
#include "blogpost.h"
#include "blogmedia.h"

#include <kxmlrpcclient/client.h>
#include <KDebug>
#include <KLocale>
#include <KDateTime>

using namespace KBlog;

MetaWeblog::MetaWeblog( const KUrl &server, QObject *parent )
  : Blogger1( server, *new MetaWeblogPrivate, parent )
{
  kDebug() << "MetaWeblog()";
}

MetaWeblog::MetaWeblog( const KUrl &server, MetaWeblogPrivate &dd, QObject *parent )
  : Blogger1( server, dd, parent )
{
  kDebug() << "MetaWeblog()";
}

MetaWeblog::~MetaWeblog()
{
  kDebug() << "~MetaWeblog()";
}

QString MetaWeblog::interfaceName() const
{
  return QLatin1String( "MetaWeblog" );
}

void MetaWeblog::listCategories()
{
    Q_D( MetaWeblog );
    kDebug() << "Fetching List of Categories...";
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    d->mXmlRpcClient->call(
      "metaWeblog.getCategories", args,
      this, SLOT(slotListCategories(const QList<QVariant>&, const QVariant&)),
      this, SLOT(slotError(int, const QString&, const QVariant&)) );
}

void MetaWeblog::createMedia( KBlog::BlogMedia *media )
{
  Q_D( MetaWeblog );
  if ( !media ) {
    kError() << "MetaWeblog::createMedia: media is a null pointer";
    emit error ( Other, i18n( "Media is a null pointer." ) );
    return;
  }
  unsigned int i = d->mCallMediaCounter++;
  d->mCallMediaMap[ i ] = media;
  kDebug() << "MetaWeblog::createMedia: name="<< media->name();
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  QMap<QString, QVariant> map;
  QList<QVariant> list;
  map["name"] = media->name();
  map["type"] = media->mimetype();
  map["bits"] = media->data();
  args << map;
  d->mXmlRpcClient->call(
    "metaWeblog.newMediaObject", args,
    this, SLOT(slotCreateMedia(const QList<QVariant>&,const QVariant&)),
    this, SLOT(slotError(int,const QString&,const QVariant&)),
    QVariant( i ) );

}

MetaWeblogPrivate::MetaWeblogPrivate()
{
  mCallMediaCounter=1;
}

MetaWeblogPrivate::~MetaWeblogPrivate()
{
  kDebug() << "~MetaWeblogPrivate()";
}

QList<QVariant> MetaWeblogPrivate::defaultArgs( const QString &id )
{
  Q_Q( MetaWeblog );
  QList<QVariant> args;
  if( !id.isEmpty() ) {
    args << QVariant( id );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

void MetaWeblogPrivate::slotListCategories( const QList<QVariant> &result,
                                            const QVariant &id )
{
  Q_Q( MetaWeblog );
  Q_UNUSED( id );

  QList<QMap<QString,QString> > categoriesList;

  kDebug() << "MetaWeblogPrivate::slotListCategories";
  kDebug() << "TOP:" << result[0].typeName();
  if ( result[0].type() != QVariant::Map &&
       result[0].type() != QVariant::List ) {
    // include fix for not metaweblog standard compatible apis with
    // array of structs instead of struct of structs, e.g. wordpress
    kError() << "Could not list categories out of the result from the server.";
    emit q->error( MetaWeblog::ParsingError,
                        i18n( "Could not list categories out of the result "
                              "from the server." ) );
  } else {
    if ( result[0].type() == QVariant::Map ) {
      const QMap<QString, QVariant> serverMap = result[0].toMap();
      const QList<QString> serverKeys = serverMap.keys();

      QList<QString>::ConstIterator it = serverKeys.begin();
      QList<QString>::ConstIterator end = serverKeys.end();
      for ( ; it != end; ++it ) {
        kDebug() << "MIDDLE:" << ( *it );
        QMap<QString,QString> category;
        const QMap<QString, QVariant> serverCategory = serverMap[*it].toMap();
        category["name"]= ( *it );
        category["description"] = serverCategory[ "description" ].toString();
        category["htmlUrl"] = serverCategory[ "htmlUrl" ].toString();
        category["rssUrl"] = serverCategory[ "rssUrl" ].toString();
        categoriesList.append( category );
        }
        emit q->listedCategories( categoriesList );
        kDebug() << "Emitting listedCategories";
      }
    }
    if ( result[0].type() == QVariant::List ) {
      // include fix for not metaweblog standard compatible apis with
      // array of structs instead of struct of structs, e.g. wordpress
      const QList<QVariant> serverList = result[0].toList();
      QList<QVariant>::ConstIterator it = serverList.begin();
      QList<QVariant>::ConstIterator end = serverList.end();
      for ( ; it != end; ++it ) {
        kDebug() << "MIDDLE:" << ( *it ).typeName();
        QMap<QString,QString> category;
        const QMap<QString, QVariant> serverCategory = ( *it ).toMap();
        category[ "name" ] = serverCategory["categoryName"].toString();
        category["description"] = serverCategory[ "description" ].toString();
        category["htmlUrl"] = serverCategory[ "htmlUrl" ].toString();
        category["rssUrl"] = serverCategory[ "rssUrl" ].toString();
        categoriesList.append( category );
      }
      kDebug() << "Emitting listedCategories()";
      emit q->listedCategories( categoriesList );
    }
  }

void MetaWeblogPrivate::slotCreateMedia( const QList<QVariant> &result,
                                         const QVariant &id )
{
  Q_Q( MetaWeblog );

  KBlog::BlogMedia *media = mCallMediaMap[ id.toInt() ];
  mCallMediaMap.remove( id.toInt() );

  kDebug() << "MetaWeblogPrivate::slotCreateMedia, no error!";
  kDebug() << "TOP:" << result[0].typeName();
  if ( result[0].type() != 8 ) {
    kError() << "Could not read the result, not a map.";
    emit q->errorMedia( MetaWeblog::ParsingError,
                        i18n( "Could not read the result, not a map." ),
                        media );
  } else {
    const QMap<QString, QVariant> resultStruct = result[0].toMap();
    const QString url = resultStruct["url"].toString();
    kDebug() << "MetaWeblog::slotCreateMedia url=" << url;

    if ( !url.isEmpty() ) {
      media->setUrl( KUrl( url ) );
      media->setStatus( BlogMedia::Created );
      emit q->createdMedia( media );
      kDebug() << "Emitting createdMedia( url=" << url  << ");";
    }
  }
}

bool MetaWeblogPrivate::readPostFromMap( BlogPost *post,
                                                        const QMap<QString, QVariant> &postInfo )
{
  // FIXME: integrate error handling
  kDebug() << "readPostFromMap()";
  if ( !post ) {
    return false;
  }
  QStringList mapkeys = postInfo.keys();
  kDebug() << endl << "Keys:" << mapkeys.join( ", " );
  kDebug() << endl;

  KDateTime dt =
    KDateTime( postInfo["dateCreated"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt );
  }

  dt =
    KDateTime( postInfo["lastModified"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt );
  }

  post->setPostId( postInfo["postid"].toString() );

  QString title( postInfo["title"].toString() );
  QString description( postInfo["description"].toString() );
  QStringList categories( postInfo["categories"].toStringList() );

  post->setTitle( title );
  post->setContent( description );
  if ( !categories.isEmpty() ){
    kDebug() << "Categories:" << categories;
    post->setCategories( categories );
  }
  return true;
}

bool MetaWeblogPrivate::readArgsFromPost( QList<QVariant> *args, const BlogPost &post )
{
  if ( !args ) {
    return false;
  }
  QMap<QString, QVariant> map;
  map["categories"] = post.categories();
  map["description"] = post.content();
  map["title"] = post.title();
  map["lastModified"] = post.modificationDateTime().toUtc().dateTime();
  map["dateCreated"] = post.creationDateTime().toUtc().dateTime();
  *args << map;
  *args << QVariant( !post.isPrivate() );
  return true;
}

QString MetaWeblogPrivate::getCallFromFunction( FunctionToCall type )
{
  switch ( type ) {
    case GetRecentPosts: return "metaWeblog.getRecentPosts";
    case CreatePost:        return "metaWeblog.newPost";
    case ModifyPost:       return "metaWeblog.editPost";
    case FetchPost:        return "metaWeblog.getPost";
    default: return QString();
  }
}
#include "metaweblog.moc"
