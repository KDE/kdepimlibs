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
