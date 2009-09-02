/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2009 Christian Weilbach <christian_weilbach@web.de>
  Copyright (c) 2007-2008 Mike Arthur <mike@mikearthur.co.uk>

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

#include "movabletype.h"
#include "movabletype_p.h"
#include "blogpost.h"

#include <kxmlrpcclient/client.h>
#include <kio/job.h>

#include <KDebug>
#include <KLocale>
#include <KDateTime>

#include <QtCore/QStringList>

using namespace KBlog;

MovableType::MovableType( const KUrl &server, QObject *parent )
  : MetaWeblog( server, *new MovableTypePrivate, parent )
{
  kDebug();
}

MovableType::MovableType( const KUrl &server, MovableTypePrivate &dd,
                        QObject *parent )
  : MetaWeblog( server, dd, parent )
{
  kDebug();
  listCategories();
}

MovableType::~MovableType()
{
  kDebug();
}

QString MovableType::interfaceName() const
{
  return QLatin1String( "Movable Type" );
}

void MovableType::listRecentPosts( int number )
{
    Q_D( MovableType );
    kDebug();
    QList<QVariant> args( d->defaultArgs( blogId() ) );
    args << QVariant( number );
    d->mXmlRpcClient->call(
      "metaWeblog.getRecentPosts", args,
      this, SLOT(slotListRecentPosts(const QList<QVariant>&,const QVariant&)),
      this, SLOT(slotError(int,const QString&,const QVariant&)),
      QVariant( number ) );
}

void MovableType::listTrackBackPings( KBlog::BlogPost *post )
{
  Q_D( MovableType );
  kDebug();
  QList<QVariant> args;
  args << QVariant( post->postId() );
  unsigned int i = d->mCallCounter++;
  d->mCallMap[ i ] = post;
  d->mXmlRpcClient->call(
    "mt.getTrackbackPings", args,
    this, SLOT(slotListTrackbackPings(const QList<QVariant>&,const QVariant&)),
    this, SLOT(slotError(int,const QString&,const QVariant&)),
    QVariant( i ) );
}

void MovableType::createPost( BlogPost *post )
{
  // reimplemented because we do this:
  // http://comox.textdrive.com/pipermail/wp-testers/2005-July/000284.html
  kDebug();
  Q_D( MovableType );

  // we need mCategoriesList to be loaded first, since we cannot use the post->categories()
  // names later, but we need to map them to categoryId of the blog
  if(d->mCategoriesList.isEmpty()){
    kDebug() << "No categories in the cache yet. Have to fetch them first.";
    d->mCreatePostCache << post;
    connect(this,SIGNAL(listedCategories(const QList<QMap<QString,QString> >&)),
            this,SLOT(slotTriggerCreatePost()));
    listCategories();
  }
  else {
    bool publish = post->isPrivate();
    // If we do setPostCategories() later than we disable publishing first.
    if( post->categories().count()>1 ){
      post->setPrivate( true );
    }
    MetaWeblog::createPost( post );
    // HACK: uuh this a bit ugly now... reenable the original publish argument,
    // since createPost should have parsed now
    post->setPrivate(publish);
  }
}

void MovableTypePrivate::slotTriggerCreatePost()
{
  kDebug();
  Q_Q( MovableType );

  q->disconnect(q,SIGNAL(listedCategories(const QList<QMap<QString,QString> >&)),
          q,SLOT(slotTriggerCreatePost()));
  // now we can recall createPost with the posts from the cache
  for(int i=0;i<mCreatePostCache.count();i++){
    q->createPost(mCreatePostCache[i]);
  }
  mCreatePostCache.clear();
}

MovableTypePrivate::MovableTypePrivate()
{
  kDebug();
}

MovableTypePrivate::~MovableTypePrivate()
{
  kDebug();
}

void MovableTypePrivate::slotCreatePost( const QList<QVariant> &result, const QVariant &id )
{
  kDebug();
  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  MetaWeblogPrivate::slotCreatePost( result, id );
  // set the categories and publish afterwards
  if( post->categories().count() > 1 ){
    setPostCategories( post, true );
  }
}

