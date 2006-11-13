/*  -*- c++ -*-
    kmime_dateformatter.h

    KMime, the KDE internet mail/usenet news message library.
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
#ifndef __KMIME_DATEFORMATTER_H__
#define __KMIME_DATEFORMATTER_H__

#include <time.h>
#include <qdatetime.h>
#include <qstring.h>
#include "kmime.h"

namespace KMime {

/**
  @brief
  A class for abstracting date formatting.

  DateFormatter deals with different kinds of date display formats.
  The formats supported by the class include:
  <ul>
      <li> fancy "Today 02:08:35"
      <li> ctime "Sun Mar 31 02:08:35 2002"
      <li> localized "2002-03-31 02:08"
      <li> iso  "2002-03-31 02:08:35"
      <li> rfc2822 "Sun, 31 Mar 2002 02:08:35 -0500"
      <li> custom "whatever you like"
  </ul>

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
      Custom      /**< custom "whatever you like" */
    };

    /**
      Constructs a date formatter.

      @param fType default format used by the class
    */
    DateFormatter( FormatType fType=DateFormatter::Fancy );

    /**
      Destroys the date formatter.
    */
    ~DateFormatter();

    /**
      Returns the currently set format
    */
    FormatType getFormat() const;

    /**
      Sets the currently used format

      @param t is the FormatType to set.
    */
    void setFormat( FormatType t );

    /**
      Constructs a formatted date string from @p otime.

      @param otime time to format
      @param lang used <em>only</em> by the Localized format,
      sets the used language
      @param shortFormat used <em>only</em> by the Localized format,
      is passed to KLocale::formatDateTime
      @param includeSecs used <em>only</em> by the Localized format,
      is passed to KLocale::formatDateTime

      @return A QString containing the formatted date.
    */
    QString dateString( time_t otime, const QString &lang=QString(),
                        bool shortFormat=true, bool includeSecs=false ) const;

    /**
      Overloaded, does exactly what #dateString does (it's slower)

      @param dtime time to format
      @param lang used <em>only</em> by the Localized format,
      sets the used language
      @param shortFormat used <em>only</em> by the Localized format,
      is passed to KLocale::formatDateTime
      @param includeSecs used <em>only</em> by the Localized format,
      is passed to KLocale::formatDateTime

      @return A QString containing the formatted date.
    */
    QString dateString( const QDateTime &dtime, const QString &lang=QString(),
                        bool shortFormat=true, bool includeSecs=false ) const;

    /**
      Makes the class use the custom format for date to string conversions.
      Method accepts the same arguments as QDateTime::toString method and adds
      "Z" expression which is substituted with the RFC-822 style numeric
      timezone (-0500).

      @param format the custom format
    */
    void setCustomFormat( const QString &format );

    /**
      Returns the custom format string.
    */
    QString getCustomFormat() const;

    /**
      Returns an rfc2822 formatted string for the specified @p time.

      @param otime time to use for formatting
    */
    QByteArray rfc2822( time_t otime ) const;

    /**
      Resets the internal clock.
    */
    void reset();

    //statics
    /**
      Convenience function dateString

      @param t specifies the FormatType to use
      @param time time to format
      @param data is either the format when FormatType is Custom,
      or language when FormatType is Localized
      @param shortFormat used <em>only</em> by the Localized format,
      is passed to KLocale::formatDateTime
      @param includeSecs used <em>only</em> by the Localized format,
      is passed to KLocale::formatDateTime
    */
    static QString  formatDate( DateFormatter::FormatType t, time_t time,
                                const QString &data=QString(),
                                bool shortFormat=true,
                                bool includeSecs=false );

    /**
      Convenience function, same as #formatDate but returns the current time
      formatted.

      @param t specifies the FormatType to use
      @param data is either the format when FormatType is Custom,
      or language when FormatType is Localized
      @param shortFormat used <em>only</em> by the Localized format,
      is passed to KLocale::formatDateTime
      @param includeSecs used <em>only</em> by the Localized format,
      is passed to KLocale::formatDateTime
    */
    static QString  formatCurrentDate( DateFormatter::FormatType t,
                                       const QString &data=QString(),
                                       bool shortFormat=true,
                                       bool includeSecs=false );

    /**
      Convenience function, same as #rfc2822
    */
    static QByteArray rfc2822FormatDate( time_t time );

    /**
      Returns true if the current time is on daylight savings time; else false.
    */
    static bool isDaylight();

  protected:
    /**
      Returns fancy formatted date string.

      @param otime time to format
      @internal
    */
    QString fancy( time_t otime ) const ;

    /**
      Returns localized formatted date string.

      @param otime time to format
      @param shortFormat
      @param includeSecs
      @param localeLanguage language used for formatting
      @internal
    */
    QString localized( time_t otime, bool shortFormat=true,
                       bool includeSecs=false,
                       const QString &localeLanguage=QString() ) const;

    /**
      Returns string as formatted with ctime function.

      @param otime time to format
      @internal
    */
    QString cTime( time_t otime ) const;

    /**
      Returns a string in the "%Y-%m-%d %H:%M:%S" format.

      @param otime time to format
      @internal
    */
    QString isoDate( time_t otime ) const;

    /**
      Returns date formatted with the earlier given custom format.

      @param t time used for formatting
      @internal
    */
    QString custom( time_t t ) const;

    /**
      Returns a string identifying the timezone (eg."-0500")

      @param otime time to compute timezone from.
      @internal
    */
    QByteArray zone( time_t otime ) const;

    time_t qdateToTimeT( const QDateTime &dt ) const;

  private:
    //@cond PRIVATE
    FormatType          mFormat;
    mutable time_t      mCurrentTime;
    mutable QDateTime   mDate;
    QString             mCustomFormat;
    static int          mDaylight;
    //@endcond
};

} // namespace KMime

#endif /* __KMIME_DATEFORMATTER_H__ */
