/******************************************************************************
 *   Copyright (C) 2003 - 2004 by Frerich Raabe <raabe@kde.org>               *
 *                                Tobias Koenig <tokoe@kde.org>               *
 *   Copyright (C) 2006 by Narayan Newton <narayannewton@gmail.com>           *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING.BSD'.                        *
 *****************************************************************************/
/**
  @file

  This file is part of KXmlRpc and defines our internal classes.

  @author Frerich Raabe <raabe@kde.org>
  @author Tobias Koenig <tokoe@kde.org>
  @author Narayan Newton <narayannewton@gmail.com>
*/

#ifndef KXML_RPC_QUERY_H
#define KXML_RPC_QUERY_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <kio/job.h>

#include "kxmlrpcclient.h"

class QString;

/** Namespace for XmlRpc related classes */
namespace KXmlRpc {

/**
  @brief
  Query is a class that represents an individual XML-RPC call.

  This is an internal class and is only used by the KXmlRpc::Server class.
  @internal
 */
class KXMLRPCCLIENT_EXPORT Query : public QObject
{
  friend class Result;
  Q_OBJECT

  public:
    /**
      Constructs a query.

      @param id an optional id for the query.
      @param parent an optional parent for the query.
     */
    static Query *create( const QVariant &id = QVariant(), QObject *parent = 0 );

  public Q_SLOTS:
    /**
      Calls the specified method on the specified server with
      the given argument list.

      @param server the server to contact.
      @param method the method to call.
      @param args an argument list to pass to said method.
      @param jobMetaData additional arguments to pass to the KIO::Job.
     */
    void call( const QString &server, const QString &method,
               const QList<QVariant> &args,
               const QMap<QString, QString> &jobMetaData );

  Q_SIGNALS:
    /**
      A signal sent when we receive a result from the server.
     */
    void message( const QList<QVariant> &result, const QVariant &id );

    /**
      A signal sent when we receive an error from the server.
     */
    void fault( int, const QString&, const QVariant &id );

    /**
      A signal sent when a query finishes.
     */
    void finished( Query * );

  private:
    Query( const QVariant &id, QObject *parent = 0 );
    virtual ~Query();

    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void slotData( KIO::Job *, const QByteArray & ) )
    Q_PRIVATE_SLOT( d, void slotResult( KJob * ) )
};

/**
  @brief
  Result is an internal class that represents a response
  from a XML-RPC server.

  This is an internal class and is only used by Query.
  @internal
 */
class Result
{
  friend class Query;
  friend class Query::Private;

  public:
    /**
      Constructs a result.
     */
    Result();

    /**
      Constructs a result based on another result.
     */
    Result( const Result &other );

    /**
      Destroys a result.
     */
    virtual ~Result();

    /**
      Assigns the values of one result to this one.
     */
    Result &operator=( const Result &other );

    /**
      Returns true if the method call succeeded, false
      if there was an XML-RPC fault.

      @see errorCode(), errorString()
     */
    bool success() const;

    /**
      Returns the error code of the fault.

      @see success(), errorString()
     */
    int errorCode() const;

    /**
      Returns the error string that describes the fault.

      @see success, errorCode()
     */
    QString errorString() const;

    /**
      Returns the data sent to us from the server.
     */
    QList<QVariant> data() const;

  private:
    class Private;
    Private *const d;
};

} // namespace XmlRpc

#endif

