# Find if we have installed Akonadi.
# Once done this will define
#
#  AKONADI_FOUND - system has Akonadi libraries
#  AKONADI_INCLUDE_DIR - the Akonadi include directory
#  AKONADI_LIBRARY - the KDE4 Akonadi client library
#  AKONADI_LIBS - the KDE4 Akonadi client library and all depending libraries
#  AKONADI_KMIME_LIBRARY - the Akonadi mail library

# Copyright (c) 2008, Volker Krause, <vkrause@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(AKONADI_INCLUDE_DIR akonadi/item.h ${KDE4_INCLUDE_DIR})

if (AKONADI_INCLUDE_DIR)
  set(AKONADI_FOUND TRUE)

  # this file contains all dependencies of all libraries of kdepimlibs, Alex
  include(KDEPimLibsDependencies)

  find_library(AKONADI_LIBRARY NAMES akonadi-kde PATHS ${KDE4_LIB_DIR} NO_DEFAULT_PATH )
  set(AKONADI_LIBS ${akonadi-kde_LIB_DEPENDS} ${AKONADI_LIBRARY} )

  find_library(AKONADI_KMIME_LIBRARY NAMES akonadi-kmime PATHS ${KDE4_LIB_DIR} NO_DEFAULT_PATH )
  set(AKONADI_KMIME_LIBS ${akonadi-kmime_LIB_DEPENDS} ${AKONADI_KMIME_LIBRARY} )

  # setup global used KDE include
  set (KDE4_INCLUDES ${KDE4_INCLUDES} ${AKONADI_INCLUDE_DIR})
else (AKONADI_INCLUDE_DIR)
  set(AKONADI_FOUND FALSE)
endif (AKONADI_INCLUDE_DIR)

if (AKONADI_FOUND)
   if (NOT Akonadi_FIND_QUIETLY)
      message(STATUS "Found Akonadi libraries")
   endif (NOT Akonadi_FIND_QUIETLY)
else (AKONADI_FOUND)
   if (Akonadi_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find a Akonadi installation in ${KDE4_INCLUDE_DIR}.")
   endif (Akonadi_FIND_REQUIRED)
endif (AKONADI_FOUND)

#### custom macros ####

macro_optional_find_package(LibXslt)
if (XSLTPROC_EXECUTABLE)

  # generates a D-Bus interface description from a KConfigXT file
  macro( kcfg_generate_dbus_interface _kcfg _name )
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_name}.xml
      COMMAND ${XSLTPROC_EXECUTABLE} --stringparam interfaceName ${_name}
              ${CMAKE_INSTALL_PREFIX}/share/apps/akonadi-kde/kcfg2dbus.xsl
              ${_kcfg}
              > ${CMAKE_CURRENT_BINARY_DIR}/${_name}.xml
      DEPENDS ${CMAKE_INSTALL_PREFIX}/share/apps/akonadi-kde/kcfg2dbus.xsl
              ${_kcfg}
    )
  endmacro( kcfg_generate_dbus_interface )

endif (XSLTPROC_EXECUTABLE)
