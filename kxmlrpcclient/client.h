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

#ifndef KXML_RPC_CLIENT_H
#define KXML_RPC_CLIENT_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <kurl.h>
#include "kxmlrpcclient.h"

namespace KXmlRpc
{

/**
  @file

  This file defines KXmlRpc::Client, our main class.
  It is the primary method of interaction with the library and is the object which represents our connection to the XML-RPC server.

  @author Narayan Newton <narayannewton@gmail.com>
  @author Frerich Raabe <raabe@kde.org>
  @author Tobias Koenig <tokoe@kde.org>
 */

/**
  KXmlRpc::Client is a class that represents our connection to a XML-RPC server
  This is the main (only) class you need to worry about for building an 
  XML-RPC client. This class has one main method, "call", which is overloaded 
  extensively to handle different arguments.

  @code
    KXmlRpc::Client *serv = new Client( KUrl( "http://localhost" ), this );
    server->setUserAgent( "Test/1.0" );
    server->call( "xmlrpc.command1", "Hi!", 
       this, SLOT( gotData( const QList<QVariant>&, const QVariant ) ),
       this, SLOT( gotError( const QString&, const QVariant& ) ) );
  @endcode

  @author Narayan Newton <narayannewton@gmail.com>
 */
class KXMLRPCCLIENT_EXPORT Client : public QObject
{
  Q_OBJECT

  public:
    /**
      The standard init function with few (possibly no) arguments

      @param parent the parent of this object, defaults to NULL.
      @param name the name of the object, defaults to NULL.
     */
    Client( QObject *parent = 0 );

    /**
      The not so standard init function that takes a server url
      as an argument

      @param url the url for the xml-rpc server we will be connecting to
      @param parent the parent of this object, defaults to NULL.
     */
    Client( const KUrl &url, QObject *parent = 0 );

    /**
      Standard destructor.
     */
    virtual ~Client();

    /**
      Returns the current url of the xml-rpc server.

      @return returns a QString set to the url of the xml-rpc server
     */
    KUrl url() const;

    /**
      Sets the url for the xml-rpc server 

      @param url the url for the xml-rpc server we will be connecting to
     */
    void setUrl( const KUrl &url );

    /**
      Returns the current user agent

      @return returns a QString set to the user agent
     */
    QString userAgent() const;

    /**
      Sets the url for the xml-rpc server

      @param userAgent the user agent to use for connecting to the xml-rpc server
     */
    void setUserAgent( const QString &userAgent );

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
         serv->call( "xmlrpc.command1", "Hi!", 
           this, SLOT( gotData( const QList<QVariant>&, const QVariant ) ),
           this, SLOT( gotError( const QString&, const QVariant& ) ) );
      @endcode

      @param method the method on the server we are going to be calling
      @param arg the argument or arguments you will be passing to the method
      @param obj the QObject of the error slot
      @param faultSlot the error slot itself
      @param obj the QObject of the data slot
      @param messageSlot the data slot itself
      @param id the id for our Client object, defaults to empty
     */

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

  private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void queryFinished( Query* ) )
};

template <typename T>
void Client::call( const QString &method, const QList<T> &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QList<QVariant> args;

  for ( int i = 0; i < arg.count(); ++i )
    args << QVariant( arg[ i ] );

  return call( method, args, faultObj, faultSlot, msgObj, messageSlot, id );
}

}

#endif

