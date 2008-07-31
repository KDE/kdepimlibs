# Find Libical
#
#  LIBICAL_FOUND - system has Libical
#  LIBICAL_INCLUDE_DIRS - the Libical include directories
#  LIBICAL_LIBRARIES - The libraries needed to use Libical

# Copyright (c) 2008, Allen Winter <winter@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(LIBICAL_FIND_REQUIRED ${Libical_FIND_REQUIRED})
if(LIBICAL_INCLUDE_DIRS AND LIBICAL_LIBRARIES)

  # Already in cache, be silent
  set(LIBICAL_FIND_QUIETLY TRUE)

endif(LIBICAL_INCLUDE_DIRS AND LIBICAL_LIBRARIES)

find_path(LIBICAL_INCLUDE_DIR NAMES ical.h PATH_SUFFIXES libical)
find_path(LIBICALSS_INCLUDE_DIR NAMES icalss.h PATH_SUFFIXES libicalss)
set(LIBICAL_INCLUDE_DIRS ${LIBICAL_INCLUDE_DIR} ${LIBICALSS_INCLUDE_DIR})

find_library(LIBICAL_LIBRARY NAMES ical)
find_library(LIBICALSS_LIBRARY NAMES icalss)
set(LIBICAL_LIBRARIES ${LIBICAL_LIBRARY} ${LIBICALSS_LIBRARY})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBICAL DEFAULT_MSG LIBICAL_LIBRARIES LIBICAL_INCLUDE_DIRS)

mark_as_advanced(LIBICAL_INCLUDE_DIRS LIBICAL_LIBRARIES)
