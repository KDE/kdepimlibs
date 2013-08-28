/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
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

#include "metaweblog.h"
#include "metaweblog_p.h"
#include "blogpost.h"
#include "blogmedia.h"

#include <kxmlrpcclient/client.h>
#include <KDebug>
#include <KLocalizedString>
#include <KDateTime>
#include <kstandarddirs.h>

#include <QtCore/QFile>
#include <QtCore/QDataStream>

using namespace KBlog;

MetaWeblog::MetaWeblog( const KUrl &server, QObject *parent )
  : Blogger1( server, *new MetaWeblogPrivate, parent )
{
  kDebug();
}

MetaWeblog::MetaWeblog( const KUrl &server, MetaWeblogPrivate &dd, QObject *parent )
  : Blogger1( server, dd, parent )
{
  kDebug();
}

MetaWeblog::~MetaWeblog()
{
  kDebug();
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
      QLatin1String("metaWeblog.getCategories"), args,
      this, SLOT(slotListCategories(QList<QVariant>,QVariant)),
      this, SLOT(slotError(int,QString,QVariant)) );
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
  kDebug() << "MetaWeblog::createMedia: name=" << media->name();
  QList<QVariant> args( d->defaultArgs( blogId() ) );
  QMap<QString, QVariant> map;
  map[QLatin1String("name")] = media->name();
  map[QLatin1String("type")] = media->mimetype();
  map[QLatin1String("bits")] = media->data();
  args << map;
  d->mXmlRpcClient->call(
    QLatin1String("metaWeblog.newMediaObject"), args,
    this, SLOT(slotCreateMedia(QList<QVariant>,QVariant)),
    this, SLOT(slotError(int,QString,QVariant)),
    QVariant( i ) );

}

MetaWeblogPrivate::MetaWeblogPrivate()
{
  kDebug();
  mCallMediaCounter=1;
  mCatLoaded=false;
}

MetaWeblogPrivate::~MetaWeblogPrivate()
{
  kDebug();
}

