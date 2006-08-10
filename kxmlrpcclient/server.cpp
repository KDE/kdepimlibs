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

#include "server.h"
#include "query.h"

/**
  @file

  The implementation of Server
**/

using namespace KXmlRpc;

class Server::Private
{
  public:
    KUrl mUrl;
    QString mUserAgent;

    QList<Query*> mPendingQueries;

    void queryFinished( Query* );
};

void Server::Private::queryFinished( Query *query )
{
  mPendingQueries.removeAll( query );
  query->deleteLater();
}

Server::Server( const KUrl &url, QObject *parent )
  : QObject( parent ), d( new Private )
{
  if ( url.isValid() )
    d->mUrl = url;

  d->mUserAgent = "KDE XMLRPC resources";
}

Server::~Server()
{
  QList<Query*>::Iterator it;
  for ( it = d->mPendingQueries.begin(); it != d->mPendingQueries.end(); ++it )
    (*it)->deleteLater();

  d->mPendingQueries.clear();

  delete d;
}

void Server::setUrl( const KUrl &url )
{
  d->mUrl = url.isValid() ? url : KUrl();
}

KUrl Server::url() const
{
  return d->mUrl;
}

QString Server::userAgent() const
{
  return d->mUserAgent;
}

void Server::setUserAgent( const QString &userAgent )
{
  d->mUserAgent = userAgent;
}

void Server::call( const QString &method, const QList<QVariant> &args,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot, const QVariant &id )
{
  if ( d->mUrl.isEmpty() )
    kWarning() << "Cannot execute call to " << method << ": empty server URL" << endl;

  Query *query = Query::create( id, this );
  connect( query, SIGNAL( message( const QList<QVariant> &, const QVariant& ) ), msgObj, messageSlot );
  connect( query, SIGNAL( fault( int, const QString&, const QVariant& ) ), faultObj, faultSlot );
  connect( query, SIGNAL( finished( Query* ) ), this, SLOT( queryFinished( Query* ) ) );
  d->mPendingQueries.append( query );

  query->call( d->mUrl.url(), method, args, d->mUserAgent );
}

void Server::call( const QString &method, const QVariant &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << arg ;
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, int arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, bool arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, double arg ,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, const QString &arg ,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, const QByteArray &arg ,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, faultObj, faultSlot, msgObj, messageSlot, id );
}

void Server::call( const QString &method, const QDateTime &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, const QStringList &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;
  for ( int i = 0; i < arg.count(); ++i )
    args << QVariant( arg[ i ] );

  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

#include "server.moc"
