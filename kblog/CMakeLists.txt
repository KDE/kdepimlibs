project(kblog)

include_directories( ${Boost_INCLUDE_DIR} "${syndication_BINARY_DIR}")

add_subdirectory( tests )

########### next target ###############

set(kblog_LIB_SRCS
   blog.cpp
   blogpost.cpp
   blogcomment.cpp
   blogmedia.cpp
   blogger1.cpp
   gdata.cpp
   # livejournal.cpp
   metaweblog.cpp
   movabletype.cpp
   wordpressbuggy.cpp
   )


kde4_add_library(kblog ${LIBRARY_TYPE} ${kblog_LIB_SRCS})

target_link_libraries(kblog kxmlrpcclient syndication ${KDE4_KIO_LIBS} kcalcore )

set_target_properties(kblog PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kblog EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ##############

install( FILES kblog_export.h blog.h blogpost.h blogcomment.h blogmedia.h blogger1.h gdata.h metaweblog.h movabletype.h wordpressbuggy.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/kblog COMPONENT Devel)
