/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#ifndef MAILTRANSPORT_MAILTRANSPORT_EXPORT_H
#define MAILTRANSPORT_MAILTRANSPORT_EXPORT_H

#include <kdemacros.h>

#ifndef MAILTRANSPORT_EXPORT
# if defined(MAKE_MAILTRANSPORT_LIB)
   /* We are building this library */
#  define MAILTRANSPORT_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define MAILTRANSPORT_EXPORT KDE_IMPORT
# endif
#endif

// TODO KDE5: Get rid of all this.
#ifndef MAILTRANSPORT_DEPRECATED
# if defined( USES_DEPRECATED_MAILTRANSPORT_API )
   /* Avoid deprecated warnings from ourselves and the MDA. */
#  define MAILTRANSPORT_DEPRECATED
# else
   /* Show deprecated warnings for anyone else. */
#  define MAILTRANSPORT_DEPRECATED KDE_DEPRECATED
# endif
#endif

#ifndef MAILTRANSPORT_EXPORT_DEPRECATED
# define MAILTRANSPORT_EXPORT_DEPRECATED MAILTRANSPORT_DEPRECATED MAILTRANSPORT_EXPORT
#endif

#endif
