project(kdepimlibs)

# Temporary: Make sure we use the right CMake Version
cmake_minimum_required(VERSION 2.8.10.2 FATAL_ERROR)
# Enable CMake's Automoc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_RELAXED_MODE ON)

################# set KDE specific information #################

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(Qt5Core REQUIRED NO_MODULE)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS X11Extras)

find_package(kdeqt5staging REQUIRED NO_MODULE)

# Load CMake, Compiler and InstallDirs settings from KF5 and the following are already somewhat "done" tier1/tier2 libs from kdelibs:
find_package(KF5 MODULE REQUIRED COMPONENTS CMake Compiler InstallDirs
                                            KIdleTime ItemModels KWidgetsAddons KWindowSystem KCodecs KArchive KCoreAddons Solid
                                            KConfig KAuth)

find_package(ki18n REQUIRED NO_MODULE)
find_package(kio REQUIRED NO_MODULE)
find_package(kjs REQUIRED NO_MODULE)
find_package(kguiaddons REQUIRED NO_MODULE)
find_package(kde4support REQUIRED NO_MODULE)

find_package(KDELibs4 REQUIRED NO_MODULE)

include(ECMVersion)
include(ECMWriteVersionHeader)

set_package_properties(ECM PROPERTIES
                       URL "https://projects.kde.org/extra-cmake-modules"
                       DESCRIPTION "Replacement for kdelibs cmake macros"
                       TYPE REQUIRED
                       PURPOSE "Required by the kdepimlibs build system"
)

remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

string(REPLACE "--pedantic-errors" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-pedantic-errors" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

############### The kdepimlibs version ###############
set(KDEPIMLIBS_VERSION_MAJOR 4)
set(KDEPIMLIBS_VERSION_MINOR 11)
set(KDEPIMLIBS_VERSION_PATCH 40)
set(KDEPIMLIBS_VERSION ${KDEPIMLIBS_VERSION_MAJOR}.${KDEPIMLIBS_VERSION_MINOR}.${KDEPIMLIBS_VERSION_PATCH})

############### search packages used by KDE ###############

# cmake would not find kde for xcompile if that is not set
if(WINCE)
    set(KDE4_DATA_DIR "${CMAKE_PREFIX_PATH}/share/apps")
endif()

# Used by options
include(CMakeDependentOption)

############### Desktop vs. Mobile options ##############

if (KDE_PLATFORM_PROFILE STREQUAL "Mobile")
  set(_default_mobile_ui TRUE)
else()
  set(_default_mobile_ui FALSE)
endif()


############### Build Options ###############

option(KDEPIM_ONLY_KLEO "Only build the libraries needed by Kleopatra." FALSE)
cmake_dependent_option(KDEPIM_MOBILE_UI "Build UI for mobile devices instead of for desktops" OFF "_default_mobile_ui" OFF)
option(STATIC_LIBRARY "Build kdepimlibs as static libraries." FALSE)
#option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled." FALSE)
option(KDEPIMLIBS_RUN_ISOLATED_TESTS "Run the isolated tests." FALSE)

############### Load the CTest options ###############
# Setting DART_TESTING_TIMEOUT has no effect on the tests timeout,
# the timeout value must be changed directly in DartConfiguration.tcl
set(_DartConfigFile "${CMAKE_BINARY_DIR}/DartConfiguration.tcl")
if(EXISTS ${_DartConfigFile})
  set(DartTestingTimeout "120")
  file(READ ${_DartConfigFile} _DartConfigFile_content)
  string(REGEX REPLACE "TimeOut: 1500" "TimeOut: ${DartTestingTimeout}" _DartConfigFile_content ${_DartConfigFile_content})
  file(WRITE ${_DartConfigFile} ${_DartConfigFile_content})
endif()

# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake)


############### Find the stuff we need ###############

set(Boost_MINIMUM_VERSION "1.34.0")
find_package(Boost ${Boost_MINIMUM_VERSION} COMPONENTS graph)
set_package_properties(Boost PROPERTIES DESCRIPTION "Boost C++ Libraries" URL "http://www.boost.org" TYPE REQUIRED PURPOSE "Boost must include the boost-graph library")

#FindGpgme.cmake already handles the log message but we must ensure it is required.
find_package(Gpgme REQUIRED)
# configure macros
if (GPGME_FOUND)
  include (gpgme++/ConfigureChecks.cmake)
endif()

if (NOT KDEPIM_ONLY_KLEO)
  #FindAkonadi.cmake is only there for compatibility reasons, but we don't want to use that.
  set(Akonadi_MIN_VERSION "1.9.80")
  find_package(Akonadi ${Akonadi_MIN_VERSION} QUIET NO_MODULE)
  set_package_properties(Akonadi PROPERTIES DESCRIPTION "Akonadi server libraries" URL "http://pim.kde.org/akonadi" TYPE REQUIRED PURPOSE "Access to PIM storage and services")

  find_package(Sasl2)
  set_package_properties(Sasl2 PROPERTIES DESCRIPTION "cyrus-sasl" URL "http://asg.web.cmu.edu/sasl/sasl-library.html" TYPE REQUIRED PURPOSE "Login authentication for IMAP and Sieve")

  include (ConfigureChecks.cmake)

  set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
  find_package(SharedMimeInfo)
  set_package_properties(SharedMimeInfo PROPERTIES DESCRIPTION "shared-mime-info" URL "http://freedesktop.org/wiki/Software/shared-mime-info" TYPE REQUIRED PURPOSE "Information about filetypes")


  set(SDO_MIN_VERSION 0.11)
  set(SOPRANO_MIN_VERSION 2.9)

  find_package(SharedDesktopOntologies ${SDO_MIN_VERSION})
  set_package_properties(SharedDesktopOntologies PROPERTIES DESCRIPTION "Shared desktop ontologies" URL "http://sourceforge.net/projects/oscaf/" TYPE REQUIRED PURPOSE "Ontology information for use with Nepomuk")

  find_package(Soprano)
  if (NOT WINCE)
    set_package_properties(Soprano PROPERTIES DESCRIPTION "Soprano" URL "http://soprano.sourceforge.net" TYPE REQUIRED PURPOSE "Storage of semantic data")
  else()
    set_package_properties(Soprano PROPERTIES DESCRIPTION "Soprano" URL "http://soprano.sourceforge.net" TYPE OPTIONAL PURPOSE "Storage of semantic data")
  endif()
  if (Soprano_FOUND AND NOT WINCE)
  include(SopranoAddOntology)
  endif()

  find_package(NepomukCore)
  set_package_properties(NepomukCore PROPERTIES DESCRIPTION "NepomukCore"  URL "http://www.kde.org" TYPE REQUIRED PURPOSE "Nepomuk extends the search and tagging functionalities in KMail and Akonadi")

  set(LIBICAL_MIN_VERSION "0.42")
  find_package(Libical)
  set_package_properties(Libical PROPERTIES DESCRIPTION "libical" URL "http://sourceforge.net/projects/freeassociation" TYPE REQUIRED PURPOSE "Calendar support")

  find_package(QJSON)
  set_package_properties(QJSON PROPERTIES DESCRIPTION "QJSON" URL "http://qjson.sourceforge.net/" TYPE REQUIRED PURPOSE "Qt library for handling JSON data")

endif()

############### Needed commands before building anything ###############

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

if (KDEPIM_MOBILE_UI)
  add_definitions( -DKDEPIM_MOBILE_UI )
endif()

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${Boost_INCLUDE_DIR})

