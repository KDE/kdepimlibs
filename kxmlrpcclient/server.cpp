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

namespace XmlRpc {

Server::Server( const KUrl &url, QObject *parent, const char *name ) : QObject( parent, name ) {

	if ( url.isValid() )
		m_url = url;

	m_userAgent = "KDE XMLRPC resources";
}

Server::~Server() {

	QList<Query*>::Iterator it;
	for ( it = mPendingQueries.begin(); it !=mPendingQueries.end(); ++it )
		(*it)->deleteLater();

	mPendingQueries.clear();
}

void Server::queryFinished( Query *query ) {

	mPendingQueries.remove( query );
	query->deleteLater();
}

void Server::setUrl( const KUrl &url ) {

	m_url = url.isValid() ? url : KUrl();
}

void Server::call( const QString &method, const QList<QVariant> &args,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot, const QVariant &id ) {

	if ( m_url.isEmpty() )
		kWarning() << "Cannot execute call to " << method << ": empty server URL" << endl;

	Query *query = Query::create( id, this );
	connect( query, SIGNAL( message( const QList<QVariant> &, const QVariant& ) ), msgObj, messageSlot );
	connect( query, SIGNAL( fault( int, const QString&, const QVariant& ) ), faultObj, faultSlot );
	connect( query, SIGNAL( finished( Query* ) ), this, SLOT( queryFinished( Query* ) ) );
	mPendingQueries.append( query );

	query->call( m_url.url(), method, args, m_userAgent );
}

void Server::call( const QString &method, const QVariant &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id ) {

	QList<QVariant> args;
	args << arg ;
	call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, int arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id ) {

	QList<QVariant> args;
	args << QVariant( arg );
	call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, bool arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id ) {

	QList<QVariant> args;
	args << QVariant( arg );
	call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, double arg ,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id ) {

	QList<QVariant> args;
	args << QVariant( arg );
	call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, const QString &arg ,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id ) {

	QList<QVariant> args;
	args << QVariant( arg );
	call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, const QByteArray &arg ,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id ) {

	QList<QVariant> args;
	args << QVariant( arg );
	call( method, args, faultObj, faultSlot, msgObj, messageSlot, id );
}

void Server::call( const QString &method, const QDateTime &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id ) {

	QList<QVariant> args;
	args << QVariant( arg );
	call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, const QStringList &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id ) {

	QList<QVariant> args;
	QStringList::ConstIterator it = arg.begin();
	QStringList::ConstIterator end = arg.end();
	for ( ; it != end; ++it )
		args << QVariant( *it );
	call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

} // namespace XmlRpc

#include "server.moc"
