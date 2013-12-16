project(kblog)

find_package(Qt5 NO_MODULE REQUIRED COMPONENTS Test)
find_package(KF5 NO_MODULE REQUIRED COMPONENTS CoreAddons KIO)

include_directories( ${Boost_INCLUDE_DIR} "${syndication_BINARY_DIR}"
  ${kcalcore_BINARY_DIR}
)


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

target_link_libraries(kblog kxmlrpcclient syndication kcalcore KF5::CoreAddons KF5::KIOCore)

set_target_properties(kblog PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kblog EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ##############

install( FILES kblog_export.h blog.h blogpost.h blogcomment.h blogmedia.h blogger1.h gdata.h metaweblog.h movabletype.h wordpressbuggy.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/kblog COMPONENT Devel)
