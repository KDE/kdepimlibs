project(kdepimlibs)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

# Remove the following lines once we depend on kdelibs trunk again
if (NOT DEFINED INSTALL_TARGETS_DEFAULT_ARGS)
  set(INSTALL_TARGETS_DEFAULT_ARGS  RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
                                    LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                                    ARCHIVE DESTINATION "${LIB_INSTALL_DIR}")
endif (NOT DEFINED INSTALL_TARGETS_DEFAULT_ARGS)

#boost is a hard dependency
find_package(Boost)
macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org" TRUE "1_33_0" "Needed by several critical PIM libraries.")

# FindBoost.cmake from cmake 2.6.0RC9 is slightly broken, in that it changes the policy settings
# readjust them here until cmake 2.6.0 or 2.6.0RC10 is released, Alex
if(COMMAND CMAKE_POLICY)
   # CMP0000: don't require cmake_minimum_version() directly in the top level CMakeLists.txt, FindKDE4Internal.cmake is good enough
   cmake_policy(SET CMP0000 OLD)
   # CMP0002: in KDE4 we have multiple targets with the same name for the unit tests
   cmake_policy(SET CMP0002 OLD)
   # CMP0003: add the link paths to the link command as with cmake 2.4
   cmake_policy(SET CMP0003 OLD)
   # CMP0005: keep escaping behaviour for definitions added via add_definitions()
   cmake_policy(SET CMP0005 OLD)
endif(COMMAND CMAKE_POLICY)


if (NOT ONLY_KLEO)
   macro_optional_find_package(Sasl2)
   macro_log_feature(SASL2_FOUND "cyrus-sasl" "Cyrus SASL API" "http://asg.web.cmu.edu/sasl/sasl-library.html" FALSE "" "Needed to support authentication of logins. IMAP and Sieve kioslaves will not be built.")
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

add_subdirectory(gpgme++)
add_subdirectory(qgpgme)
add_subdirectory(kmime)

if (NOT ONLY_KLEO)

find_package(Akonadi REQUIRED)
macro_log_feature(Akonadi_FOUND "akonadi" "Akonadi shared libraries" "http://pim.kde.org/akonadi" FALSE "" "Needed to support Akonadi KDE client libraries. These are an essential part of the platform.")
if(Akonadi_FOUND)
   add_subdirectory(akonadi)
endif(Akonadi_FOUND)

# (gpgme++/qgpgme handle their conditions inside their own CMakeLists.txt files)
add_subdirectory(kabc)
add_subdirectory(kblog)
add_subdirectory(kcal)
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

endif (NOT ONLY_KLEO)

add_subdirectory(cmake)

# ...and append all library dependencies
string(REGEX REPLACE "gpgmepp" "gpgme++" qgpgme_LIB_DEPENDS "${qgpgme_LIB_DEPENDS}")
export_library_dependencies(${CMAKE_CURRENT_BINARY_DIR}/KDEPimLibsDependencies.cmake APPEND)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/KDEPimLibsDependencies.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)

macro_display_feature_log()

# taken from kdelibs/CMakeLists.txt, needed for having different install prefixes
# for kdepimlibs and kdelibs
# TODO: we probably need all the other variables here as well


# we need the absolute directories where stuff will be installed too
# but since the variables which contain the destinations can be relative
# or absolute paths, we need this macro to make them all absoulte, Alex
macro(MAKE_INSTALL_PATH_ABSOLUTE out in)
   if (UNIX)
      if ("${in}" MATCHES "^/.*")
         set(${out} "${in}")
      else ("${in}" MATCHES "^/.*")
         set(${out} "\${KDE4_INSTALL_DIR}/${in}")
      endif ("${in}" MATCHES "^/.*")
   else (UNIX)
      if ("${in}" MATCHES "^[a-zA-Z]:.*")
        set(${out} "${in}")
      else ("${in}" MATCHES "^[a-zA-Z]:.*")
         set(${out} "\${KDE4_INSTALL_DIR}/${in}")
      endif ("${in}" MATCHES "^[a-zA-Z]:.*")
   endif (UNIX)
endmacro(MAKE_INSTALL_PATH_ABSOLUTE out in)

make_install_path_absolute(KDEPIMLIBS4_DBUS_INTERFACES_DIR ${DBUS_INTERFACES_INSTALL_DIR})
make_install_path_absolute(KDEPIMLIBS4_DBUS_SERVICES_DIR   ${DBUS_SERVICES_INSTALL_DIR})

file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/KDEPimLibsDependencies.cmake " 

set(KDEPIMLIBS4_DBUS_INTERFACES_DIR \"${KDEPIMLIBS4_DBUS_INTERFACES_DIR}\")
set(KDEPIMLIBS4_DBUS_SERVICES_DIR   \"${KDEPIMLIBS4_DBUS_SERVICES_DIR}\")

\n")
