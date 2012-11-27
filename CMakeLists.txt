project(kdepimlibs)

# where to look first for cmake modules. This line must be the first one or cmake will use the system's FindFoo.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")


############### The kdepimlibs version (used e.g. in KdepimLibsConfig.cmake) ###############
set(KDEPIMLIBS_VERSION_MAJOR 4)
set(KDEPIMLIBS_VERSION_MINOR 9)
set(KDEPIMLIBS_VERSION_PATCH 4)
set(KDEPIMLIBS_VERSION ${KDEPIMLIBS_VERSION_MAJOR}.${KDEPIMLIBS_VERSION_MINOR}.${KDEPIMLIBS_VERSION_PATCH})


############### search packages used by KDE ###############

# cmake would not find kde for xcompile if that is not set 
if(WINCE)
    set(KDE4_DATA_DIR "${CMAKE_PREFIX_PATH}/share/apps")
endif()

set (QT_MIN_VERSION "4.7.0")
# kdepim depends on the latest released version of kdelibs, ie. 4.x kdepim depends on 4.x-1 kdelibs
find_package(KDE4 4.7.0 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(KDEPlatformProfile)


############### Desktop vs. Mobile options ##############

if (KDE_PLATFORM_PROFILE STREQUAL "Mobile")
  set(_default_mobile_ui TRUE)
else()
  set(_default_mobile_ui FALSE)
endif()


############### Build Options ###############

option(KDEPIM_ONLY_KLEO "Only build the libraries needed by Kleopatra." FALSE)
option(KDEPIM_NO_KRESOURCES "Build without legacy KResource support." ${KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED})
option(KDEPIM_NO_KCAL "Build without legacy KCal." ${KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED})
option(KDEPIM_MOBILE_UI "Build UI for mobile devices instead of for desktops" ${_default_mobile_ui})
option(STATIC_LIBRARY "Build kdepimlibs as static libraries." FALSE)

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
if (WIN32)
  find_package(Boost ${Boost_MINIMUM_VERSION}) # (jstaniek) temp. fix unless we have graph library, not just headers
else()
  find_package(Boost ${Boost_MINIMUM_VERSION} COMPONENTS graph)
endif()
macro_log_feature(Boost_FOUND "Boost C++ Libraries" "Required by core components" "http://www.boost.org" TRUE "${Boost_MINIMUM_VERSION}" "Boost must include the boost-graph library")



#FindGpgme.cmake already handles the log message but we must ensure it is required.
find_package(Gpgme REQUIRED)
# configure macros
if (GPGME_FOUND)
  include (gpgme++/ConfigureChecks.cmake)
endif()

if (NOT KDEPIM_ONLY_KLEO)
  #FindAkonadi.cmake is only there for compatibility reasons, but we don't want to use that.
  set(Akonadi_MIN_VERSION "1.8.1")
  find_package(Akonadi ${Akonadi_MIN_VERSION} QUIET NO_MODULE)
  macro_log_feature(Akonadi_FOUND "Akonadi server libraries" "Access to PIM storage and services" "http://pim.kde.org/akonadi" TRUE "${Akonadi_MIN_VERSION}" "")

  find_package(Sasl2)
  macro_log_feature(SASL2_FOUND "cyrus-sasl" "Login authentication for IMAP and Sieve" "http://asg.web.cmu.edu/sasl/sasl-library.html" TRUE "" "")
  include (ConfigureChecks.cmake)

  set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
  find_package(SharedMimeInfo)
  macro_log_feature(SHARED_MIME_INFO_FOUND "shared-mime-info" "Information about filetypes" "http://freedesktop.org/wiki/Software/shared-mime-info" TRUE "0.30" "")

  set(SDO_MIN_VERSION 0.10)
  set(SOPRANO_MIN_VERSION 2.3.70)

  find_package(SharedDesktopOntologies ${SDO_MIN_VERSION})
  macro_log_feature(SHAREDDESKTOPONTOLOGIES_FOUND "Shared desktop ontologies" "Ontology information for use with Nepomuk" "http://oscaf.sourceforge.net" TRUE "${SDO_MIN_VERSION}" "")

  find_package(Soprano)
  if (NOT WINCE)
    macro_log_feature(Soprano_FOUND "Soprano" "Storage of semantic data" "http://soprano.sourceforge.net" TRUE "${SOPRANO_MIN_VERSION}" "")
  else()
    macro_log_feature(Soprano_FOUND "Soprano" "Storage of semantic data" "http://soprano.sourceforge.net" FALSE "${SOPRANO_MIN_VERSION}" "")
  endif()
  if (Soprano_FOUND AND NOT WINCE)
  include(SopranoAddOntology)
  endif()

  find_package(Nepomuk)
  if (WINCE)
    macro_log_feature(Nepomuk_FOUND "Nepomuk" "Support for the semantic desktop, including indexing of PIM data" "http://www.kde.org" FALSE "" "STRONGLY RECOMMENDED: Nepomuk is part of kdelibs")
  else()
    macro_log_feature(Nepomuk_FOUND "Nepomuk" "Support for the semantic desktop, including indexing of PIM data" "http://www.kde.org" TRUE "" "STRONGLY RECOMMENDED: Nepomuk is part of kdelibs")
  endif()

  set(LIBICAL_MIN_VERSION "0.42")
  find_package(Libical)
  macro_log_feature(LIBICAL_FOUND "libical" "Calendar support" "http://sourceforge.net/projects/freeassociation" TRUE "${LIBICAL_MIN_VERSION}" "Needed by the kcal and kcalcore libraries")

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
if (KDEPIM_NO_KCAL)
  add_definitions(-DKDEPIM_NO_KCAL)
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


############### Now, we add the KDEPIMLibs components ###############
# These targets will always be built
add_subdirectory(cmake)
add_subdirectory(gpgme++)
add_subdirectory(qgpgme)

if (NOT KDEPIM_ONLY_KLEO)
  add_subdirectory(kabc)
  add_subdirectory(kalarmcal)
  if (NOT KDEPIM_NO_KCAL)
    if (NOT WINCE)
    add_subdirectory(kblog)
    endif()
    add_subdirectory(kcal)
  endif()
  add_subdirectory(kcalcore)
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
  if (NOT KDEPIM_NO_KRESOURCES)
    add_subdirectory(kresources)
  endif()
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
macro_display_feature_log()


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
endmacro(MAKE_INSTALL_PATH_ABSOLUTE out in)

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
make_install_path_absolute(KDEPIMLIBS_MIME_DIR            ${MIME_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_SOUND_DIR           ${SOUND_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_TEMPLATES_DIR       ${TEMPLATES_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_WALLPAPER_DIR       ${WALLPAPER_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_KCONF_UPDATE_DIR    ${KCONF_UPDATE_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_AUTOSTART_DIR       ${AUTOSTART_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_XDG_APPS_DIR        ${XDG_APPS_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_XDG_DIRECTORY_DIR   ${XDG_DIRECTORY_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_SYSCONF_DIR         ${SYSCONF_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_MAN_DIR             ${MAN_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_INFO_DIR            ${INFO_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_SERVICES_DIR        ${SERVICES_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS_SERVICETYPES_DIR    ${SERVICETYPES_INSTALL_DIR})

# Used in configure_file() and install(EXPORT)
set(KDEPIMLIBS_TARGET_PREFIX KDEPIMLibs__)

# this file is installed and contains all necessary information about the installed kdepimlibs, it also loads the file with the exported targets
configure_file(KdepimLibsConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/KdepimLibsConfig.cmake" @ONLY)

# this file will be installed too and will be used by cmake when searching for the Config.cmake file to check the version of kdepimlibs, Alex
macro_write_basic_cmake_version_file(${CMAKE_CURRENT_BINARY_DIR}/KdepimLibsConfigVersion.cmake
                                     ${KDEPIMLIBS_VERSION_MAJOR} ${KDEPIMLIBS_VERSION_MINOR} ${KDEPIMLIBS_VERSION_PATCH})

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
