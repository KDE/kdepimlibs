/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2009 Urs Wolfer <uwolfer @ kde.org>
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
 *
 */

#ifndef KDEWEBKIT_EXPORT_H
#define KDEWEBKIT_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KDEWEBKIT_EXPORT
# if defined(KDELIBS_STATIC_LIBS)
   /* No export/import for static libraries */
#  define KDEWEBKIT_EXPORT
# elif defined(MAKE_KDEWEBKIT_LIB)
   /* We are building this library */ 
#  define KDEWEBKIT_EXPORT KDE_EXPORT
# else
   /* We are using this library */ 
#  define KDEWEBKIT_EXPORT KDE_IMPORT
# endif
#endif

# ifndef KDEWEBKIT_EXPORT_DEPRECATED
#  define KDEWEBKIT_EXPORT_DEPRECATED KDE_DEPRECATED KDEWEBKIT_EXPORT
# endif

#endif // KDEWEBKIT_EXPORT_H
