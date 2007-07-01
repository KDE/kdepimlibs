project(kdepimlibs)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)
#include (gpgmepp/ConfigureChecks.cmake)

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

add_subdirectory(kpimutils)
add_subdirectory(kimap)
add_subdirectory(kxmlrpcclient)
add_subdirectory(kcal)
add_subdirectory(ktnef)
add_subdirectory(syndication)
add_subdirectory(kldap)
add_subdirectory(kmime)
add_subdirectory(kresources)
add_subdirectory(kabc)
add_subdirectory(kblog)
add_subdirectory(kpimidentities)
add_subdirectory(mailtransport)

# dont enable winterz
#add_subdirectory(gpgmepp)
# dont enable winterz
#add_subdirectory(qgpgme)

if(NOT WIN32)
  add_subdirectory(kpgp)
endif(NOT WIN32)
add_subdirectory(cmake)

macro_display_feature_log()
