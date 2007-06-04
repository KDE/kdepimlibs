/******************************************************************************
 *   Copyright (C) 2006 by Narayan Newton <narayannewton@gmail.com>           *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING.BSD'.                        *
 *****************************************************************************/

#ifndef KXMLRPCCLIENT_EXPORT_H
#define KXMLRPCCLIENT_EXPORT_H

#include <kdemacros.h>

#ifndef KXMLRPCCLIENT_EXPORT
# if defined(MAKE_KXMLRPCCLIENT_LIB)
   /* We are building this library */
#  define KXMLRPCCLIENT_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KXMLRPCCLIENT_EXPORT KDE_IMPORT
# endif
#endif

#endif

