/*
    This file is part of the kcal library.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KCAL_EXCEPTIONS_H
#define KCAL_EXCEPTIONS_H

#include <QtCore/QString>
#include "kcal_export.h"

namespace KCal {

/**
  Exceptions base class, currently used as a fancy kind of error code
  and not as an C++ exception.
*/
class Exception
{
  public:
    /**
      Construct an exception with a descriptive message.
      @param message is the message string.
    */
    explicit Exception( const QString &message = QString() );

    /**
      Destructor.
    */
    virtual ~Exception();

    /**
      Returns the exception message.
    */
    virtual QString message();

  protected:
    /** The current exception message. */
    QString mMessage;

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( Exception )
    class Private;
    Private *d;
    //@endcond
};

/**
  Calendar format related error class.
*/
class KCAL_EXPORT ErrorFormat : public Exception
{
  public:
    /**
      The different types of Calendar format errors.
    */
    enum ErrorCodeFormat {
      LoadError,         /**< Load error */
      SaveError,         /**< Save error */
      ParseErrorIcal,    /**< Parse error in libical */
      ParseErrorKcal,    /**< Parse error in libkcal */
      NoCalendar,        /**< No calendar component found */
      CalVersion1,       /**< vCalendar v1.0 detected */
      CalVersion2,       /**< iCalendar v2.0 detected */
      CalVersionUnknown, /**< Unknown calendar format detected */
      Restriction,       /**< Restriction violation */
      UserCancel         /**< User canceled the operation */
    };

    /**
      Creates a format error exception.

      @param code is the exception #ErrorCodeFormat.
      @param message is the exception message string.
    */
    explicit ErrorFormat( ErrorCodeFormat code,
                          const QString &message = QString() );

    /**
      Destructor.
    */
    ~ErrorFormat();

    /**
      Returns the format error message.
    */
    QString message();

    /**
      Returns the format error code.
    */
    ErrorCodeFormat errorCode();

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( ErrorFormat )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