if (KDEPIM_NO_KRESOURCES)
  add_definitions(-DKDEPIM_NO_KRESOURCES)
endif()
if(WINCE)
  set(STATIC_LIBRARY ON)
  add_definitions(-DKDELIBS_STATIC_LIBS)
endif()

if(STATIC_LIBRARY)
  set(LIBRARY_TYPE STATIC)
  add_definitions(-DKDEPIM_STATIC_LIBS)
  message(STATUS "Building kdepimlibs as static libraries")
else()
  set(LIBRARY_TYPE SHARED)
endif()


include_directories (
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE4_INCLUDES}
  ${Boost_INCLUDE_DIR}
  ${kcalcore_BINARY_DIR}
)

#FindGpgme.cmake already handles the log message but we must ensure it is required.
find_package(Gpgme)
set_package_properties(Gpgme PROPERTIES
                       URL "http://www.gnupg.org/related_software/gpgme/"
                       DESCRIPTION "The GnuPG Made Easy (GPGME) library)"
                       TYPE REQUIRED
)

# configure macros
include (gpgme++/ConfigureChecks.cmake)

include (ConfigureChecks.cmake)


############### Needed commands before building anything ###############

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

if (KDEPIM_MOBILE_UI)
  add_definitions( -DKDEPIM_MOBILE_UI )
endif()

add_subdirectory(cmake)
add_subdirectory(gpgme++)
add_subdirectory(qgpgme)

if (NOT KDEPIM_ONLY_KLEO)
  add_subdirectory(kabc)
  add_subdirectory(kalarmcal)
  if (NOT KDEPIM_NO_KCAL)
  endif()
  add_subdirectory(kcalcore)
  add_subdirectory(kblog)
  add_subdirectory(kcalutils)
  add_subdirectory(akonadi) # depends on kabc is build
  add_subdirectory(kholidays)
  add_subdirectory(kimap)
  add_subdirectory(kioslave)
  add_subdirectory(kldap)
  add_subdirectory(kmbox)
endif()
  add_subdirectory(kmime)
if (NOT KDEPIM_ONLY_KLEO)
  add_subdirectory(kpimidentities)
  add_subdirectory(kpimutils)
  add_subdirectory(kpimtextedit)
  add_subdirectory(ktnef)
  add_subdirectory(kxmlrpcclient)
  add_subdirectory(mailtransport)
  add_subdirectory(microblog)
  add_subdirectory(syndication)
  if (NOT WINCE)
  add_subdirectory(kontactinterface)
  endif()
  # Build the CamelCase headers
  add_subdirectory(includes)
