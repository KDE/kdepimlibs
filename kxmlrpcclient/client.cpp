/**************************************************************************
*   Copyright (C) 2006 by Narayan Newton <narayannewton@gmail.com>        *
*   Copyright (C) 2003 - 2004 by Frerich Raabe <raabe@kde.org>            *
*                                Tobias Koenig <tokoe@kde.org>            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>
#include <kurl.h>

#include <QVariant>

#include "client.h"
#include "query.h"

/**
  @file

  The implementation of KXmlRpc::Client
**/

using namespace KXmlRpc;

class Client::Private
{
  public:
    KUrl mUrl;
    QString mUserAgent;
    bool mDigestAuth;

    QList<Query*> mPendingQueries;

    void queryFinished( Query * );
};

void Client::Private::queryFinished( Query *query )
{
  mPendingQueries.removeAll( query );
  query->deleteLater();
}

Client::Client( QObject *parent )
  : QObject( parent ), d( new Private )
{
  d->mUserAgent = "KDE XMLRPC resources";
  d->mDigestAuth = false;
}

Client::Client( const KUrl &url, QObject *parent )
  : QObject( parent ), d( new Private )
{
  if ( url.isValid() ) {
    d->mUrl = url;
  }

  d->mUserAgent = "KDE XMLRPC resources";
  d->mDigestAuth = false;
}

Client::~Client()
{
  QList<Query *>::Iterator it;
  for ( it = d->mPendingQueries.begin(); it != d->mPendingQueries.end(); ++it ) {
    (*it)->deleteLater();
  }

  d->mPendingQueries.clear();

  delete d;
}

void Client::setUrl( const KUrl &url )
{
  d->mUrl = url.isValid() ? url : KUrl();
}

KUrl Client::url() const
{
  return d->mUrl;
}

QString Client::userAgent() const
{
  return d->mUserAgent;
}

void Client::setUserAgent( const QString &userAgent )
{
  d->mUserAgent = userAgent;
}

bool Client::digestAuth() const
{
  return d->mDigestAuth;
}

void Client::enableDigestAuth()
{
  d->mDigestAuth = true;
}

void Client::disableDigestAuth()
{
  d->mDigestAuth = false;
}

void Client::call( const QString &method, const QList<QVariant> &args,
                   QObject *msgObj, const char *messageSlot,
                   QObject *faultObj, const char *faultSlot, const QVariant &id )
{

  QMap<QString, QString> metaData;

  if ( d->mUrl.isEmpty() ) {
    kWarning() << "Cannot execute call to " << method << ": empty server URL" << endl;
  }

  //Fill metadata, with userAgent and possible digest auth
  if ( d->mUserAgent.isEmpty() ) {
    metaData["UserAgent"] = "KDE-XMLRPC";
  } else {
    metaData["UserAgent"] = d->mUserAgent;
  }

  if ( d->mDigestAuth ) {
    metaData["WWW-Authenticate:"] = "Digest";
  }

  Query *query = Query::create( id, this );
  connect( query, SIGNAL( message( const QList<QVariant> &, const QVariant & ) ), msgObj, messageSlot );
  connect( query, SIGNAL( fault( int, const QString &, const QVariant & ) ), faultObj, faultSlot );
  connect( query, SIGNAL( finished( Query * ) ), this, SLOT( queryFinished( Query * ) ) );
  d->mPendingQueries.append( query );

  query->call( d->mUrl.url(), method, args, metaData );
}

void Client::call( const QString &method, const QVariant &arg,
                   QObject *msgObj, const char *messageSlot,
                   QObject *faultObj, const char *faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << arg ;
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Client::call( const QString &method, int arg,
                   QObject *msgObj, const char *messageSlot,
                   QObject *faultObj, const char *faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Client::call( const QString &method, bool arg,
                   QObject *msgObj, const char *messageSlot,
                   QObject *faultObj, const char *faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Client::call( const QString &method, double arg ,
                   QObject *msgObj, const char *messageSlot,
                   QObject *faultObj, const char *faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Client::call( const QString &method, const QString &arg ,
                   QObject *msgObj, const char *messageSlot,
                   QObject *faultObj, const char *faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Client::call( const QString &method, const QByteArray &arg ,
                   QObject *msgObj, const char *messageSlot,
                   QObject *faultObj, const char *faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, faultObj, faultSlot, msgObj, messageSlot, id );
}

void Client::call( const QString &method, const QDateTime &arg,
                   QObject *msgObj, const char *messageSlot,
                   QObject *faultObj, const char *faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Client::call( const QString &method, const QStringList &arg,
                   QObject *msgObj, const char *messageSlot,
                   QObject *faultObj, const char *faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  for ( int i = 0; i < arg.count(); ++i ) {
    args << QVariant( arg[ i ] );
  }

  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

#include "client.moc"
