#
# Find an installation of Akonadi
#
# Sets the following variables:
#  Akonadi_FOUND            - true if Akonadi has been found
#  AKONADI_INCLUDE_DIR      - The include directory
#  AKONADI_COMMON_LIBRARIES - The Akonadi core library to link to (libsoprano)
#  AKONADI_VERSION          - The Akonadi version (string value)
#
# Options:
#  Set AKONADI_MIN_VERSION to set the minimum required Akonadi version (default: 0.80)
#

#if(AKONADI_INCLUDE_DIR AND AKONADI_COMMON_LIBRARIES)

  # read from cache
#  set(Akonadi_FOUND TRUE)

#else(AKONADI_INCLUDE_DIR AND AKONADI_COMMON_LIBRARIES)
  INCLUDE(FindLibraryWithDebug)

  FIND_PATH(AKONADI_INCLUDE_DIR
    NAMES
    akonadi/akonadi_export.h
    PATHS
    ${KDE4_INCLUDE_DIR}
    ${INCLUDE_INSTALL_DIR}
    )

  FIND_LIBRARY_WITH_DEBUG(AKONADI_COMMON_LIBRARIES
    WIN32_DEBUG_POSTFIX d
    NAMES
    akonadiprotocolinternals
    PATHS
    ${KDE4_LIB_DIR}
    ${LIB_INSTALL_DIR}
    )

  # check for all the libs as required to make sure that we do not try to compile with an old version

  if(AKONADI_INCLUDE_DIR AND AKONADI_COMMON_LIBRARIES)
    set(Akonadi_FOUND TRUE)
  endif(AKONADI_INCLUDE_DIR AND AKONADI_COMMON_LIBRARIES)

  # check Akonadi version

  # We set a default for the minimum required version to be backwards compatible
  IF(NOT AKONADI_MIN_VERSION)
    SET(AKONADI_MIN_VERSION "1.99")
  ENDIF(NOT AKONADI_MIN_VERSION)

  #if(Akonadi_FOUND)
  if(FALSE)
    FILE(READ ${AKONADI_INCLUDE_DIR}/soprano/version.h AKONADI_VERSION_CONTENT)
    STRING(REGEX MATCH "AKONADI_VERSION_STRING \".*\"\n" AKONADI_VERSION_MATCH ${AKONADI_VERSION_CONTENT})
    IF (AKONADI_VERSION_MATCH)
      STRING(REGEX REPLACE "AKONADI_VERSION_STRING \"(.*)\"\n" "\\1" AKONADI_VERSION ${AKONADI_VERSION_MATCH})
      if(AKONADI_VERSION STRLESS "${AKONADI_MIN_VERSION}")
        set(Akonadi_FOUND FALSE)
        if(Akonadi_FIND_REQUIRED)
          message(FATAL_ERROR "Akonadi version ${AKONADI_VERSION} is too old. Please install ${AKONADI_MIN_VERSION} or newer")
        else(Akonadi_FIND_REQUIRED)
          message(STATUS "Akonadi version ${AKONADI_VERSION} is too old. Please install ${AKONADI_MIN_VERSION} or newer")
        endif(Akonadi_FIND_REQUIRED)
      endif(AKONADI_VERSION STRLESS "${AKONADI_MIN_VERSION}")
    ENDIF (AKONADI_VERSION_MATCH)
  endif(FALSE)

  set(AKONADI_VERSION "0.80.0")

  if(Akonadi_FOUND)
    if(NOT Akonadi_FIND_QUIETLY)
      message(STATUS "Found Akonadi: ${AKONADI_COMMON_LIBRARIES}")
      message(STATUS "Found Akonadi includes: ${AKONADI_INCLUDE_DIR}")
      message(STATUS "Found Akonadi common libraries: ${AKONADI_COMMON_LIBRARIES}")
    endif(NOT Akonadi_FIND_QUIETLY)
  else(Akonadi_FOUND)
    if(Akonadi_FIND_REQUIRED)
      if(NOT AKONADI_INCLUDE_DIR)
	message(FATAL_ERROR "Could not find Akonadi includes.")
      endif(NOT AKONADI_INCLUDE_DIR)
      if(NOT AKONADI_COMMON_LIBRARIES)
	message(FATAL_ERROR "Could not find Akonadi library.")
      endif(NOT AKONADI_COMMON_LIBRARIES)
    else(Akonadi_FIND_REQUIRED)
      if(NOT AKONADI_INCLUDE_DIR)
        message(STATUS "Could not find Akonadi includes.")
      endif(NOT AKONADI_INCLUDE_DIR)
      if(NOT AKONADI_COMMON_LIBRARIES)
        message(STATUS "Could not find Akonadi library.")
      endif(NOT AKONADI_COMMON_LIBRARIES)
    endif(Akonadi_FIND_REQUIRED)
  endif(Akonadi_FOUND)

mark_as_advanced(AKONADI_COMMON_LIBRARIES AKONADI_INCLUDE_DIR )

#endif(AKONADI_INCLUDE_DIR AND AKONADI_COMMON_LIBRARIES)