void MovableTypePrivate::slotModifyPost( const QList<QVariant> &result, const QVariant &id )
{
  kDebug();
  KBlog::BlogPost *post = mCallMap[ id.toInt() ];
  MetaWeblogPrivate::slotModifyPost( result, id );
  if( post->categories().count() > 1 ){
    setPostCategories( post, false );
  }
}

void MovableTypePrivate::setPostCategories( BlogPost *post, bool publishAfterCategories )
{
  kDebug();
  Q_Q( MovableType );

  unsigned int i = mCallCounter++;
  mCallMap[ i ] = post;
  mPublishAfterCategories[ i ] = publishAfterCategories;
  QList<QVariant> args( defaultArgs( post->postId() ) );

  // map the categoryId of the server to the name
  QStringList categories = post->categories();
  for( int j=0; j<=categories.count(); j++ ){
     for( int k=0; k<=mCategoriesList.count(); k++ ){
       if(mCategoriesList[k]["name"]==categories[j]){
         kDebug() << "Matched category with name: " << categories[ j ];
         QMap<QString,QVariant> category;
         //the first in the QStringList of post->categories()
         // is the primary category
         category["categoryId"]=mCategoriesList[k]["categoryId"];
         args<<QVariant(category);
         break;
       }
       if(k==mCategoriesList.count()){
         kDebug() << "Couldn't find categoryId for: " << categories[j];
       }
     }
  }

  mXmlRpcClient->call(
    "mt.setPostCategories", args,
    q, SLOT(slotSetPostCategories(const QList<QVariant>&,const QVariant&)),
    q, SLOT(slotError(int, const QString&,const QVariant&)),
    QVariant( i ) );
}

void MovableTypePrivate::slotSetPostCategories(const QList<QVariant>& result,const QVariant& id)
{
  kDebug();
  Q_Q( MovableType );

  int i = id.toInt();
  BlogPost* post = mCallMap[ i ];
  bool publish = mPublishAfterCategories[ i ];
  mCallMap.remove(i);
  mPublishAfterCategories.remove(i);

  if ( result[0].type() != QVariant::Bool ) {
    kError() << "Could not read the result, not a boolean. Category setting failed! We will still publish if now if necessary. ";
    emit q->errorPost( Blogger1::ParsingError,
                          i18n( "Could not read the result, not a boolean. Category setting failed! We will still publish now if necessary." ),
                          post );
  }
  // Finally publish now, if the post was meant to be published in the beginning.
  // The first boolean is necessary to only publish if the post is created, not
  // modified.
  if( publish && !post->isPrivate() ){
    q->modifyPost( post );
  }
}

QList<QVariant> MovableTypePrivate::defaultArgs( const QString &id )
{
  Q_Q( MovableType );
  QList<QVariant> args;
  if( !id.isEmpty() ) {
    args << QVariant( id );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

bool MovableTypePrivate::readPostFromMap( BlogPost *post, const QMap<QString, QVariant> &postInfo )
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
    post->setCreationDateTime( dt.toLocalZone() );
  }

  dt =
    KDateTime( postInfo["lastModified"].toDateTime(), KDateTime::UTC );
  if ( dt.isValid() && !dt.isNull() ) {
    post->setModificationDateTime( dt.toLocalZone() );
  }

  post->setPostId( postInfo["postid"].toString() );

  QString title( postInfo["title"].toString() );
  QString description( postInfo["description"].toString() );
  QStringList categories( postInfo["categories"].toStringList() );
  //TODO 2 new keys are:
  // String mt_convert_breaks, the value for the convert_breaks field
  post->setSlug( postInfo["wp_slug"].toString() );
  post->setAdditionalContent( postInfo["mt_text_more"].toString() );
  post->setTitle( title );
  post->setContent( description );
  post->setCommentAllowed( (bool)postInfo["mt_allow_comments"].toInt() );
  post->setTrackBackAllowed( (bool)postInfo["mt_allow_pings"].toInt() );
  post->setSummary( postInfo["mt_excerpt"].toString() );
  post->setTags( postInfo["mt_keywords"].toStringList() );
  post->setLink( postInfo["link"].toString() );
  post->setPermaLink( postInfo["permaLink"].toString() );
  QString postStatus = postInfo["post_status"].toString();
  if( postStatus != "publish" && !postStatus.isEmpty() ){
    /**
     * Maybe this field wasn't set by server! so, on that situation, we will assume it as non-Private,
     * The postStatus.isEmpty() check is for that!
     * I found this field on Wordpress output! it's value can be: publish, private, draft (as i see)
    */
    post->setPrivate(true);
  }
  if ( !categories.isEmpty() ){
    kDebug() << "Categories:" << categories;
    post->setCategories( categories );
  }
  return true;
}

