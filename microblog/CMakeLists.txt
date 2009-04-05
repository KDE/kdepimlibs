project(microblog)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5328)

include_directories (${KDE4_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIR} ${QT_INCLUDES})

set(microblog_LIB_SRCS
   statusitem.cpp
   )

kde4_add_library(microblog SHARED ${microblog_LIB_SRCS})

target_link_libraries(microblog ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} kpimutils )

set_target_properties(microblog PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS microblog EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
         statusitem.h
         microblog_export.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/microblog COMPONENT Devel)
