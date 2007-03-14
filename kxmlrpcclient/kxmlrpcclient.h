/******************************************************************************
 *   Copyright (C) 2006 by Narayan Newton <narayannewton@gmail.com>           *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING.BSD'.                        *
 *****************************************************************************/

#ifndef KXML_RPCCLIENT_H
#define KXML_RPCCLIENT_H

#include <kdemacros.h>

#if defined(_WIN32) || defined(_WIN64)
#ifdef MAKE_KXMLRPCCLIENT_LIB
#define KXMLRPCCLIENT_EXPORT KDE_EXPORT
#else
#define KXMLRPCCLIENT_EXPORT KDE_IMPORT
#endif
#else
#define KXMLRPCCLIENT_EXPORT KDE_EXPORT
#endif

#endif