void MovableTypePrivate::slotListTrackBackPings(
    const QList<QVariant> &result, const QVariant &id )
{
  Q_Q( MovableType );
  kDebug() << "slotTrackbackPings()";
  BlogPost *post = mCallMap[ id.toInt() ];
  mCallMap.remove( id.toInt() );
  QList<QMap<QString,QString> > trackBackList;
  if ( result[0].type() != QVariant::List ) {
    kError() << "Could not fetch list of trackback pings out of the"
                 << "result from the server.";
    emit q->error( MovableType::ParsingError,
                   i18n( "Could not fetch list of trackback pings out of the "
                         "result from the server." ) );
    return;
  }
  const QList<QVariant> trackBackReceived = result[0].toList();
  QList<QVariant>::ConstIterator it = trackBackReceived.begin();
  QList<QVariant>::ConstIterator end = trackBackReceived.end();
  for ( ; it != end; ++it ) {
    QMap<QString,QString> tping;
    kDebug() << "MIDDLE:" << ( *it ).typeName();
    const QMap<QString, QVariant> trackBackInfo = ( *it ).toMap();
    tping[ "title" ] = trackBackInfo[ "pingTitle"].toString();
    tping[ "url" ] = trackBackInfo[ "pingURL"].toString();
    tping[ "ip" ] = trackBackInfo[ "pingIP"].toString();
    trackBackList << tping;
  }
  kDebug() << "Emitting listedTrackBackPings()";
  emit q->listedTrackBackPings( post, trackBackList );
}

bool MovableTypePrivate::readArgsFromPost( QList<QVariant> *args, const BlogPost &post )
{
  //TODO 2 new keys are:
  // String mt_convert_breaks, the value for the convert_breaks field
  // array mt_tb_ping_urls, the list of TrackBack ping URLs for this entry
  if ( !args ) {
    return false;
  }
  QMap<QString, QVariant> map;
  map["categories"] = post.categories();
  map["description"] = post.content();
  if( !post.additionalContent().isEmpty() )
    map["mt_text_more"] = post.additionalContent();
  map["title"] = post.title();
  map["dateCreated"] = post.creationDateTime().toUtc().dateTime();
  map["mt_allow_comments"] = (int)post.isCommentAllowed();
  map["mt_allow_pings"] = (int)post.isTrackBackAllowed();
  map["mt_excerpt"] = post.summary();
  map["mt_keywords"] = post.tags().join(",");
  //map["mt_tb_ping_urls"] check for that, i think this should only be done on the server.
  *args << map;
  *args << QVariant( !post.isPrivate() );
  return true;
}

