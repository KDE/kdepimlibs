/*
 * spellingfilter.h
 *
 * Copyright (c) 2002 Dave Corrie <kde@davecorrie.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
/**
  @file
  This file is part of the KDEPIM Utilities library and provides the
  SpellingFilter class.

  @brief
  Filters message text that should not be spellchecked.

  @author Dave Corrie \<kde@davecorrie.com\>
*/

#ifndef KPIMUTILS_SPELLINGFILTER_H
#define KPIMUTILS_SPELLINGFILTER_H

#include "kpimutils_export.h"
#include "kpimutils/linklocator.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace KPIMUtils {

class KPIMUTILS_EXPORT SpellingFilter
{
  public:
    enum UrlFiltering {
      DontFilterUrls,
      FilterUrls
    };
    enum EmailAddressFiltering {
      DontFilterEmailAddresses,
      FilterEmailAddresses
    };

    SpellingFilter( const QString &text, const QString &quotePrefix,
                    UrlFiltering filterUrls = FilterUrls,
                    EmailAddressFiltering filterEmailAddresses = FilterEmailAddresses,
                    const QStringList &filterStrings = QStringList() );
    ~SpellingFilter();

    QString originalText() const;
    QString filteredText() const;

  class TextCensor;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

class SpellingFilter::TextCensor : public LinkLocator
{
  public:
    TextCensor( const QString &s );

    void censorQuotations( const QString &quotePrefix );
    void censorUrls();
    void censorEmailAddresses();
    void censorString( const QString &s );

    QString censoredText() const;

  private:
    bool atLineStart() const;
    void skipLine();

    bool atQuotation( const QString &quotePrefix ) const;
    void skipQuotation( const QString &quotePrefix );
    void findQuotation( const QString &quotePrefix );

    void findEmailAddress();
};

}

#endif
