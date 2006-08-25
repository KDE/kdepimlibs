include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckPrototypeExists)
include(CheckTypeSize)
include(MacroBoolTo01)
# The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
# definitions like _GNU_SOURCE that are needed on each platform.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})

#now check for dlfcn.h using the cmake supplied CHECK_include_FILE() macro
# If definitions like -D_GNU_SOURCE are needed for these checks they
# should be added to _KDE4_PLATFORM_DEFINITIONS when it is originally
# defined outside this file.  Here we include these definitions in
# CMAKE_REQUIRED_DEFINITIONS so they will be included in the build of
# checks below.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
if (WIN32)
   set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES} )
   set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES} )
endif (WIN32)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(wctype.h HAVE_WCTYPE_H)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)   # vsnprintf.c
check_symbol_exists(iswspace "wctype.h" HAVE_ISWSPACE)  # icalparser.c
check_symbol_exists(S_ISDIR  "sys/stat.h" HAVE_S_ISDIR) # icaldirset.c
check_symbol_exists(S_ISREG  "sys/stat.h" HAVE_S_ISREG) # icaldirset.c
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)
check_type_size("uint64_t" SIZEOF_UINT64_T)
check_type_size("unsigned long long" SIZEOF_UNSIGNED_LONG_LONG)
