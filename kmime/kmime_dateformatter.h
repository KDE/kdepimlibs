/*  -*- c++ -*-
    kmime_dateformatter.h

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

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
  This file is part of the API for handling @ref MIME data and
  defines the DateFormatter class.

  @brief
  Defines the DateFormatter class.

  @authors the KMime authors (see AUTHORS file)

  @glossary @anchor RFC2822 @anchor rfc2822 @b RFC @b 2822:
  RFC that defines the <a href="http://tools.ietf.org/html/rfc2822">
  Internet Message Format</a>.

  @glossary @anchor ISO8601 @anchor iso8601 @b ISO @b 8601:
  International Standards Organization (ISO) standard that defines the
  <a href="http://http://en.wikipedia.org/wiki/ISO_8601">
  international standard for date and time representations</a>.

  @glossary @anchor ctime @b ctime:
  a Unix library call which returns the local time as a human readable
  ASCII string of the form "Sun Mar 31 02:08:35 2002".
*/

#ifndef __KMIME_DATEFORMATTER_H__
#define __KMIME_DATEFORMATTER_H__

#include <time.h>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include "kmime_export.h"

namespace KMime {

/**
  @brief
  A class for abstracting date formatting.

  This class deals with different kinds of date display formats.
  The formats supported include:
  - @b fancy "Today 02:08:35"
  - @b ctime as with the @ref ctime function, eg. "Sun Mar 31 02:08:35 2002"
  - @b localized according to the control center setting, eg. "2002-03-31 02:08"
  - @b iso  according to the @ref ISO8601 standard, eg. "2002-03-31 02:08:35"
  - @b rfc according to @ref RFC2822 (Section 3.3), eg. "Sun, 31 Mar 2002 02:08:35 -0500"
  - @b custom "whatever you like"
*/
class KMIME_EXPORT DateFormatter
{
  public:
    /**
      The different types of date formats.
    */
    enum FormatType {
      CTime,      /**< ctime "Sun Mar 31 02:08:35 2002" */
      Localized,  /**< localized "2002-03-31 02:08" */
      Fancy,      /**< fancy "Today 02:08:35" */
      Iso,        /**< iso  "2002-03-31 02:08:35" */
      Rfc,        /**< rfc  "Sun, 31 Mar 2002 02:08:35 -0500" */
      Custom      /**< custom "whatever you like" */
    };

    /**
      Constructs a date formatter with a default #FormatType.

      @param ftype is the default #FormatType to use.
    */
    explicit DateFormatter( FormatType ftype=DateFormatter::Fancy );

    /**
      Destroys the date formatter.
    */
    ~DateFormatter();

    /**
      Returns the #FormatType currently set.

      @see setFormat().
    */
    FormatType format() const;

    /**
      Sets the date format to @p ftype.

      @param ftype is the #FormatType.

      @see format().
    */
    void setFormat( FormatType ftype );

    /**
      Constructs a formatted date string from time_t @p t.

      @param t is the time_t to use for formatting.
      @param lang is the language, only used if #FormatType is #Localized.
      @param shortFormat if true, create the short version of the date string,
      only used if #FormatType is #Localized.
      @param includeSecs if true, include the seconds field in the date string,
      only used if #FormatType is #Localized.

      @return a QString containing the formatted date.
    */
    QString dateString( time_t t, const QString &lang=QString(),
                        bool shortFormat=true, bool includeSecs=false ) const;

    /**
      Constructs a formatted date string from QDateTime @p dtime.

      @param dtime is the QDateTime to use for formatting.
      @param lang is the language, only used if #FormatType is #Localized.
      @param shortFormat if true, create the short version of the date string,
      only used if #FormatType is #Localized.
      @param includeSecs if true, include the seconds field in the date string,
      only used if #FormatType is #Localized.

      @return a QString containing the formatted date.
    */
    QString dateString( const QDateTime &dtime, const QString &lang=QString(),
                        bool shortFormat=true, bool includeSecs=false ) const;