// void MovableType::setPostCategories(const QString &postId, const QMap<QString, bool> &categoriesList)
// {
//   Q_D( MovableType );
//   kDebug();
//   int count = categoriesList.count();
//   if(count < 1){
//     kError() << "MovableType::setPostCategories: the category list is empty";
//     emit error ( Other, i18n( "The category list is empty" ) );
//     return;
//   }
// 
//   kDebug() << "Setting"<< count<<"Categories to Post "<< postId<<" with blogId" << blogId();
// 
//   QString xmlMarkup = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
//   xmlMarkup += "<methodCall>";
//   xmlMarkup += "<methodName>mt.setPostCategories</methodName>";
//   xmlMarkup += "<params><param>";
//   xmlMarkup += "<value><string><![CDATA["+postId+"]]></string></value>";
//   xmlMarkup += "</param>";
//   xmlMarkup += "<param>";
//   xmlMarkup += "<value><string><![CDATA["+username()+"]]></string></value>";
//   xmlMarkup += "</param><param>";
//   xmlMarkup += "<value><string><![CDATA["+password()+"]]></string></value>";
//   xmlMarkup += "</param>";
//   xmlMarkup += "<param><value><array><data>";
// 
//   QMap<QString, bool>::ConstIterator it = categoriesList.constBegin();
//   QMap<QString, bool>::ConstIterator end = categoriesList.constEnd();
//   for ( ; it != end; ++it ){
//     xmlMarkup += "<value><struct><member><name>categoryId</name><value><string><![CDATA[" +
//     ( it.key() ) + "]]></string></value></member>";
//     xmlMarkup += "<member><name>isPrimary</name><value><boolean><![CDATA[" +
//     ( QString::number(QVariant(it.value()).toInt()) ) +
//                  "]]></boolean></value></member>""</struct></value>";
//   }
// 
//   xmlMarkup += "</data></array></value></param></params></methodCall>";
// 
//   QByteArray postData;
//   QDataStream stream( &postData, QIODevice::WriteOnly );
//   stream.writeRawData( xmlMarkup.toUtf8(), xmlMarkup.toUtf8().length() );
// 
//   KIO::TransferJob *job = KIO::http_post( url(), postData, KIO::HideProgressInfo );
// 
//   d->mSetPostCategoriesMap[ job ] = postId;
// 
//   if ( !job ) {
//     kWarning() << "Failed to create job for: " << url().url();
//   }
// 
//   job->addMetaData( "content-type", "Content-Type: text/xml; charset=utf-8" );
//   job->addMetaData( "ConnectTimeout", "50" );
//   job->addMetaData( "UserAgent", userAgent() );
// 
//   connect( job, SIGNAL(result(KJob *)),
//            this, SLOT(slotSetPostCategories(KJob *)) );
//   connect( job, SIGNAL(data(KIO::Job *,const QByteArray &)),
//            this, SLOT(slotSetPostCategoriesData(KIO::Job *,const QByteArray &)) );
// }
// 
// void MovableTypePrivate::slotSetPostCategories(KJob *job)
// {
//   kDebug();
//   const QString data = QString::fromUtf8( mSetPostCategoriesBuffer[ job ].data(),
//                                           mSetPostCategoriesBuffer[ job ].size() );
//   mSetPostCategoriesBuffer[ job ].resize( 0 );
// 
//   Q_Q( MovableType );
// 
//   QString postId = mSetPostCategoriesMap[ job ];
//   mSetPostCategoriesMap.remove( job );
// 
//   if ( job->error() != 0 ) {
//     kError() << "slotSetPostCategories error:" << job->errorString();
//     emit q->error( MovableType::Other, job->errorString());
//     return;
//   }
// 
//   QRegExp rxId( "<boolean>(.+)</boolean>" );
//   if ( rxId.indexIn( data ) == -1 ){
//     kError() << "Could not regexp the result out of the result:" << data;
//     emit q->error( MovableType::XmlRpc, i18n( "Could not regexp the result out of the result." ));
//     return;
//   }
//   kDebug() << "QRegExp rx( \"<boolean>(.+)</boolean>\" ) matches " << rxId.cap( 1 );
// 
//   if( rxId.cap( 1 ) == "1" ){
//     kDebug() << "Emitting settedPostCategories(const QString &postId)";
//     emit q->settedPostCategories( postId );
//   } else {
//     emit q->error( MovableType::Other, i18n( "Error on server, Post categories couldn't sets" ));
//   }
// }
// 
// void MovableTypePrivate::slotSetPostCategoriesData(KIO::Job *job,const QByteArray &data)
// {
//   kDebug();
//   mSetPostCategoriesBuffer[ job ].append( data );
// }

#include "movabletype.moc"