QList<QVariant> MetaWeblogPrivate::defaultArgs( const QString &id )
{
  Q_Q( MetaWeblog );
  QList<QVariant> args;
  if ( !id.isEmpty() ) {
    args << QVariant( id );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

void MetaWeblogPrivate::loadCategories()
{
  kDebug();

  if ( mCatLoaded ) {
    return;
  }
  mCatLoaded = true;

  if ( mUrl.isEmpty() || mBlogId.isEmpty() || mUsername.isEmpty() ) {
    kDebug() << "We need at least url, blogId and the username to create a unique filename.";
    return;
  }

  QString filename = QLatin1String("kblog/") + mUrl.host() + QLatin1Char('_') + mBlogId + QLatin1Char('_') + mUsername;
  filename = KStandardDirs::locateLocal( "data", filename, true );

  QFile file( filename );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    kDebug() << "Cannot open cached categories file: " << filename;
    return;
  }

  QDataStream stream( &file );
  stream >> mCategoriesList;
  file.close();
}

void MetaWeblogPrivate::saveCategories()
{
  kDebug();
  if ( mUrl.isEmpty() || mBlogId.isEmpty() || mUsername.isEmpty() ) {
    kDebug() << "We need at least url, blogId and the username to create a unique filename.";
    return;
  }

  QString filename = QLatin1String("kblog/") + mUrl.host() + QLatin1Char('_') + mBlogId + QLatin1Char('_') + mUsername;
  filename = KStandardDirs::locateLocal( "data", filename, true );

  QFile file( filename );
  if ( !file.open( QIODevice::WriteOnly ) ) {
    kDebug() << "Cannot open cached categories file: " << filename;
    return;
  }

  QDataStream stream( &file );
  stream << mCategoriesList;
  file.close();
}

void MetaWeblogPrivate::slotListCategories( const QList<QVariant> &result,
                                            const QVariant &id )
{
  Q_Q( MetaWeblog );
  Q_UNUSED( id );

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
        category[QLatin1String("name")]= ( *it );
        category[QLatin1String("description")] = serverCategory[ QLatin1String("description") ].toString();
        category[QLatin1String("htmlUrl")] = serverCategory[ QLatin1String("htmlUrl") ].toString();
        category[QLatin1String("rssUrl")] = serverCategory[ QLatin1String("rssUrl") ].toString();
        category[QLatin1String("categoryId")] = serverCategory[ QLatin1String("categoryId") ].toString();
        category[QLatin1String("parentId")] = serverCategory[ QLatin1String("parentId") ].toString();
        mCategoriesList.append( category );
      }
      kDebug() << "Emitting listedCategories";
      emit q->listedCategories( mCategoriesList );
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
      category[ QLatin1String("name") ] = serverCategory[QLatin1String("categoryName")].toString();
      category[QLatin1String("description")] = serverCategory[ QLatin1String("description") ].toString();
      category[QLatin1String("htmlUrl")] = serverCategory[ QLatin1String("htmlUrl") ].toString();
      category[QLatin1String("rssUrl")] = serverCategory[ QLatin1String("rssUrl") ].toString();
      category[QLatin1String("categoryId")] = serverCategory[ QLatin1String("categoryId") ].toString();
      category[QLatin1String("parentId")] = serverCategory[ QLatin1String("parentId") ].toString();
      mCategoriesList.append( category );
    }
    kDebug() << "Emitting listedCategories()";
    emit q->listedCategories( mCategoriesList );
  }
  saveCategories();
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
    return;
  }
  const QMap<QString, QVariant> resultStruct = result[0].toMap();
  const QString url = resultStruct[QLatin1String("url")].toString();
  kDebug() << "MetaWeblog::slotCreateMedia url=" << url;

  if ( !url.isEmpty() ) {
    media->setUrl( KUrl( url ) );
    media->setStatus( BlogMedia::Created );
    kDebug() << "Emitting createdMedia( url=" << url  << ");";
    emit q->createdMedia( media );
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
  kDebug() << endl << "Keys:" << mapkeys.join( QLatin1String(", ") );
  kDebug() << endl;

  KDateTime dt =
    KDateTime( postInfo[QLatin1String("dateCreated")].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setCreationDateTime( dt.toLocalZone() );
  }

  dt =
    KDateTime( postInfo[QLatin1String("lastModified")].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt.toLocalZone() );
  }

  post->setPostId( postInfo[QLatin1String("postid")].toString().isEmpty() ? postInfo[QLatin1String("postId")].toString() :
                   postInfo[QLatin1String("postid")].toString() );

  QString title( postInfo[QLatin1String("title")].toString() );
  QString description( postInfo[QLatin1String("description")].toString() );
  QStringList categories( postInfo[QLatin1String("categories")].toStringList() );

  post->setTitle( title );
  post->setContent( description );
  if ( !categories.isEmpty() ) {
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
  map[QLatin1String("categories")] = post.categories();
  map[QLatin1String("description")] = post.content();
  map[QLatin1String("title")] = post.title();
  map[QLatin1String("lastModified")] = post.modificationDateTime().dateTime().toUTC();
  map[QLatin1String("dateCreated")] = post.creationDateTime().dateTime().toUTC();
  *args << map;
  *args << QVariant( !post.isPrivate() );
  return true;
}

QString MetaWeblogPrivate::getCallFromFunction( FunctionToCall type )
{
  switch ( type ) {
    case GetRecentPosts: return QLatin1String("metaWeblog.getRecentPosts");
    case CreatePost:        return QLatin1String("metaWeblog.newPost");
    case ModifyPost:       return QLatin1String("metaWeblog.editPost");
    case FetchPost:        return QLatin1String("metaWeblog.getPost");
    default: return QString();
  }
}
#include "moc_metaweblog.cpp"
