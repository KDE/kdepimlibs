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

namespace KMime {

/**
  Finds the first header of type @p name in @p src.
  @param end The end index of the header.
  @param dataBegin begin of the data part of the header, -1 if not found.
  @param folded true if the headder is folded into multiple lines
  @returns the begin index of the header, -1 if not found.
*/
extern int indexOfHeader( const QByteArray &src, const QByteArray &name, int &end, int &dataBegin, bool *folded = 0 );

/**
  Removes the first occurrence of the @p name from @p head.
*/
extern void removeHeader( QByteArray &head, const QByteArray &name );

}

// @endcond

#endif
