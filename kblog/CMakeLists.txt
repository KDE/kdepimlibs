project(kdepimlibs)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories (${KDE4_LIB_DIR})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})


include (ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

project( kblog )
add_subdirectory( tests )

include_directories( ${KDE4_KIO_INCLUDES} )

add_definitions(${QDBUS_DEFINITIONS})

########### next target ###############

set(kblog_LIB_SRCS
blog.cpp
blogger.cpp
metaweblog.cpp
)

kde4_automoc(${kblog_LIB_SRCS})

kde4_add_library(kblog SHARED ${kblog_LIB_SRCS})

target_link_libraries(kblog kxmlrpcclient ${KDE4_KIO_LIBS} )

set_target_properties(kblog PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kblog  DESTINATION ${LIB_INSTALL_DIR} )

########### install files ##############

install( FILES kblog.h blog.h blogger.h metaweblog.h DESTINATION ${INCLUDE_INSTALL_DIR}/kblog)
