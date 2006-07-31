project(kdepimlibs)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories (${KDE4_LIB_DIR})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})


include (ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)


add_subdirectory(emailfunctions)

if(NOT MSVC)
add_subdirectory(kcal)
endif(NOT MSVC)
add_subdirectory(ktnef)
if(LDAP_FOUND)
  add_subdirectory(kldap)
endif(LDAP_FOUND)
add_subdirectory(kresources)
add_subdirectory(kabc)

add_subdirectory(cmake)
