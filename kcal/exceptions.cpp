/*
    This file is part of the kcal library.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Exception and ErrorFormat classes.

  We don't use actual C++ exceptions right now. These classes are currently
  returned by an error function; but we can build upon them, if/when we start
  to use C++ exceptions.

  @brief
  Exceptions base class along with the Calendar ErrorFormat class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "exceptions.h"
#include "calformat.h"

#include <klocale.h>

using namespace KCal;

Exception::Exception( const QString &message )
{
  mMessage = message;
}

Exception::~Exception()
{
}

QString Exception::message()
{
  if ( mMessage.isEmpty() ) {
    return i18n( "%1 Error", CalFormat::application() );
  } else {
    return mMessage;
  }
}

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::ErrorFormat::Private
{
  public:
    ErrorCodeFormat mCode;
};
//@endcond

ErrorFormat::ErrorFormat( ErrorCodeFormat code, const QString &message )
  : Exception( message ), d( new KCal::ErrorFormat::Private )
{
  d->mCode = code;
}

ErrorFormat::~ErrorFormat()
{
  delete d;
}

QString ErrorFormat::message()
{
  QString message = "";

  switch ( d->mCode ) {
  case LoadError:
    message = i18n( "Load Error" );
    break;
  case SaveError:
    message = i18n( "Save Error" );
    break;
  case ParseErrorIcal:
    message = i18n( "Parse Error in libical" );
    break;
  case ParseErrorKcal:
    message = i18n( "Parse Error in libkcal" );
    break;
  case NoCalendar:
    message = i18n( "No calendar component found." );
    break;
  case CalVersion1:
    message = i18n( "vCalendar Version 1.0 detected." );
    break;
  case CalVersion2:
    message = i18n( "iCalendar Version 2.0 detected." );
    break;
  case CalVersionUnknown:
    message = i18n( "Unknown calendar format detected." );
    break;
  case Restriction:
    message = i18n( "Restriction violation" );
    break;
  }

  if ( !mMessage.isEmpty() ) {
    message += ": " + mMessage;
  }

  return message;
}

ErrorFormat::ErrorCodeFormat ErrorFormat::errorCode()
{
  return d->mCode;
}
