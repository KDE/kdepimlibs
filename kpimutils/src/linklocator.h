/*
  Copyright (c) 2002 Dave Corrie <kde@davecorrie.com>

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
  This file is part of the KDEPIM Utilities library and provides the
  LinkLocator class.

  @brief
  Identifies URLs and email addresses embedded in plaintext.

  @author Dave Corrie \<kde@davecorrie.com\>
*/
#ifndef KPIMUTILS_LINKLOCATOR_H
#define KPIMUTILS_LINKLOCATOR_H

#include "kpimutils_export.h"

#include <QtCore/QString>

namespace KPIMUtils
{

/**
  LinkLocator assists in identifying sections of text that can usefully
  be converted in hyperlinks in HTML. It is intended to be used in two ways:
  either by calling convertToHtml() to convert a plaintext string into HTML,
  or to be derived from where more control is needed.

  please note that you are responsible for handling the links. That means you
  should not execute the link directly but instead open it for example. See
  the KRun documentation about this parameter if applicable.
*/
class KPIMUTILS_EXPORT LinkLocator
{
public:
    /**
      Constructs a LinkLocator that will search a plaintext string
      from a given starting point.

      @param text The string in which to search.
      @param pos  An index into 'text' from where the search should begin.
    */
    explicit LinkLocator(const QString &text, int pos = 0);

    /**
     * Destructor.
     */
    ~LinkLocator();

    /**
      Sets the maximum length of URLs that will be matched by getUrl().
      By default, this is set to 4096 characters. The reason for this limit
      is that there may be possible security implications in handling URLs of
      unlimited length.
      @see maxUrlLen()

      @param length A new maximum length of URLs that will be matched by getUrl().
    */
    void setMaxUrlLen(int length);

    /**
      Returns the current limit on the maximum length of a URL.

      @see setMaxUrlLen().
    */
    int maxUrlLen() const;

    /**
      Sets the maximum length of email addresses that will be matched by
      getEmailAddress(). By default, this is set to 255 characters. The
      reason for this limit is that there may be possible security implications
      in handling addresses of unlimited length.
      @see maxAddressLen().

      @param length The new maximum length of email addresses that will be
      matched by getEmailAddress().
    */
    void setMaxAddressLen(int length);

    /**
      Returns the current limit on the maximum length of an email address.
      @see setMaxAddressLen().
    */
    int maxAddressLen() const;

    /**
      Attempts to grab a URL starting at the current scan position.
      If there is no URL at the current scan position, then an empty
      string is returned. If a URL is found, the current scan position
      is set to the index of the last character in the URL.

      @return The URL at the current scan position, or an empty string.
    */
    QString getUrl();

    /**
      Attempts to grab an email address. If there is an @ symbol at the
      current scan position, then the text will be searched both backwards
      and forwards to find the email address. If there is no @ symbol at
      the current scan position, an empty string is returned. If an address
      is found, then the current scan position is set to the index of the
      last character in the address.

      @return The email address at the current scan position, or an empty string.
    */
    QString getEmailAddress();

    /**
      Converts plaintext into html. The following characters are converted
      to HTML entities: & " < >. Newlines are also preserved.

      @param  plainText  The text to be converted into HTML.
      @param  flags      The flags to consider when processing plainText.
                         Currently supported flags are:
                         - PreserveSpaces, preserves the appearance of sequences
                         of space and tab characters in the resulting HTML.
                         - ReplaceSmileys, replace text smileys with
                         emoticon images.
                         - IgnoreUrls, doesn't parse any URLs.
                         - HighlightText, interprets text highlighting
                         markup like *bold*, _underlined_ and /italic/.
      @param  maxUrlLen  The maximum length of permitted URLs. (@see maxUrlLen().)
      @param  maxAddressLen  The maximum length of permitted email addresses.
                             (@see  maxAddressLen().)
      @return An HTML version of the text supplied in the 'plainText'
      parameter, suitable for inclusion in the BODY of an HTML document.
    */
    static QString convertToHtml(const QString &plainText, int flags = 0,
                                 int maxUrlLen = 4096, int maxAddressLen = 255);

    static const int PreserveSpaces = 0x01;
    static const int ReplaceSmileys = 0x02;
    static const int IgnoreUrls     = 0x04;
    static const int HighlightText  = 0x08;

    /**
      Embeds the given PNG image into a data URL.
      @param iconPath path to the PNG image
      @return A data URL, QString() if the image could not be read.
    */
    static QString pngToDataUrl(const QString &iconPath);

protected:
    /**
      The plaintext string being scanned for URLs and email addresses.
    */
    QString mText;

    /**
      The current scan position.
    */
    int mPos;

    bool atUrl() const;
    bool isEmptyUrl(const QString &url) const;

    /**
      Highlight text according to *bold*, /italic/ and _underlined_ markup.
      @return A HTML string.
    */
    QString highlightedText();

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

};

}

#endif
