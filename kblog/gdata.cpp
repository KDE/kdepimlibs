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

#include "gdata.h"
#include "gdata_p.h"
#include "blogposting.h"

#include <syndication/loader.h>
#include <kio/netaccess.h>
#include <kio/http.h>
#include <kio/job.h>

#include <KDebug>
#include <KLocale>

using namespace KBlog;

APIGData::APIGData( const KUrl &server, QObject *parent )
  : APIBlog( server, parent ), d( new APIGDataPrivate )
{
  d->parent = this;
  setUrl( server );
}

APIGData::~APIGData()
{
  delete d;
}

QString APIGData::interfaceName() const
{
  return QLatin1String( "GData API" );
}

QString APIGData::fullName() const
{
  return d->mFullName;
}

void APIGData::setFullName( const QString &fullName )
{
  d->mFullName = fullName;
}

QString APIGData::profileId() const
{
  return d->mProfileId;
}

void APIGData::setProfileId( const QString& pid )
{
  d->mProfileId = pid;
}

void APIGData::userInfo()
{
  // fetch the introspection file synchronously and parse it
  QByteArray data;
  KIO::Job *job = KIO::get( url(), false, false );
  KUrl blogUrl = url();
  /* make asynchronous */
  if ( KIO::NetAccess::synchronousRun( job, (QWidget*)0, &data, &blogUrl ) ) {
    kDebug() << "Fetched Homepage data." << endl;
//     QRegExp pp( "<link.+rel=\"service.post\".+href=\"(.+)\".*/>" );
//     if( pp.indexIn( data )!=-1 )
//        mCreatePostingsPath = pp.cap(1);
//     else
//        emit parent->error( Other,
//    i18n( "Could not regexp the service.post path." ) );
//     kDebug(5323)<<
//   "QRegExp pp( \"<link.+rel=\"service.post\".+href=\"(.+)\".*/>\" ) matches "
//    << pp.cap(1) << endl;

    QRegExp bid( "<link.+blogID=(\\d+).*/>" );
    if( bid.indexIn( data )!=-1 )
       setBlogId( bid.cap(1) );
    else
      emit error( Other, i18n( "Could not regexp the blogID path." ) );
    kDebug(5323)<<"QRegExp bid( '<link.+blogID=(\\d+).*/>' matches "
        << bid.cap(1) << endl;
  }
  else {
    emit error( Other, i18n( "Could not fetch the homepage data." ) );
    kDebug(5323)<< "Could not fetch the homepage data." << endl;
  }
}

void APIGData::listBlogs()
{
    kDebug() << "listBlogs()" << endl;
    Syndication::Loader *loader = Syndication::Loader::create();
    connect( loader, SIGNAL(loadingComplete(Syndication::Loader*,
                            Syndication::FeedPtr, Syndication::ErrorCode)),
            d, SLOT(slotLoadingBlogsComplete(Syndication::Loader*,
                    Syndication::FeedPtr, Syndication::ErrorCode)) );
    loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + profileId()
        + QString( "/blogs" ) );
}

void APIGData::listRecentPostings( int number )
{
    kDebug() << "listRecentPostings()" << endl;
    Syndication::Loader *loader = Syndication::Loader::create();
    connect( loader, SIGNAL(loadingComplete(Syndication::Loader*,
                            Syndication::FeedPtr, Syndication::ErrorCode)),
            d, SLOT(slotLoadingPostingsComplete(Syndication::Loader*,
                    Syndication::FeedPtr, Syndication::ErrorCode)) );
    loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + blogId()
        + QString( "/posts/default" ) );
}

void APIGData::fetchPosting( KBlog::BlogPosting *posting )
{
  Q_UNUSED( posting );
//   if ( d->mLock.tryLock() ) {
//     kDebug() << "fetchPosting()" << endl;
//     Syndication::Loader *loader = Syndication::Loader::create();
//     d->setFetchPostingId( postingId );
//     connect( loader, SIGNAL(loadingComplete(Syndication::Loader*,
//                      Syndication::FeedPtr, Syndication::ErrorCode)),
//             d, SLOT(slotFetchingPostingComplete(Syndication::Loader*,
//                     Syndication::FeedPtr, Syndication::ErrorCode)));
//     loader->loadFrom( QString( "http://www.blogger.com/feeds/" ) + blogId()
//         + QString( "/posts/default" ) );
//     return true;
//   }
//   return false;
}

void APIGData::modifyPosting( KBlog::BlogPosting* posting )
{
    Q_UNUSED( posting );
    kDebug() << "modifyPosting()" << endl;
    d->authenticate();
}

void APIGData::createPosting( KBlog::BlogPosting* posting )
{
  Q_UNUSED( posting );
    kDebug() << "createPosting()" << endl;
    d->authenticate();

    QString atomMarkup = "<entry xmlns='http://www.w3.org/2005/Atom'>";
    atomMarkup += "<title type='text'>"+posting->title() +"</title>";
    if( !posting->isPublished() )
    {
      atomMarkup += "<app:control xmlns:app=*http://purl.org/atom/app#'>";
      atomMarkup += "<app:draft>yes</app:draft></app:control>";
    }
    atomMarkup += "<content type='xhtml'>";
    atomMarkup += "<div xmlns='http://www.w3.org/1999/xhtml'>";
    atomMarkup += posting->content();
    atomMarkup += "</div></content>";
    atomMarkup += "<author>";
    atomMarkup += "<name>" + fullName() + "</name>"; //FIXME user's name
    atomMarkup += "<email>" + username() + "</email>";
    atomMarkup += "</author>";
    atomMarkup += "</entry>";

    QByteArray postData;
    QDataStream stream( &postData, QIODevice::WriteOnly );
    stream.writeRawData( atomMarkup.toUtf8(), atomMarkup.toUtf8().length() );

    KIO::TransferJob *job = KIO::http_post(
        KUrl( "http://www.blogger.com/feeds/" + blogId() + "/posts/default" ),
        postData, false );

    if ( !job ) {
      kWarning() << "Unable to create KIO job for http://www.blogger.com/feeds/"
          << blogId() <<"/posts/default" << endl;
    }

    job->addMetaData( "content-type", "Content-Type: text/xml; charset=utf-8" );
    job->addMetaData( "ConnectTimeout", "50" );

    connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             d, SLOT( slotData( KIO::Job *, const QByteArray & ) ) );
    connect( job, SIGNAL( result( KJob * ) ),
             d, SLOT( slotCreatePosting( KJob * ) ) );
}

void APIGData::removePosting( KBlog::BlogPosting *posting )
{
    Q_UNUSED( posting );
    kDebug() << "deletePosting()" << endl;
    d->authenticate();
}

#include "gdata.moc"
