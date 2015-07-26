project(microblog)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5328)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories (${KDE4_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIR} ${QT_INCLUDES})

set(microblog_LIB_SRCS
   statusitem.cpp
   )

include_directories(
  ${CMAKE_BINARY_DIR}/kpimutils
)

add_library(microblog ${LIBRARY_TYPE} ${microblog_LIB_SRCS})
generate_export_header(microblog)

target_link_libraries(microblog LINK_PRIVATE ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} kpimutils )

set_target_properties(microblog PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS microblog EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
         statusitem.h
         ${CMAKE_CURRENT_BINARY_DIR}/microblog_export.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/microblog COMPONENT Devel)
