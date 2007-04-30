project(kdepimlibs)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

macro_optional_find_package(Boost)
macro_log_feature(BOOST_FOUND "boost" "Boost C++ Libraries" "http://www.boost.org/" FALSE "" "Needed by libsyndication")

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

add_subdirectory(kpimutils)
add_subdirectory(kimap)
add_subdirectory(kxmlrpcclient)
add_subdirectory(kcal)
add_subdirectory(ktnef)
if(BOOST_FOUND)
  add_subdirectory(syndication)
endif(BOOST_FOUND)
add_subdirectory(kldap)
add_subdirectory(kmime)
add_subdirectory(kresources)
add_subdirectory(kabc)
add_subdirectory(kblog)

# Move not complete. Do not enable. Toma
# add_subdirectory(mailtransport)
add_subdirectory(cmake)

macro_display_feature_log()
