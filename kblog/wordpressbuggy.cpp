/*
  This file is part of the kblog library.

  Copyright (c) 2006-2009 Christian Weilbach <christian_weilbach@web.de>

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

#include "wordpressbuggy.h"
#include "wordpressbuggy_p.h"

#include "blogpost.h"

#include <KDebug>
#include <KLocalizedString>
#include <KDateTime>

#include <kio/http.h>
#include <kio/job.h>

#include <QtCore/QStringList>

using namespace KBlog;

WordpressBuggy::WordpressBuggy( const KUrl &server, QObject *parent )
  : MovableType( server, *new WordpressBuggyPrivate, parent )
{
  kDebug();
}

WordpressBuggy::WordpressBuggy( const KUrl &server, WordpressBuggyPrivate &dd,
                                QObject *parent )
  : MovableType( server, dd, parent )
{
  kDebug();
}

WordpressBuggy::~WordpressBuggy()
{
  kDebug();
}

void WordpressBuggy::createPost( KBlog::BlogPost *post )
{
  // reimplemented because we do this:
  // http://comox.textdrive.com/pipermail/wp-testers/2005-July/000284.html
  kDebug();
  Q_D( WordpressBuggy );

  // we need mCategoriesList to be loaded first, since we cannot use the post->categories()
  // names later, but we need to map them to categoryId of the blog
  d->loadCategories();
  if ( d->mCategoriesList.isEmpty() ) {
    kDebug() << "No categories in the cache yet. Have to fetch them first.";
    d->mCreatePostCache << post;
    connect( this,SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
             this,SLOT(slotTriggerCreatePost()) );
    listCategories();
  }
  else {
    kDebug() << "createPost()";
    if ( !post ) {
      kError() << "WordpressBuggy::createPost: post is a null pointer";
      emit error ( Other, i18n( "Post is a null pointer." ) );
      return;
    }
    kDebug() << "Creating new Post with blogId" << blogId();

    bool publish = post->isPrivate();
    // If we do setPostCategories() later than we disable publishing first.
    if ( !post->categories().isEmpty() ) {
      post->setPrivate( true );
      if ( d->mSilentCreationList.contains( post ) ) {
        kDebug() << "Post already in mSilentCreationList, this *should* never happen!";
      } else {
        d->mSilentCreationList << post;
      }
    }

    QString xmlMarkup = QLatin1String("<?xml version=\"1.0\"?>");
    xmlMarkup += QLatin1String("<methodCall>");
    xmlMarkup += QLatin1String("<methodName>metaWeblog.newPost</methodName>");
    xmlMarkup += QLatin1String("<params><param>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[")+blogId()+QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</param>");
    xmlMarkup += QLatin1String("<param>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[")+username()+QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</param><param>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[")+password()+QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</param>");
    xmlMarkup += QLatin1String("<param><struct>");
    xmlMarkup += QLatin1String("<member><name>description</name>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[")+post->content()+QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>title</name>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[")+post->title()+QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</member><member>");

    xmlMarkup += QLatin1String("<name>dateCreated</name>");
    xmlMarkup += QLatin1String("<value><dateTime.iso8601>") +
                 post->creationDateTime().dateTime().toUTC().toString( QLatin1String("yyyyMMddThh:mm:ss") ) +
                 QLatin1String("</dateTime.iso8601></value>");
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>mt_allow_comments</name>");
    xmlMarkup += QString::fromLatin1( "<value><int>%1</int></value>" ).arg( (int)post->isCommentAllowed() );
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>mt_allow_pings</name>");
    xmlMarkup += QString::fromLatin1( "<value><int>%1</int></value>" ).arg( (int)post->isTrackBackAllowed() );
    xmlMarkup += QLatin1String("</member><member>");
    if ( !post->additionalContent().isEmpty() ) {
      xmlMarkup += QLatin1String("<name>mt_text_more</name>");
      xmlMarkup += QLatin1String("<value><string><![CDATA[") + post->additionalContent() + QLatin1String("]]></string></value>");
      xmlMarkup += QLatin1String("</member><member>");
    }
    xmlMarkup += QLatin1String("<name>wp_slug</name>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[") + post->slug() + QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>mt_excerpt</name>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[") + post->summary() + QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>mt_keywords</name>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[") + post->tags().join(QLatin1String(",")) + QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</member></struct></param>");
    xmlMarkup += QLatin1String("<param><value><boolean>") +
                 QString::fromLatin1( "%1" ).arg( (int)(!post->isPrivate() ) ) +
                 QLatin1String("</boolean></value></param>");
    xmlMarkup += QLatin1String("</params></methodCall>");

    QByteArray postData;
    QDataStream stream( &postData, QIODevice::WriteOnly );
    stream.writeRawData( xmlMarkup.toUtf8(), xmlMarkup.toUtf8().length() );

    KIO::StoredTransferJob *job = KIO::storedHttpPost( postData, url(), KIO::HideProgressInfo );

    d->mCreatePostMap[ job ] = post;

    if ( !job ) {
      kWarning() << "Failed to create job for: " << url().url();
    }

    job->addMetaData(
      QLatin1String("customHTTPHeader"), QLatin1String("X-hacker: Shame on you Wordpress, ") + QString() +
      QLatin1String("you took another 4 hours of my life to work around the stupid dateTime bug.") );
    job->addMetaData( QLatin1String("content-type"), QLatin1String("Content-Type: text/xml; charset=utf-8") );
    job->addMetaData( QLatin1String("ConnectTimeout"), QLatin1String("50") );
    job->addMetaData( QLatin1String("UserAgent"), userAgent() );

    connect( job, SIGNAL(result(KJob*)),
             this, SLOT(slotCreatePost(KJob*)) );
    // HACK: uuh this a bit ugly now... reenable the original publish argument,
    // since createPost should have parsed now
    post->setPrivate( publish );
  }
}

void WordpressBuggy::modifyPost( KBlog::BlogPost *post )
{
  // reimplemented because we do this:
  // http://comox.textdrive.com/pipermail/wp-testers/2005-July/000284.html
  kDebug();
  Q_D( WordpressBuggy );

  // we need mCategoriesList to be loaded first, since we cannot use the post->categories()
  // names later, but we need to map them to categoryId of the blog
  d->loadCategories();
  if ( d->mCategoriesList.isEmpty() ) {
    kDebug() << "No categories in the cache yet. Have to fetch them first.";
    d->mModifyPostCache << post;
    connect( this,SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
             this,SLOT(slotTriggerModifyPost()) );
    listCategories();
  }
  else {
    if ( !post ) {
      kError() << "WordpressBuggy::modifyPost: post is a null pointer";
      emit error ( Other, i18n( "Post is a null pointer." ) );
      return;
    }

    kDebug() << "Uploading Post with postId" << post->postId();

    QString xmlMarkup = QLatin1String("<?xml version=\"1.0\"?>");
    xmlMarkup += QLatin1String("<methodCall>");
    xmlMarkup += QLatin1String("<methodName>metaWeblog.editPost</methodName>");
    xmlMarkup += QLatin1String("<params><param>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[")+post->postId()+QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</param>");
    xmlMarkup += QLatin1String("<param>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[")+username()+QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</param><param>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[")+password()+QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</param>");
    xmlMarkup += QLatin1String("<param><struct>");
    xmlMarkup += QLatin1String("<member><name>description</name>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[")+post->content()+QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>title</name>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[")+post->title()+QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</member><member>");

    xmlMarkup += QLatin1String("<name>lastModified</name>");
    xmlMarkup += QLatin1String("<value><dateTime.iso8601>") +
                 post->modificationDateTime().dateTime().toUTC().toString( QLatin1String("yyyyMMddThh:mm:ss") ) +
                 QLatin1String("</dateTime.iso8601></value>");
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>dateCreated</name>");
    xmlMarkup += QLatin1String("<value><dateTime.iso8601>") +
                 post->creationDateTime().dateTime().toUTC().toString( QLatin1String("yyyyMMddThh:mm:ss") ) +
                 QLatin1String("</dateTime.iso8601></value>");
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>mt_allow_comments</name>");
    xmlMarkup += QString::fromLatin1( "<value><int>%1</int></value>" ).arg( (int)post->isCommentAllowed() );
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>mt_allow_pings</name>");
    xmlMarkup += QString::fromLatin1( "<value><int>%1</int></value>" ).arg( (int)post->isTrackBackAllowed() );
    xmlMarkup += QLatin1String("</member><member>");
    if ( !post->additionalContent().isEmpty() ) {
        xmlMarkup += QLatin1String("<name>mt_text_more</name>");
        xmlMarkup += QLatin1String("<value><string><![CDATA[") + post->additionalContent() + QLatin1String("]]></string></value>");
        xmlMarkup += QLatin1String("</member><member>");
    }
    xmlMarkup += QLatin1String("<name>wp_slug</name>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[") + post->slug() + QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>mt_excerpt</name>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[") + post->summary() + QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</member><member>");
    xmlMarkup += QLatin1String("<name>mt_keywords</name>");
    xmlMarkup += QLatin1String("<value><string><![CDATA[") + post->tags().join( QLatin1String(",") ) + QLatin1String("]]></string></value>");
    xmlMarkup += QLatin1String("</member></struct></param>");
    xmlMarkup += QLatin1String("<param><value><boolean>") +
                 QString::fromLatin1( "%1" ).arg( (int)( !post->isPrivate() ) ) +
                 QLatin1String("</boolean></value></param>");
    xmlMarkup += QLatin1String("</params></methodCall>");

    QByteArray postData;
    QDataStream stream( &postData, QIODevice::WriteOnly );
    stream.writeRawData( xmlMarkup.toUtf8(), xmlMarkup.toUtf8().length() );

    KIO::StoredTransferJob *job = KIO::storedHttpPost( postData, url(), KIO::HideProgressInfo );

    d->mModifyPostMap[ job ] = post;

    if ( !job ) {
      kWarning() << "Failed to create job for: " << url().url();
    }

    job->addMetaData(
      QLatin1String("customHTTPHeader"), QLatin1String("X-hacker: Shame on you Wordpress, ") + QString() +
      QLatin1String("you took another 4 hours of my life to work around the stupid dateTime bug.") );
    job->addMetaData( QLatin1String("content-type"), QLatin1String("Content-Type: text/xml; charset=utf-8") );
    job->addMetaData( QLatin1String("ConnectTimeout"), QLatin1String("50") );
    job->addMetaData( QLatin1String("UserAgent"), userAgent() );

    connect( job, SIGNAL(result(KJob*)),
             this, SLOT(slotModifyPost(KJob*)) );
  }
}

QString WordpressBuggy::interfaceName() const
{
  return QLatin1String( "Movable Type" );
}

WordpressBuggyPrivate::WordpressBuggyPrivate()
{
}

WordpressBuggyPrivate::~WordpressBuggyPrivate()
{
  kDebug();
}

QList<QVariant> WordpressBuggyPrivate::defaultArgs( const QString &id )
{
  Q_Q( WordpressBuggy );
  QList<QVariant> args;
  if ( !id.isEmpty() ) {
    args << QVariant( id );
  }
  args << QVariant( q->username() )
       << QVariant( q->password() );
  return args;
}

void WordpressBuggyPrivate::slotCreatePost( KJob *job )
{
  kDebug();

  KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob*>( job );
  const QString data = QString::fromUtf8( stj->data(), stj->data().size() );

  Q_Q( WordpressBuggy );

  KBlog::BlogPost *post = mCreatePostMap[ job ];
  mCreatePostMap.remove( job );

  if ( job->error() != 0 ) {
    kError() << "slotCreatePost error:" << job->errorString();
    emit q->errorPost( WordpressBuggy::XmlRpc, job->errorString(), post );
    return;
  }

  QRegExp rxError( QLatin1String("faultString") );
  if ( rxError.indexIn( data ) != -1 ) {
    rxError = QRegExp( QLatin1String("<string>(.+)</string>") );
    if ( rxError.indexIn( data ) != -1 ) {
      kDebug() << "RegExp of faultString failed.";
    }
    kDebug() << rxError.cap( 1 );
    emit q->errorPost( WordpressBuggy::XmlRpc, rxError.cap( 1 ), post );
    return;
  }

  QRegExp rxId( QLatin1String("<string>(.+)</string>") );
  if ( rxId.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the id out of the result:" << data;
    emit q->errorPost( WordpressBuggy::XmlRpc,
                       i18n( "Could not regexp the id out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( \"<string>(.+)</string>\" ) matches" << rxId.cap( 1 );

  post->setPostId( rxId.cap( 1 ) );
  if ( mSilentCreationList.contains(  post ) )
  {
    // set the categories and publish afterwards
    setPostCategories( post, !post->isPrivate() );
  } else {
    kDebug() << "emitting createdPost()"
                << "for title: \"" << post->title();
    emit q->createdPost( post );
    post->setStatus( KBlog::BlogPost::Created );
  }
}

void WordpressBuggyPrivate::slotModifyPost( KJob *job )
{
  kDebug();

  KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob*>( job );
  const QString data = QString::fromUtf8( stj->data(), stj->data().size() );

  KBlog::BlogPost *post = mModifyPostMap[ job ];
  mModifyPostMap.remove( job );
  Q_Q( WordpressBuggy );
  if ( job->error() != 0 ) {
    kError() << "slotModifyPost error:" << job->errorString();
    emit q->errorPost( WordpressBuggy::XmlRpc, job->errorString(), post );
    return;
  }

  QRegExp rxError( QLatin1String("faultString") );
  if ( rxError.indexIn( data ) != -1 ) {
    rxError = QRegExp( QLatin1String("<string>(.+)</string>") );
    if ( rxError.indexIn( data ) != -1 ) {
      kDebug() << "RegExp of faultString failed.";
    }
    kDebug() << rxError.cap( 1 );
    emit q->errorPost( WordpressBuggy::XmlRpc, rxError.cap( 1 ), post );
    return;
  }

  QRegExp rxId( QLatin1String("<boolean>(.+)</boolean>") );
  if ( rxId.indexIn( data ) == -1 ) {
    kError() << "Could not regexp the id out of the result:" << data;
    emit q->errorPost( WordpressBuggy::XmlRpc,
                       i18n( "Could not regexp the id out of the result." ), post );
    return;
  }
  kDebug() << "QRegExp rx( \"<boolean>(.+)</boolean>\" ) matches" << rxId.cap( 1 );

  if ( rxId.cap( 1 ).toInt() == 1 ) {
    kDebug() << "Post successfully updated.";
    if ( mSilentCreationList.contains( post ) ) {
      post->setStatus( KBlog::BlogPost::Created );
      emit q->createdPost( post );
      mSilentCreationList.removeOne( post );
    } else {
      if ( !post->categories().isEmpty() ) {
        setPostCategories( post, false );
      }
    }
  }
}

#include "moc_wordpressbuggy.cpp"
