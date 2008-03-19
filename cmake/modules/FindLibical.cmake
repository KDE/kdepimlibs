# Find Libical
#
#  LIBICAL_FOUND - system has Libical
#  LIBICAL_INCLUDE_DIRS - the Libical include directory
#  LIBICAL_LIBRARIES - The libraries needed to use Libical

# Copyright (c) 2008, Allen Winter <winter@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

SET (LIBICAL_FIND_REQUIRED ${Libical_FIND_REQUIRED})
if (LIBICAL_INCLUDE_DIR AND LIBICAL_LIBRARIES)

  # Already in cache, be silent
  set(LIBICAL_FIND_QUIETLY TRUE)

else (LIBICAL_INCLUDE_DIR AND LIBICAL_LIBRARIES)

  find_path(LIBICAL_INCLUDE_DIR ical.h)
  find_library(LIBICAL_LIBRARIES NAMES ical )

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBICAL DEFAULT_MSG
                                    LIBICAL_LIBRARIES LIBICAL_INCLUDE_DIR)

  mark_as_advanced(LIBICAL_INCLUDE_DIR LIBICAL_LIBRARIES)

endif (LIBICAL_INCLUDE_DIR AND LIBICAL_LIBRARIES)
