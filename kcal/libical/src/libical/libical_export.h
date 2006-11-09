#ifndef LIBICAL_EXPORT_H
#define LIBICAL_EXPORT_H

#include <kdemacros.h>

#if defined(_WIN32) || defined(_WIN64)
#ifdef MAKE_LIBICAL_LIB
#define LIBICAL_EXPORT KDE_EXPORT
#else
#define LIBICAL_EXPORT KDE_IMPORT
#endif
#else
#define LIBICAL_EXPORT KDE_EXPORT
#endif

#endif
