project(kdepimlibs)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)
include (gpgmepp/ConfigureChecks.cmake)

#boost is a hard dependency
find_package(Boost)
macro_log_feature(Boost_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org/" TRUE "" "Needed by libsyndication")

#we require the gpgme package for Unix.
#we make gpgme optional for non-Unix (since it does not exist yet).
if(UNIX)
  find_package(Gpgme REQUIRED)
else(UNIX)
  find_package(Gpgme)
endif(UNIX)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

if(GPGME_FOUND)
  add_subdirectory(gpgmepp)
  add_subdirectory(qgpgme)
  add_subdirectory(kleo)
endif(GPGME_FOUND)
add_subdirectory(kabc)
add_subdirectory(kblog)
add_subdirectory(kcal)
add_subdirectory(kimap)
add_subdirectory(kldap)
add_subdirectory(kmime)
add_subdirectory(kpimidentities)
add_subdirectory(kpimutils)
add_subdirectory(kresources)
add_subdirectory(ktnef)
add_subdirectory(kxmlrpcclient)
add_subdirectory(mailtransport)
add_subdirectory(syndication)

add_subdirectory(cmake)

# ...and append all library dependencies
export_library_dependencies(${CMAKE_CURRENT_BINARY_DIR}/KDEPimLibsDependencies.cmake APPEND)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/KDEPimLibsDependencies.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)

macro_display_feature_log()
