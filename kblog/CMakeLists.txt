project(kblog)

include_directories(${Boost_INCLUDE_DIR})

add_subdirectory( tests )

########### next target ###############

set(kblog_LIB_SRCS
blog.cpp
blogposting.cpp
blogpostingcomment.cpp
blogmedia.cpp
blogger1.cpp
gdata.cpp
livejournal.cpp
metaweblog.cpp
movabletype.cpp
)


kde4_add_library(kblog SHARED ${kblog_LIB_SRCS})

target_link_libraries(kblog kxmlrpcclient syndication ${KDE4_KIO_LIBS} kcal )

set_target_properties(kblog PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kblog  DESTINATION ${LIB_INSTALL_DIR} )

########### install files ##############

install( FILES kblog_export.h blog.h blogposting.h blogpostingcomment.h blogmedia.h blogger1.h gdata.h livejournal.h metaweblog.h movabletype.h DESTINATION ${INCLUDE_INSTALL_DIR}/kblog)
