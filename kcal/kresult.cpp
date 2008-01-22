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

  @brief
  Classes to represents the result of an operation. It's meant to be used
  as return value of functions for returning status and especially error
  information.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "kresult.h"

#include <klocale.h>
#include <kdebug.h>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::KResult::Private
{
  public:
    Private()
      : mType( Ok ),
        mErrorType( NotAnError ),
        mChainedResult( 0 )
    {
    }

    Private( Type type )
      : mType( type ),
        mChainedResult( 0 )
    {
      if ( mType == Error ) {
        mErrorType = Undefined;
      } else {
        mErrorType = NotAnError;
      }
    }

    Private( ErrorType error, const QString &details )
      : mType( Error ),
        mErrorType( error ),
        mDetails( details ),
        mChainedResult( 0 )
    {
    }

    Type mType;
    ErrorType mErrorType;
    QString mDetails;
    KResult *mChainedResult;
};
//@endcond

KResult::KResult()
  : d( new KCal::KResult::Private )
{
}

KResult::KResult( Type type )
  : d( new KCal::KResult::Private( type ) )
{
}

KResult::KResult( ErrorType error, const QString &details )
  : d( new KCal::KResult::Private( error, details ) )
{
}

KResult::~KResult()
{
  delete d->mChainedResult;
  delete d;
}

KResult::KResult( const KResult &o ) : d( new KCal::KResult::Private )
{
  d->mType = o.d->mType;
  d->mErrorType = o.d->mErrorType;
  d->mDetails = o.d->mDetails;
  if ( o.d->mChainedResult ) {
    d->mChainedResult = new KResult( *o.d->mChainedResult );
  } else {
    d->mChainedResult = 0;
  }
}

KResult::operator bool() const
{
  return !isError();
}

bool KResult::isOk() const
{
  return d->mType == Ok;
}

bool KResult::isInProgress() const
{
  return d->mType == InProgress;
}

bool KResult::isError() const
{
  return d->mType == Error;
}

KResult::ErrorType KResult::error() const
{
  return d->mErrorType;
}

QString KResult::message() const
{
  switch ( d->mType ) {
  case Ok:
    return i18n( "Ok" );
  case InProgress:
    return i18n( "In progress" );
  case Error:
    switch ( d->mErrorType ) {
    case NotAnError:
      return i18n( "Not an error" );
    case Undefined:
      return i18n( "Error" );
    case InvalidUrl:
      return i18n( "Invalid URL" );
    case ConnectionFailed:
      return i18n( "Connection failed" );
    case WriteError:
      return i18n( "Write error" );
    case ReadError:
      return i18n( "Read error" );
    case WrongParameter:
      return i18n( "Wrong Parameter" );
    case ParseError:
      return i18n( "Parse Error" );
    case WrongSchemaRevision:
      return i18n( "Wrong revision of schema" );
    }
  }

  kError() << "KResult::message(): Unhandled case";
  return QString();
}

void KResult::setDetails( const QString &details )
{
  d->mDetails = details;
}

QString KResult::details() const
{
  return d->mDetails;
}

KResult &KResult::chain( const KResult &result )
{
  d->mChainedResult = new KResult( result );
  return *this;
}

bool KResult::hasChainedResult() const
{
  return d->mChainedResult;
}

KResult KResult::chainedResult() const
{
  return *d->mChainedResult;
}

QString KResult::fullMessage() const
{
  QString msg = message();
  if ( !details().isEmpty() ) {
    msg += ": " + details();
  }
  return msg;
}

QString KResult::chainedMessage() const
{
  QString msg = fullMessage();
  if ( hasChainedResult() ) {
    msg += '\n' + chainedResult().chainedMessage();
  }
  return msg;
}
