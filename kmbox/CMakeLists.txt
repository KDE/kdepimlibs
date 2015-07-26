project(kmbox)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

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

target_link_libraries(kmbox LINK_PRIVATE
  ${KDE4_KDECORE_LIBS}
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
