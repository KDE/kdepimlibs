/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KMIME_UTIL_P_H
#define KMIME_UTIL_P_H

// @cond PRIVATE

/* Internal helper functions. Not part of the public API. */

namespace KMime
{

/**
  Finds the header end in @p src. Aligns the @p dataBegin if needed.
  @param dataBegin beginning of the data part of the header
  @param folded true if the headder is folded into multiple lines
  @returns the end index of the header, -1 if the @p dataBegin was -1.
*/
extern int findHeaderLineEnd(const QByteArray &src, int &dataBegin, bool *folded = 0);

/**
  Finds the first header of type @p name in @p src.
  @param end The end index of the header.
  @param dataBegin begin of the data part of the header, -1 if not found.
  @param folded true if the headder is folded into multiple lines
  @returns the begin index of the header, -1 if not found.
*/
extern int indexOfHeader(const QByteArray &src, const QByteArray &name, int &end, int &dataBegin, bool *folded = 0);

/**
  Removes the first occurrence of the @p name from @p head.
*/
// This is used in zero places at the moment.
extern void removeHeader(QByteArray &head, const QByteArray &name);

/**
 * Same as encodeRFC2047String(), but with a crucial difference: Instead of encoding the complete
 * string as a single encoded word, the string will be split up at control characters, and only parts of
 * the sentence that really need to be encoded will be encoded.
 */
extern QByteArray encodeRFC2047Sentence(const QString &src, const QByteArray &charset);

}

// @endcond

#endif
