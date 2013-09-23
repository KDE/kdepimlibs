project(kblog)

include_directories(${Boost_INCLUDE_DIR})
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


#add_subdirectory( tests )

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

include_directories(
  ${CMAKE_BINARY_DIR}/kcal
  ${CMAKE_BINARY_DIR}/kxmlrpcclient
  ${CMAKE_BINARY_DIR}/syndication
)

add_definitions(
  -DKCAL_DEPRECATED=
)
# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DKCAL_DEPRECATED_EXPORT=KCAL_EXPORT
)


add_library(kblog ${LIBRARY_TYPE} ${kblog_LIB_SRCS})
generate_export_header(kblog)

target_link_libraries(kblog kxmlrpcclient syndication ${KDE4_KIO_LIBS} kcal )

set_target_properties(kblog PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kblog EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ##############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kblog_export.h blog.h blogpost.h blogcomment.h blogmedia.h blogger1.h gdata.h metaweblog.h movabletype.h wordpressbuggy.h 
         DESTINATION ${INCLUDE_INSTALL_DIR}/kblog COMPONENT Devel)
