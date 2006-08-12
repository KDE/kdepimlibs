/**************************************************************************
*   Copyright (C) 2006 by Narayan Newton <narayannewton@gmail.com>        *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

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

