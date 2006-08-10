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

#ifndef KXML_RPC_QUERY_H
#define KXML_RPC_QUERY_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <kio/job.h>

class QString;
class QDomDocument;
class QDomElement;

namespace KXmlRpc {

/**
  @file

  This file defines Result and Query, our internal classes
 */

/**
  Result is an internal class that represents a response from the XML-RPC 
  server. This is an internal class and is only used by Query
 */
class Result
{
  friend class Query;

  public:
    Result();
    Result( const Result &other );
    Result& operator=( const Result &other );
    virtual ~Result();

    bool success() const;

    int errorCode() const;

    QString errorString() const;

    QList<QVariant> data() const;

  private:
    class Private;
    Private* const d;
};

/**
  Query is a class that represents an individual XML-RPC call.
  This is an internal class and is only used by the Server class.
 */
class Query : public QObject
{
  Q_OBJECT

  public:
    static Query *create( const QVariant &id = QVariant(), QObject *parent = 0 );

  public slots:
    void call( const QString &server, const QString &method,
               const QList<QVariant> &args = QList<QVariant>(),
               const QString &userAgent = "KDE-XMLRPC" );

  Q_SIGNALS:
    void message( const QList<QVariant> &result, const QVariant &id );
    void fault( int, const QString&, const QVariant &id );
    void finished( Query* );

  private:
    Query( const QVariant &id, QObject *parent = 0 );
    virtual ~Query();

    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotData( KIO::Job*, const QByteArray& ) )
    Q_PRIVATE_SLOT( d, void slotResult( KIO::Job* ) )
};

} // namespace XmlRpc

#endif

