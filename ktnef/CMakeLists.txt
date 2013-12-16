project(ktnef)

find_package(KF5 NO_MODULE REQUIRED COMPONENTS KIO KDE4Support)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5975)

add_subdirectory(tests)

########### next target ###############

include_directories(
  ${CMAKE_SOURCE_DIR}/kabc
  ${CMAKE_BINARY_DIR}/kabc
  ${kcalcore_BINARY_DIR}
)

set(ktnef_LIB_SRCS
  ktnefparser.cpp
  ktnefproperty.cpp
  ktnefattach.cpp
  mapi.cpp
  ktnefpropertyset.cpp
  ktnefmessage.cpp
  ktnefwriter.cpp
  lzfu.cpp
  formatter.cpp
)

include_directories(
  ${CMAKE_BINARY_DIR}/kcalcore
  ${CMAKE_BINARY_DIR}/kcalutils
  ${CMAKE_BINARY_DIR}/kabc
  ${CMAKE_BINARY_DIR}/kcal
  ${CMAKE_BINARY_DIR}/kpimutils
)

add_definitions(
  -DKCAL_DEPRECATED=
)
# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DKCAL_DEPRECATED_EXPORT=KCAL_EXPORT
)

add_library(ktnef ${LIBRARY_TYPE} ${ktnef_LIB_SRCS})
generate_export_header(ktnef)

target_link_libraries(ktnef KF5::KIOCore KF5::KDE4Support kabc kcalcore kcalutils)

set_target_properties(ktnef PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS ktnef EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  formatter.h
  ${CMAKE_CURRENT_BINARY_DIR}/ktnef_export.h
  ktnefattach.h
  ktnefdefs.h
  ktnefmessage.h
  ktnefparser.h
  ktnefproperty.h
  ktnefpropertyset.h
  ktnefwriter.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/ktnef COMPONENT Devel
)
