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

#ifndef _KXML_RPC_SERVER_H_
#define _KXML_RPC_SERVER_H_

#include <kurl.h>

#include <QObject>
#include <QList>
#include <QVariant>

namespace XmlRpc
{
	
/**   
   @file
  
  	This file defines Server, our main class.
	It is the primary method of interaction with the library and is the object which represents the xml-rpc server.

	@author Narayan Newton <narayannewton@gmail.com>
	@author Frerich Raabe <raabe@kde.org>
	@author Tobias Koenig <tokoe@kde.org>
  
**/

//pre-decls
class Query;
class Server;


/**
		Server is a class that represents an xml-rpc server
		This is the main (only) class you need to worry about for building an 
		xml-rpc client. This class has one main method, "call", which is overloaded 
		extensively to handle different arguments.

		@code
	Server *serv = new Server(KUrl("http://localhost"), this);
	serv->setUserAgent("Test/1.0");
	serv->call("xmlrpc.command1", "Hi!", 
			   this, SLOT(gotData(const QList<QVariant>&, const QVariant)),
			   this, SLOT(gotError(const QString&, const QVariant&)));
		@endcode

		@author Narayan Newton <narayannewton@gmail.com>

**/

class Server : public QObject {

	Q_OBJECT
	
	public:

		/**
				The standard init function with few (possibly no) arguments

				@param parent the parent of this object, defaults to NULL.
				@param name the name of the object, defaults to NULL.


		**/

		Server( QObject *parent = 0, const char *name = 0 );

		/**
				The not so standard init function that takes a server url 
				as an argument 
	
				@param url the url for the xml-rpc server we will be connecting to
				@param parent the parent of this object, defaults to NULL.
				@param name the name of the object, defaults to NULL.

		**/

		Server( const KUrl &url, QObject *parent = 0, const char *name = 0 );

		/**

			Standard destructor.

		**/

		~Server();

		/**
			Gets the current url of the xml-rpc server.

			@return returns a QString set to the url of the xml-rpc server

		**/

		const KUrl &url() const { return m_url; }

		/**
			Sets the url for the xml-rpc server 
			
			@param url the url for the xml-rpc server we will be connecting to


		**/

		void setUrl( const KUrl &url );

		/**
			Gets the current user agent

			@return returns a QString set to the user agent

		**/

		QString userAgent() const { return m_userAgent; }

		/**
			Sets the url for the xml-rpc server

			@param userAgent the user agent to use for connecting to the xml-rpc server


		**/

		void setUserAgent( const QString &userAgent ) { m_userAgent = userAgent; }


		/**
			The main function for this class. This make a xml-rpc call to the server set via
			the constructor or via setUrl. You pass in the method, the argument list, 
			a slot for data arrival and a slot for possible errors.

			This method is HIGHLY over-loaded and relies heavily on QLists and QVariants.

			The following are the types of arguments supported:

				QList<QVariant>, 
				QVariant, 
				QString, 
				QByteArray, 
				QDateTime, 
				QStringList, 
				int, 
				bool, 
				double

			@code
	serv->call("xmlrpc.command1", "Hi!", 
			   this, SLOT(gotData(const QList<QVariant>&, const QVariant)),
			   this, SLOT(gotError(const QString&, const QVariant&)));
			@endcode

			@param method the method on the server we are going to be calling
			@param arg the argument or arguments you will be passing to the method
			@param obj the QObject of the error slot
			@param faultSlot the error slot itself
			@param obj the QObject of the data slot
			@param messageSlot the data slot itself
			@param id the id for our Server object, defaults to empty

		**/

		template <typename T>
		void call( const QString &method, const QList<T> &arg,
				QObject* obj, const char* messageSlot, 
				QObject* obj, const char* faultSlot,
				const QVariant &id = QVariant() );


	public slots:
	
		void call( const QString &method, const QList<QVariant> &args,
				QObject* msgObj, const char* messageSlot,
				QObject* faultObj, const char* faultSlot,
				const QVariant &id = QVariant() );

		void call( const QString &method, const QVariant &arg,
				QObject* msgObj, const char* messageSlot,
				QObject* faultObj, const char* faultSlot,
				const QVariant &id = QVariant() );

		void call( const QString &method, int arg ,
				QObject* msgObj, const char* messageSlot,
				QObject* faultObj, const char* faultSlot,
				const QVariant &id = QVariant() );

		void call( const QString &method, bool arg,
				QObject* msgObj, const char* messageSlot,
				QObject* faultObj, const char* faultSlot,
				const QVariant &id = QVariant() );

		void call( const QString &method, double arg,
				QObject* msgObj, const char* messageSlot,
				QObject* faultObj, const char* faultSlot,
				const QVariant &id = QVariant() );

		void call( const QString &method, const QString &arg,
				QObject* msgObj, const char* messageSlot,
				QObject* faultObj, const char* faultSlot,
				const QVariant &id = QVariant() );

		void call( const QString &method, const QByteArray &arg,
				QObject* msgObj, const char* messageSlot,
				QObject* faultObj, const char* faultSlot,
				const QVariant &id = QVariant() );

		void call( const QString &method, const QDateTime &arg,
				QObject* msgObj, const char* messageSlot,
				QObject* faultObj, const char* faultSlot,
				const QVariant &id = QVariant() );

		void call( const QString &method, const QStringList &arg,
				QObject* msgObj, const char* messageSlot,
				QObject* faultObj, const char* faultSlot,
				const QVariant &id = QVariant() );

	private slots:

		void queryFinished( Query* );

	private:
		
		KUrl m_url;
		QString m_userAgent;

		QList<Query*> mPendingQueries;
};

template <typename T>
void Server::call( const QString &method, const QList<T> &arg,
						QObject* msgObj, const char* messageSlot, 							
						QObject* faultObj, const char* faultSlot,
						const QVariant &id ) {

	QList<QVariant> args;

	typename QList<T>::ConstIterator it = arg.begin();
	typename QList<T>::ConstIterator end = arg.end();
	for ( ; it != end; ++it )
		args << QVariant( *it );

	return call( method, args, faultObj, faultSlot, msgObj, messageSlot, id );
}

} // namespace XmlRpc

#endif

