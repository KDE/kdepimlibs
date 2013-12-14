project(kmbox)

find_package(KF5 NO_MODULE REQUIRED COMPONENTS CoreAddons)

set(kmbox_LIB_SRCS
    mboxentry.cpp
    mbox_p.cpp
    mbox.cpp
)

add_subdirectory(tests)

kde4_add_library(kmbox ${LIBRARY_TYPE} ${kmbox_LIB_SRCS})

target_link_libraries(kmbox
  KF5::CoreAddons
  kmime
  kpimutils
)

set_target_properties(kmbox PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kmbox EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kmbox_export.h
  mbox.h
  mboxentry.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmbox
  COMPONENT Devel
)
