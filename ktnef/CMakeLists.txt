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

kde4_add_library(ktnef ${LIBRARY_TYPE} ${ktnef_LIB_SRCS})

target_link_libraries(ktnef KF5::KIOCore KF5::KDE4Support kabc kcalcore kcalutils)

set_target_properties(ktnef PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS ktnef EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  formatter.h
  ktnef_export.h
  ktnefattach.h
  ktnefdefs.h
  ktnefmessage.h
  ktnefparser.h
  ktnefproperty.h
  ktnefpropertyset.h
  ktnefwriter.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/ktnef COMPONENT Devel
)
