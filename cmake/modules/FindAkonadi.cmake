# Find if Akonadi was installed
# Once done this will define:
#
# Akonadi_FOUND - system has KDE PIM Libraries
# AKONADI_INCLUDE_DIR - the KDE PIM Libraries include directory
# AKONADI_VERSION - The Akonadi version (short string, eg. 1.1.85)
# AKONADI_VERSION_STRING - The Akonadi version (including the SVN revision if available)
#
# Options:
#  Use AKONADI_MIN_VERSION to set the minimum required Akonadi version
#
# The following variables are set:
# AKONADI_BIN_DIR
# AKONADI_CONFIG_DIR
# AKONADI_DBUS_INTERFACES_DIR
# AKONADI_DBUS_SERVICES_DIR
# AKONADI_INCLUDE_DIR
# AKONADI_INSTALL_DIR
# AKONADI_LIB_DIR
# AKONADI_SHARE_DIR
# AKONADI_XDG_MIME_INSTALL_DIR


# Copyright (c) 2009, Christophe Giboudeaux, <cgiboudeaux@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


# AKONADI_MIN_VERSION did exist before KDE 4.3. Let's keep this option
if ( NOT Akonadi_FIND_VERSION AND AKONADI_MIN_VERSION )
  set ( Akonadi_FIND_VERSION ${AKONADI_MIN_VERSION} )
endif ( NOT Akonadi_FIND_VERSION AND AKONADI_MIN_VERSION )

set( _Akonadi_FIND_QUIETLY  ${Akonadi_FIND_QUIETLY} )
find_package( Akonadi ${Akonadi_FIND_VERSION} QUIET NO_MODULE PATHS ${LIB_INSTALL_DIR}/Akonadi/cmake )
set( Akonadi_FIND_QUIETLY ${_Akonadi_FIND_QUIETLY} )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Akonadi DEFAULT_MSG Akonadi_CONFIG )
