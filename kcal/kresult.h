/*
  This file is part of KDE.

  Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
  Boston, MA  02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CalendarLocal class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/
#ifndef KRESULT_H
#define KRESULT_H

#include <QtCore/QString>
#include "kcal_export.h"

namespace KCal {

/**
  @brief
  This class represents the result of an operation. It's meant to be used
  as return value of functions for returning status and especially error
  information.

  There are three main types of result: Ok (operation successful completed),
  InProgress (operation still in progress) and Error (operation failed).
  InProgress is used by asynchronous operations. Functions which start an
  operation and return before the operation is finished should return the
  InProgress type result.

  An error result can include information about the type of the error and a
  detailed error message. Translated error messages for the error types are
  available through the message() function. Additional detailed error messages
  can be set by the setDetails() function. A full error message including the
  type specific message and the details is available through fullMessage().

  KResult objects can be chained using the chain function. If an operation
  executes a suboperation which indicates failure by returning a KResult object
  the operation can create a new KResult object and chain the suboperation's
  KResult object to it. The error information of chained results is available
  through the chainedMessage() function.

  Examples:

    A function returning ok:

      KResult load()
      {
        return KResultOk();
      }

    Alternative notation:

      KResult load()
      {
        return KResult::Ok;
      }

    A function returning an error with a specific error message:

      KResult load()
      {
        return KResultError( i18n("Details about error") );
      }

    A function returning an error of a sepcific type:

      KResult load()
      {
        return KResultError( KResult::InvalidUrl );
      }

    Chaining errors:

      KResult loadFile()
      {
        KResult result = mFile.load();
        if ( !result.isError() ) {
          return KResultError( "File load error" ).chain( result );
        } else {
          return result;
        }
      }

    Checking for errors:

      KResult load() { ... }

      ...
      if ( !load() ) error();
*/
class KCAL_EXPORT KResult
{
  public:
    /**
      The different types of results.
    */
    enum Type {
      Ok,         /**< Operation successfully completed */
      InProgress, /**< Operation still in-progress */
      Error       /**< Operation failed */
    };

    /**
      The different types of error conditions.
    */
    enum ErrorType {
      NotAnError,         /**< Not an error */
      Undefined,          /**< Undefined error */
      InvalidUrl,         /**< Invalid URL */
      WrongParameter,     /**< Invalid parameter */
      ConnectionFailed,   /**< unable to establish a connection */
      WriteError,         /**< Write error */
      ReadError,          /**< Read error */
      ParseError,         /**< Parse error */
      WrongSchemaRevision /**< Invalid schema revision */
    };

    /**
      Constructs a KResult object. Default Type is Ok.
    */
    KResult();

    /**
      Copy constructor.
    */
    KResult( const KResult & );

    /**
      Creates a KResult object of the specified Type.
      @param type is the result #Type.
    */
    explicit KResult( Type type );

    /**
      Creates a KResult object of the specified ErrorType and an optional
      detailed error message.
      @param error is the #ErrorType.
      @param details is a QString containing optional details to add
      to the message corresponding to this error.
    */
    explicit KResult( ErrorType error, const QString &details = QString() );

    /**
      Destroys the result.
    */
    ~KResult();

    /**
      Behave like a bool in the corresponding context. Ok and InProgress are
      considered as success and return true, Error is considered as failure and
      returns false.
    */
    operator bool() const;

    /**
      Returns true if the result is Ok.
    */
    bool isOk() const;

    /**
      Returns true if the result is InProgress.
    */
    bool isInProgress() const;

    /**
      Returns true if the result is Error.
    */
    bool isError() const;

    /**
      Returns the specific result ErrorType.
    */
    ErrorType error() const;

    /**
      Returns a translated string describing the result corresponding to Type
      and ErrorType.
      @see fullMessage().
    */
    QString message() const;

    /**
      Sets a detailed error message. This error message should include all
      details needed to understand and recover from the error. This could be
      information like the URL which was tried, the file which could not be
      written or which parameter was missing.

      @param details is a QString containing details to add to the message
      for this error.
      @see details().
    */
    void setDetails( const QString &details );

    /**
      Returns the detailed error message.
      @see setDetails().
    */
    QString details() const;

    /**
      Returns the full error message. This includes the type-specific message
      (see message()) and the detailed message (see details()).
    */
    QString fullMessage() const;

    /**
      Chains result objects. This can be used to remember the cause of an error.
      The full error messages including the messages from chained objects can be
      accessed through chainedMessage().
      @param result is another KResult to chain this one to.
    */
    KResult &chain( const KResult &result );

    /**
      Returns true if the KResult object has a chained KResult object;
      else returns false.
    */
    bool hasChainedResult() const;

    /**
      Returns a chained KResult object.
    */
    KResult chainedResult() const;

    /**
      Returns an error message including full details of all chained messages.
      This can constitute a backtrace of a error.
    */
    QString chainedMessage() const;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

/**
  @brief
  Convenience class for creating a KResult of type Ok.
*/
class KCAL_EXPORT KResultOk : public KResult
{
  public:
    /**
      Create KResult object of type Ok.
    */
    KResultOk() : KResult( Ok ), d( 0 ) {}

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

/**
  @brief
  Convenience class for creating a KResult of type InProgress.
*/
class KCAL_EXPORT KResultInProgress : public KResult
{
  public:
    /**
      Create KResult object of type InProgress.
    */
    KResultInProgress() : KResult( InProgress ), d( 0 ) {}

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

/**
  @brief
  Convenience class for creating a KResult of type Error.
*/
class KCAL_EXPORT KResultError : public KResult
{
  public:
    /**
      Create KResult object of type Error.
    */
    KResultError() : KResult( Error ), d( 0 ) {}

    /**
      Create KResult object of type Error with given error type and optionally
      a detailed error message.

      @param error is the #ErrorType.
      @param details is a QString containing optional details to add
      to the message corresponding to this error.
    */
    explicit KResultError( ErrorType error, const QString &details = QString() )
      : KResult( error, details ), d( 0 ) {}

    /**
      Create KResult object of type Error with given detailed error message.

      @param details is a QString containing optional details to add
      to the message corresponding to this error.
    */
    KResultError( const QString &details ) :
      KResult( Undefined, details ), d( 0 ) {}

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( KResultError )
    class Private;
    Private *const d;
    //@endcond PRIVATE
};

}

#endif
