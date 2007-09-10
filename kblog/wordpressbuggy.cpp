/*
  This file is part of the kblog library.

  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>

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

#include "blogposting.h"

#include <KDebug>
#include <KLocale>
#include <KDateTime>

#include <kio/http.h>
#include <kio/job.h>

#include <QtCore/QStringList>

using namespace KBlog;

WordpressBuggy::WordpressBuggy( const KUrl &server, QObject *parent )
  : MovableType( server, *new WordpressBuggyPrivate, parent )
{
  kDebug(5323) << "WordpressBuggy()";
}

WordpressBuggy::WordpressBuggy( const KUrl &server, WordpressBuggyPrivate &dd,
                        QObject *parent )
  : MovableType( server, dd, parent )
{
  kDebug(5323) << "WordpressBuggy()";
}

WordpressBuggy::~WordpressBuggy()
{
  kDebug(5323) << "~WordpressBuggy()";
}

void WordpressBuggy::createPosting( KBlog::BlogPosting *posting )
{
  Q_D(WordpressBuggy);
  if ( !posting ) {
    kError(5323) << "WordpressBuggy::createPosting: posting is a null pointer";
    emit error ( Other, i18n( "Posting is a null pointer." ) );
    return;
  }
  kDebug(5323) << "Creating new Posting with blogId" << blogId();

  QString xmlMarkup = "<?xml version=\"1.0\"?>";
  xmlMarkup += "<methodCall>";
  xmlMarkup += "<methodName>metaWeblog.newPost</methodName>";
  xmlMarkup += "<params><param>";
  xmlMarkup += "<value><![CDATA["+blogId()+"]]></value>";
  xmlMarkup += "</param>";
  xmlMarkup += "<param>";
  xmlMarkup += "<value><![CDATA["+username()+"]]></value>";
  xmlMarkup += "</param><param>";
  xmlMarkup += "<value><![CDATA["+password()+"]]></value>";
  xmlMarkup += "</param>";
  xmlMarkup += "<param><struct>";
  xmlMarkup += "<member><name>description</name>";
  xmlMarkup += "<value><![CDATA["+posting->content().toUtf8()+"]]></value>";
  xmlMarkup += "</member><member>";
  xmlMarkup += "<name>title</name>";
  xmlMarkup += "<value><![CDATA["+posting->title().toUtf8()+"]]></value>";
  xmlMarkup += "</member><member>";

  QList<QString> catList = posting->categories();
  if( !catList.empty() ){
    xmlMarkup += "<name>categories</name>";
    xmlMarkup += "<value><array>";
    QList<QString>::ConstIterator it = catList.begin();
    QList<QString>::ConstIterator end = catList.end();
    for( ; it!=end; it++ ){
      xmlMarkup += "<data><string><![CDATA[" + ( *it ).toUtf8() + "]]></string></data>";
    }
    xmlMarkup += "</array></value>";
    xmlMarkup += "</member><member>";
  }

  xmlMarkup += "<name>dateCreated</name>";
  xmlMarkup += "<value><dateTime.iso8601>"+
    posting->creationDateTime().toUtc().dateTime().toString("yyyy-MM-ddThhmmss")+
  "</dateTime.iso8601></value>";
  xmlMarkup += "</member><member>";
  xmlMarkup += "<name>mt_allow_comments</name>";
  xmlMarkup += QString("<value><i4>%1</i4></value>").
    arg( (int)posting->isCommentAllowed() );
  xmlMarkup += "</member><member>";
  xmlMarkup += "<name>mt_allow_pings</name>";
  xmlMarkup += QString("<value><i4>%1</i4></value>").
    arg( (int)posting->isTrackBackAllowed() );
  xmlMarkup += "</member><member>";
  xmlMarkup += "<name>mt_excerpt</name>";
  xmlMarkup += "<value><![CDATA["+posting->summary().toUtf8()+"]]></value>";
  xmlMarkup += "</member><member>";
  xmlMarkup += "<name>mt_keywords</name>";
  xmlMarkup += "<value><![CDATA["+posting->tags().join(" ").toUtf8()+"]]></value>";
  xmlMarkup += "</struct></member>";
  xmlMarkup += "<param><value><boolean>"+
    QString( "%1" ).arg( (int)(!posting->isPrivate()) )+
    "</boolean></value></param>";
  xmlMarkup += "</params></methodCall>";

  QByteArray postData;
  QDataStream stream( &postData, QIODevice::WriteOnly );
  stream.writeRawData( xmlMarkup.toUtf8(), xmlMarkup.toUtf8().length() );

  KIO::TransferJob *job = KIO::http_post( url(), postData, false );

  d->mCreatePostingMap[ job ] = posting;

  if ( !job ) {
    kWarning() << "Failed to create job for: " << url().url();
  }

  job->addMetaData( "content-type", "Content-Type: text/xml; charset=utf-8" );
  job->addMetaData( "ConnectTimeout", "50" );
  job->addMetaData( "UserAgent", userAgent() );

  connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           this, SLOT( slotCreatePostingData( KIO::Job *, const QByteArray & ) ) );
  connect( job, SIGNAL( result( KJob * ) ),
           this, SLOT( slotCreatePosting( KJob * ) ) );
}

void WordpressBuggy::modifyPosting( KBlog::BlogPosting *posting )
{
  Q_D(WordpressBuggy);
  if ( !posting ) {
    kError(5323) << "WordpressBuggy::modifyPosting: posting is a null pointer";
    emit error ( Other, i18n( "Posting is a null pointer." ) );
    return;
  }

  kDebug(5323) << "Uploading Posting with postId" << posting->postingId();
  QString xmlMarkup = "<?xml version=\"1.0\"?>";
  xmlMarkup += "<methodCall>";
  xmlMarkup += "<methodName>metaWeblog.editPost</methodName>";
  xmlMarkup += "<params><param>";
  xmlMarkup += "<value><![CDATA["+posting->postingId()+"]]></value>";
  xmlMarkup += "</param>";
  xmlMarkup += "<param>";
  xmlMarkup += "<value><![CDATA["+username()+"]]></value>";
  xmlMarkup += "</param><param>";
  xmlMarkup += "<value><![CDATA["+password()+"]]></value>";
  xmlMarkup += "</param>";
  xmlMarkup += "<param><struct>";
  xmlMarkup += "<member><name>description</name>";
  xmlMarkup += "<value><![CDATA["+posting->content().toUtf8()+"]]></value>";
  xmlMarkup += "</member><member>";
  xmlMarkup += "<name>title</name>";
  xmlMarkup += "<value><![CDATA["+posting->title().toUtf8()+"]]></value>";
  xmlMarkup += "</member><member>";

  QList<QString> catList = posting->categories();
  if( !catList.empty() ){
    xmlMarkup += "<name>categories</name>";
    xmlMarkup += "<value><array>";
    QList<QString>::ConstIterator it = catList.begin();
    QList<QString>::ConstIterator end = catList.end();
    for( ; it!=end; it++ ){
      xmlMarkup += "<data><string><![CDATA[" + ( *it ).toUtf8() + "]]></string></data>";
    }
    xmlMarkup += "</array></value>";
    xmlMarkup += "</member><member>";
  }

  xmlMarkup += "<name>lastModified</name>";
  xmlMarkup += "<value><dateTime.iso8601>"+
    posting->modificationDateTime().toUtc().dateTime().toString("yyyy-MM-ddThhmmss")+
  "</dateTime.iso8601></value>";
  xmlMarkup += "</member><member>";
  xmlMarkup += "<name>mt_allow_comments</name>";
  xmlMarkup += QString("<value><i4>%1</i4></value>").
    arg( (int)posting->isCommentAllowed() );
  xmlMarkup += "</member><member>";
  xmlMarkup += "<name>mt_allow_pings</name>";
  xmlMarkup += QString("<value><i4>%1</i4></value>").
    arg( (int)posting->isTrackBackAllowed() );
  xmlMarkup += "</member><member>";
  xmlMarkup += "<name>mt_excerpt</name>";
  xmlMarkup += "<value><![CDATA["+posting->summary().toUtf8()+"]]></value>";
  xmlMarkup += "</member><member>";
  xmlMarkup += "<name>mt_keywords</name>";
  xmlMarkup += "<value><![CDATA["+posting->tags().join(" ").toUtf8()+"]]></value>";
  xmlMarkup += "</struct></member>";
  xmlMarkup += "<param><value><boolean>"+
    QString( "%1" ).arg( (int)(!posting->isPrivate()) )+
    "</boolean></value></param>";
  xmlMarkup += "</params></methodCall>";

  QByteArray postData;
  QDataStream stream( &postData, QIODevice::WriteOnly );
  stream.writeRawData( xmlMarkup.toUtf8(), xmlMarkup.toUtf8().length() );

  KIO::TransferJob *job = KIO::http_post( url(), postData, false );

  d->mModifyPostingMap[ job ] = posting;

  if ( !job ) {
    kWarning() << "Failed to create job for: " << url().url();
  }

  job->addMetaData( "content-type", "Content-Type: text/xml; charset=utf-8" );
  job->addMetaData( "ConnectTimeout", "50" );
  job->addMetaData( "UserAgent", userAgent() );

  connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           this, SLOT( slotCreatePostingData( KIO::Job *, const QByteArray & ) ) );
  connect( job, SIGNAL( result( KJob * ) ),
           this, SLOT( slotCreatePosting( KJob * ) ) );
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
  kDebug(5323) << "~WordpressBuggyPrivate()";
}

QList<QVariant> WordpressBuggyPrivate::defaultArgs( const QString &id )
{
  Q_Q(WordpressBuggy);
  QList<QVariant> args;
  if( !id.isEmpty() )
    args << QVariant( id );
  args << QVariant( q->username() )
          << QVariant( q->password() );
  return args;
}

void WordpressBuggyPrivate::slotCreatePostingData( KIO::Job *job, const QByteArray &data )
{
  kDebug(5323) << "slotCreatePostingData()";
  unsigned int oldSize = mCreatePostingBuffer[ job ].size();
  mCreatePostingBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mCreatePostingBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void WordpressBuggyPrivate::slotCreatePosting( KJob *job )
{
  kDebug(5323) << "slotCreatePosting()";
  const QString data = QString::fromUtf8( mCreatePostingBuffer[ job ].data(), mCreatePostingBuffer[ job ].size() );
  mCreatePostingBuffer[ job ].resize( 0 );

  Q_Q(WordpressBuggy);

  KBlog::BlogPosting* posting = mCreatePostingMap[ job ];
  mCreatePostingMap.remove( job );

  if ( job->error() != 0 ) {
    kError(5323) << "slotCreatePosting error:" << job->errorString();
    emit q->errorPosting( WordpressBuggy::Atom, job->errorString(), posting );
    return;
  }

  QRegExp rxError( "faultString" );
  if( rxError.indexIn( data ) != -1 ){
    rxError = QRegExp( "<string>(.+)</string>" );
    if( rxError.indexIn( data ) != -1 )
      kDebug(5323) << "RegExp of faultString failed.";
    kDebug(5323) << rxError.cap(1);
    emit q->errorPosting( WordpressBuggy::XmlRpc, rxError.cap(1), posting );
    return;
  }

  QRegExp rxId( "<string>(.+)</string>" );
  if( rxId.indexIn( data )==-1 ){
    kError(5323) << "Could not regexp the id out of the result:" << data;
    emit q->errorPosting( WordpressBuggy::XmlRpc,
                          i18n( "Could not regexp the id out of the result." ),
                          posting );
    return;
  }
  kDebug(5323) << "QRegExp rx(  \"<string>(.+)</string>\" ) matches" << rxId.cap(1);

  posting->setPostingId( rxId.cap(1) );
  posting->setStatus( BlogPosting::Created );
  emit q->createdPosting( posting );
  kDebug(5323) << "Emitting createdPosting()";
}

void WordpressBuggyPrivate::slotModifyPostingData( KIO::Job *job, const QByteArray &data )
{
  kDebug(5323) << "slotModifyPostingData()";
  unsigned int oldSize = mModifyPostingBuffer[ job ].size();
  mModifyPostingBuffer[ job ].resize( oldSize + data.size() );
  memcpy( mModifyPostingBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void WordpressBuggyPrivate::slotModifyPosting( KJob *job )
{
  kDebug(5323) << "slotModifyPosting()";
  const QString data = QString::fromUtf8( mModifyPostingBuffer[ job ].data(), mModifyPostingBuffer[ job ].size() );
  mModifyPostingBuffer[ job ].resize( 0 );

  KBlog::BlogPosting* posting = mModifyPostingMap[ job ];
  mModifyPostingMap.remove( job );
  Q_Q(WordpressBuggy);
  if ( job->error() != 0 ) {
    kError(5323) << "slotModifyPosting error:" << job->errorString();
    emit q->errorPosting( WordpressBuggy::Atom, job->errorString(), posting );
    return;
  }


  QRegExp rxError( "faultString" );
  if( rxError.indexIn( data ) != -1 ){
    rxError = QRegExp( "<string>(.+)</string>" );
    if( rxError.indexIn( data ) != -1 )
      kDebug(5323) << "RegExp of faultString failed.";
    kDebug(5323) << rxError.cap(1);
    emit q->errorPosting( WordpressBuggy::XmlRpc, rxError.cap(1), posting );
    return;
  }

  QRegExp rxId( "<string>(.+)</string>" );
  if( rxId.indexIn( data )==-1 ){
    kError(5323) << "Could not regexp the id out of the result:" << data;
    emit q->errorPosting( WordpressBuggy::XmlRpc,
                          i18n( "Could not regexp the id out of the result." ),
                          posting );
    return;
  }
  kDebug(5323) << "QRegExp rx(  \"<string>(.+)</string>\" ) matches" << rxId.cap(1);

  posting->setPostingId( rxId.cap(1) );
  posting->setStatus( BlogPosting::Modified );
  emit q->modifiedPosting( posting );
}


#include "wordpressbuggy.moc"
