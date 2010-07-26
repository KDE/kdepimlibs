/*
  This file is part of the kcalcore library.

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
  defines the CalFormat abstract base class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#ifndef KCALCORE_CALFORMAT_H
#define KCALCORE_CALFORMAT_H

#include "kcalcore_export.h"
#include "exceptions.h"

#include <kcalcore/memorycalendar.h>

#include <QtCore/QString>

namespace KCalCore {

/**
  @brief
  An abstract base class that provides an interface to various calendar formats.

  This is the base class for calendar formats. It provides an interface for the
  generation/interpretation of a textual representation of a calendar.
*/
class KCALCORE_EXPORT CalFormat
{
  public:
    /**
      Constructs a new Calendar Format object.
    */
    CalFormat();

    /**
      Destructor.
    */
    virtual ~CalFormat();

    /**
      Loads a calendar on disk into the calendar associated with this format.

      @param calendar is the MemoryCalendar to be loaded.
      @param fileName is the name of the disk file containing the Calendar data.

      @return true if successful; false otherwise.
    */
    virtual bool load( const MemoryCalendar::Ptr &calendar, const QString &fileName ) = 0;

    /**
      Writes the calendar to disk.

      @param calendar is the MemoryCalendar containing the data to be saved.
      @param fileName is the name of the file to write the calendar data.

      @return true if successful; false otherwise.
    */
    virtual bool save( const MemoryCalendar::Ptr &calendar, const QString &fileName ) = 0;

    /**
      Loads a calendar from a string

      @param calendar is the MemoryCalendar to be loaded.
      @param string is the QString containing the Calendar data.
      @param deleted use deleted incidences
      @param notebook notebook uid

      @return true if successful; false otherwise.
      @see fromRawString(), toString().
    */
    virtual bool fromString( const MemoryCalendar::Ptr &calendar, const QString &string,
                             bool deleted = false, const QString &notebook = QString() ) = 0;

    /**
      Parses a utf8 encoded string, returning the first iCal component
      encountered in that string. This is an overload used for efficient
      reading to avoid utf8 conversions, which are expensive when reading
      from disk.

      @param calendar is the MemoryCalendar to be loaded.
      @param string is the QByteArray containing the Calendar data.
      @param deleted use deleted incidences
      @param notebook notebook uid

      @return true if successful; false otherwise.
      @see fromString(), toString().
    */
    virtual bool fromRawString( const MemoryCalendar::Ptr &calendar, const QByteArray &string,
                                bool deleted = false, const QString &notebook = QString() ) = 0;

    /**
      Returns the calendar as a string.
      @param calendar is the MemoryCalendar containing the data to be saved.
      @param notebook uid use only incidences with given notebook
      @param deleted use deleted incidences

      @return a QString containing the Calendar data if successful;
      an empty string otherwise.
      @see fromString(), fromRawString().
    */
    virtual QString toString( const MemoryCalendar::Ptr &calendar, const QString &notebook = QString(),
                              bool deleted = false ) = 0;

    /**
      Clears the exception status.
    */
    void clearException();

    /**
      Returns an exception, if there is any, containing information about the
      last error that occurred.
    */
    Exception *exception() const;

    /**
      Sets the application name for use in unique IDs and error messages,
      and product ID for incidence PRODID property

      @param application is a string containing the application name.
      @param productID is a string containing the product identifier.
    */
    static void setApplication( const QString &application,
                                const QString &productID );

    /**
      Returns the application name used in unique IDs and error messages.
    */
    static const QString &application(); //krazy:exclude=constref

    /**
      Returns the PRODID string to write into calendar files.
    */
    static const QString &productId(); //krazy:exclude=constref

    /**
      Returns the PRODID string loaded from calendar file.
    */
    QString loadedProductId();

    /**
      Creates a unique id string.
    */
    static QString createUniqueId();

    /**
      Sets an exception that is to be used by the functions of this class
      to report errors.

      @param error is a pointer to an Exception which contains the exception.
    */
    void setException( Exception *error );

  protected:
    /** PRODID string loaded from calendar file. */
    void setLoadedProductId( const QString &id );

    /**
      @copydoc
      IncidenceBase::virtual_hook()
    */
    virtual void virtual_hook( int id, void *data );

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( CalFormat )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
