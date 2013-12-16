project(kmbox)

find_package(KF5 NO_MODULE REQUIRED COMPONENTS CoreAddons)


set(kmbox_LIB_SRCS
    mboxentry.cpp
    mbox_p.cpp
    mbox.cpp
)

add_subdirectory(tests)

add_library(kmbox ${LIBRARY_TYPE} ${kmbox_LIB_SRCS})
generate_export_header(kmbox)

include_directories(
  ${CMAKE_BINARY_DIR}/kmime
)

target_link_libraries(kmbox
  KF5::CoreAddons
  kmime
  kpimutils # TODO KF5 remove this
)

set_target_properties(kmbox PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kmbox EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kmbox_export.h
  mbox.h
  mboxentry.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmbox
  COMPONENT Devel
)