endif()


# doc must be a subdir of kdepimlibs
macro_optional_add_subdirectory(doc)

# All done, let's display what we found...
feature_summary(WHAT ALL
                     INCLUDE_QUIET_PACKAGES
                     FATAL_ON_MISSING_REQUIRED_PACKAGES
               )

############### Here we install some extra stuff  ###############

if (NOT KDEPIM_ONLY_KLEO)
  install(FILES kdepimlibs-mime.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
  update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
endif()


# now create the KdepimLibsConfig.cmake file, which will be loaded by
# kdelibs/cmake/modules/FindKdepimLibs.cmake and which has to contain all information
# about the installed kdepimlibs anybody would like to have. Alex

# we need the absolute directories where stuff will be installed too
# but since the variables which contain the destinations can be relative
# or absolute paths, we need this macro to make them all absoulte, Alex
macro(MAKE_INSTALL_PATH_ABSOLUTE out in)
   if (IS_ABSOLUTE "${in}")    # IS_ABSOLUTE is new since cmake 2.4.8
      set(${out} "${in}")
   else()
      set(${out} "\${KDEPIMLIBS_INSTALL_DIR}/${in}")
   endif()
endmacro()

# all the following variables are put into KdepimLibsConfig.cmake, so
# they are usable by projects using kdepimlibs. Alex
make_install_path_absolute(KDEPIMLIBS_DATA_DIR            ${DATA_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_DBUS_INTERFACES_DIR ${DBUS_INTERFACES_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_DBUS_SERVICES_DIR   ${DBUS_SERVICES_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_INCLUDE_DIR         ${INCLUDE_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_LIB_DIR             ${LIB_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_BIN_DIR             ${BIN_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_LIBEXEC_DIR         ${LIBEXEC_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_SBIN_DIR            ${SBIN_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_HTML_DIR            ${HTML_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_CONFIG_DIR          ${CONFIG_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_ICON_DIR            ${ICON_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_KCFG_DIR            ${KCFG_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_LOCALE_DIR          ${LOCALE_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_SOUND_DIR           ${SOUND_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_TEMPLATES_DIR       ${TEMPLATES_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_WALLPAPER_DIR       ${WALLPAPER_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_KCONF_UPDATE_DIR    ${KCONF_UPDATE_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_AUTOSTART_DIR       ${AUTOSTART_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_XDG_APPS_DIR        ${XDG_APPS_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_XDG_DIRECTORY_DIR   ${XDG_DIRECTORY_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_SYSCONF_DIR         ${SYSCONF_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_MAN_DIR             ${MAN_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_SERVICES_DIR        ${SERVICES_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_SERVICETYPES_DIR    ${SERVICETYPES_INSTALL_DIR})

# Used in configure_file() and install(EXPORT)
set(KDEPIMLIBS_TARGET_PREFIX KDEPIMLibs__)

# this file is installed and contains all necessary information about the installed kdepimlibs, it also loads the file with the exported targets
configure_file(KdepimLibsConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/KdepimLibsConfig.cmake" @ONLY)

# this file will be installed too and will be used by cmake when searching for the Config.cmake file to check the version of kdepimlibs, Alex
include(WriteBasicConfigVersionFile)
write_basic_config_version_file(${CMAKE_CURRENT_BINARY_DIR}/KdepimLibsConfigVersion.cmake
                                  VERSION ${KDEPIMLIBS_VERSION_MAJOR}.${KDEPIMLIBS_VERSION_MINOR}.${KDEPIMLIBS_VERSION_PATCH}
                                  COMPATIBILITY SameMajorVersion)

set(_KdepimLibsConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/KdepimLibs/cmake)
# places where find_package() looks for FooConfig.cmake files:
# CMake >= 2.6.0 looks in lib/Foo*/cmake/, CMake >= 2.6.3 also looks in
# lib/cmake/Foo*/, which packagers prefer. So they can set the KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR
# option to have kdepimlibs install its Config file there. Alex
if(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)
   set(_KdepimLibsConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KdepimLibs)
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KdepimLibsConfigVersion.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/KdepimLibsConfig.cmake
        DESTINATION ${_KdepimLibsConfig_INSTALL_DIR} )

# Install the file with the exported targets, use ${KDEPIMLIBS_TARGET_PREFIX} as prefix for the names of these targets, Alex
install(EXPORT kdepimlibsLibraryTargets
        NAMESPACE ${KDEPIMLIBS_TARGET_PREFIX}
        DESTINATION ${_KdepimLibsConfig_INSTALL_DIR}
        FILE KDEPimLibsLibraryTargetsWithPrefix.cmake )

# Install a KDEPimLibsDependencies.cmake so people using kdepimlibs 4.2 with kdelibs < 4.2 get a useful error message, Alex
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/KDEPimLibsDependencies.cmake "\n message(FATAL_ERROR \"For using this version of kdepimlibs (${KDEPIMLIBS_VERSION}) you need a newer version of kdelibs, please update.\")\n")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KDEPimLibsDependencies.cmake
        DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)

