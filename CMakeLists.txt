project(kdepimlibs)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

macro_optional_find_package(Ldap)
macro_bool_to_01(LDAP_FOUND HAVE_LDAP)
macro_optional_find_package(Sasl2)
macro_bool_to_01(SASL2_FOUND HAVE_SASL2)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories (${KDE4_LIB_DIR})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})


include (ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)


add_subdirectory(emailfunctions)
add_subdirectory(kxmlrpcclient)

if(NOT MSVC)
add_subdirectory(kcal)
endif(NOT MSVC)
add_subdirectory(ktnef)
if(HAVE_LDAP AND HAVE_SASL2)
add_subdirectory(kldap)
endif(HAVE_LDAP AND HAVE_SASL2)

add_subdirectory(kresources)
add_subdirectory(kabc)
add_subdirectory(syndication)

add_subdirectory(cmake)
