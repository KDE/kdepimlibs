project(kdepimlibs)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules )

# Used e.g. in KdepimLibsConfig.cmake, Alex
set(KDEPIMLIBS_VERSION_MAJOR 4)
set(KDEPIMLIBS_VERSION_MINOR 3)
set(KDEPIMLIBS_VERSION_PATCH 0)
set(KDEPIMLIBS_VERSION ${KDEPIMLIBS_VERSION_MAJOR}.${KDEPIMLIBS_VERSION_MINOR}.${KDEPIMLIBS_VERSION_PATCH} )

find_package(KDE4  4.2.0  REQUIRED)

include (KDE4Defaults)
include (MacroLibrary)


find_package(Boost REQUIRED)
macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org" TRUE "1.33.1" "Needed by several critical PIM libraries.")


if (NOT ONLY_KLEO)
   macro_optional_find_package(Sasl2)
   macro_log_feature(SASL2_FOUND "cyrus-sasl" "Cyrus SASL API" "http://asg.web.cmu.edu/sasl/sasl-library.html" FALSE "" "STRONGLY RECOMMENDED: Needed to support authentication of logins. IMAP and Sieve kioslaves will not be built.")
endif (NOT ONLY_KLEO)

# gpgme is a hard dependency
find_package(Gpgme REQUIRED)

include (gpgme++/ConfigureChecks.cmake)
include (ConfigureChecks.cmake)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

# TODO(move to kdesupport scripts) temporary way to support gpgme-qt under win32
if (EXISTS ${CMAKE_SOURCE_DIR}/gpgme-qt)
  message(STATUS "gpgme-qt for Windows will be compiled: ${CMAKE_SOURCE_DIR}/gpgme-qt")
  add_subdirectory(gpgme-qt)
endif (EXISTS ${CMAKE_SOURCE_DIR}/gpgme-qt)

macro_optional_add_subdirectory(doc)
add_subdirectory(gpgme++)
add_subdirectory(qgpgme)
add_subdirectory(kmime)

if (NOT ONLY_KLEO)

find_package(Akonadi REQUIRED)
if(Akonadi_FOUND)
   add_subdirectory(akonadi)
endif(Akonadi_FOUND)

# (gpgme++/qgpgme handle their conditions inside their own CMakeLists.txt files)
add_subdirectory(kabc)
add_subdirectory(kblog)
add_subdirectory(kcal)
add_subdirectory(kholidays)
add_subdirectory(kimap)
add_subdirectory(kldap)
add_subdirectory(kpimidentities)
add_subdirectory(kpimutils)
add_subdirectory(kresources)
add_subdirectory(ktnef)
add_subdirectory(kxmlrpcclient)
add_subdirectory(mailtransport)
add_subdirectory(syndication)
add_subdirectory(kioslave)
add_subdirectory(pimtextedit)

endif (NOT ONLY_KLEO)

add_subdirectory(cmake)

macro_display_feature_log()


# now create the KdepimLibsConfig.cmake file, which will be loaded by 
# kdelibs/cmake/modules/FindKdepimLibs.cmake and which has to contain all information
# about the installed kdepimlibs anybody would like to have. Alex

# we need the absolute directories where stuff will be installed too
# but since the variables which contain the destinations can be relative
# or absolute paths, we need this macro to make them all absoulte, Alex
macro(MAKE_INSTALL_PATH_ABSOLUTE out in)
   if (IS_ABSOLUTE "${in}")    # IS_ABSOLUTE is new since cmake 2.4.8
      set(${out} "${in}")
   else (IS_ABSOLUTE "${in}")
      set(${out} "\${KDEPIMLIBS_INSTALL_DIR}/${in}")
   endif (IS_ABSOLUTE "${in}")
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

# this file is installed and contains all necessary information about the installed kdepimibs, it also loads the file with the exported targets
configure_file(KdepimLibsConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/KdepimLibsConfig.cmake" @ONLY)

# this file will be installed too and will be used by cmake when searching for the Config.cmake file to check the version of kdepimlibs, Alex
macro_write_basic_cmake_version_file(${CMAKE_CURRENT_BINARY_DIR}/KdepimLibsConfigVersion.cmake 
                                     ${KDEPIMLIBS_VERSION_MAJOR} ${KDEPIMLIBS_VERSION_MINOR} ${KDEPIMLIBS_VERSION_PATCH})

set(_KdepimLibsConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/KdepimLibs-${KDEPIMLIBS_VERSION}/cmake)
# places where find_package() looks for FooConfig.cmake files:
# CMake >= 2.6.0 looks in lib/Foo*/cmake/, CMake >= 2.6.3 also looks in
# lib/cmake/Foo*/, which packagers prefer. So they can set the KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR
# option to have kdepimlibs install its Config file there. Alex
if(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)
   set(_KdepimLibsConfig_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KdepimLibs-${KDEPIMLIBS_VERSION} )
endif(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)

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

add_subdirectory(includes)
