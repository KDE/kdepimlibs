/*
    mapi.h

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

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
 * @file
 * This file is part of the API for handling TNEF data and
 * provides functions that convert MAPI keycodes to/from tag strings.
 *
 * @author Michael Goffioul
 */

#ifndef MAPI_H
#define MAPI_H

#include <QtCore/QString>
namespace KTnef
{
/**
 * Convert a keycode to a @acronym MAPI tag string.
 * @param key The input code to convert.
 * @return A QString containing the tag string.
 */
QString mapiTagString(int key);

/**
 * Convert a keycode to a @acronym MAPI named tag string.
 * @param key The input code to convert.
 * @param tag An input tag.
 * @return A QString containing the named tag string.
 */
QString mapiNamedTagString(int key, int tag = -1);
}
#endif