    /**
      Sets the custom format for date to string conversions to @p format.
      This method accepts the same arguments as QDateTime::toString(), but
      also supports the "Z" expression which is substituted with the
      @ref RFC2822 (Section 3.3) style numeric timezone (-0500).

      @param format is a QString containing the custom format.

      @see QDateTime::toString(), customFormat().
    */
    void setCustomFormat( const QString &format );

    /**
      Returns the custom format string.

      @see setCustomFormat().
    */
    QString customFormat() const;

    /**
      Resets the cached current date used for calculating the fancy date.
      This should be called whenever the current date changed, i.e. on midnight.
      @deprecated Can be safely removed. The date change is taken care of internally (as of 4.3).
    */
    void reset();

    //static methods
    /**
      Convenience function dateString

      @param ftype is the #FormatType to use.
      @param t is the time_t to use for formatting.
      @param data is either the format when #FormatType is Custom,
      or language when #FormatType is #Localized.
      @param shortFormat if true, create the short version of the date string,
      only used if #FormatType is #Localized.
      @param includeSecs if true, include the seconds field in the date string,
      only used if #FormatType is #Localized.

      @return a QString containing the formatted date.
    */
    static QString formatDate( DateFormatter::FormatType ftype, time_t t,
                               const QString &data=QString(),
                               bool shortFormat=true,
                               bool includeSecs=false );

    /**
      Convenience function, same as formatDate() but returns the current time
      formatted.

      @param ftype is the #FormatType to use.
      @param data is either the format when #FormatType is Custom,
      or language when #FormatType is #Localized.
      @param shortFormat if true, create the short version of the date string,
      only used if #FormatType is #Localized.
      @param includeSecs if true, include the seconds field in the date string,
      only used if #FormatType is #Localized.

      @return a QString containing the formatted date.
    */
    static QString formatCurrentDate( DateFormatter::FormatType ftype,
                                      const QString &data=QString(),
                                      bool shortFormat=true,
                                      bool includeSecs=false );

    /**
      Returns true if the current time is on daylight savings time; else false.
    */
    static bool isDaylight();

  protected:
    /**
      Returns a QString containing the specified time_t @p t formatted
      using the #Fancy #FormatType.

      @param t is the time_t to use for formatting.
    */
    QString fancy( time_t t ) const ;

    /**
      Returns a QString containing the specified time_t @p t formatted
      using the #Localized #FormatType.

      @param t is the time_t to use for formatting.
      @param shortFormat if true, create the short version of the date string.
      @param includeSecs if true, include the seconds field in the date string.
      @param lang is a QString containing the language to use.
    */
    QString localized( time_t t, bool shortFormat=true,
                       bool includeSecs=false,
                       const QString &lang=QString() ) const;

    /**
      Returns a QString containing the specified time_t @p t formatted
      with the ctime() function.

      @param t is the time_t to use for formatting.
    */
    QString cTime( time_t t ) const;

    /**
      Returns a QString containing the specified time_t @p t in the
      "%Y-%m-%d %H:%M:%S" #Iso #FormatType.

      @param t is the time_t to use for formatting.
    */
    QString isoDate( time_t t ) const;

    /**
      Returns a QString containing the specified time_t @p t in the
      #Rfc #FormatType.

      @param t is the time_t to use for formatting.
    */
    QString rfc2822( time_t t ) const;

    /**
      Returns a QString containing the specified time_t @p t formatted
      with a previously specified custom format.

      @param t time used for formatting
    */
    QString custom( time_t t ) const;

    /**
      Returns a QString that identifies the timezone (eg."-0500")
      of the specified time_t @p t.

      @param t time to compute timezone from.
    */
    QByteArray zone( time_t t ) const;

    /**
      Converts QDateTime @p dt to a time_t value.

      @param dt is the QDateTime to be converted.
      @return the time_t equivalent of the specified QDateTime.
    */
    time_t qdateToTimeT( const QDateTime &dt ) const;

  private:
    //@cond PRIVATE
    FormatType          mFormat;
    mutable time_t      mTodayOneSecondBeforeMidnight;
    mutable QDateTime   mUnused; // KDE5: remove
    QString             mCustomFormat;
    static int          mDaylight;
    //@endcond
};

} // namespace KMime

#endif /* __KMIME_DATEFORMATTER_H__ */
