project(kxmlrpcclient)

add_subdirectory(tests)

########### next target ###############

set(kxmlrpcclient_LIB_SRCS
  client.cpp
  query.cpp
)

kde4_add_library(kxmlrpcclient ${LIBRARY_TYPE} ${kxmlrpcclient_LIB_SRCS})
generate_export_header(kxmlrpcclient)

target_link_libraries(kxmlrpcclient ${KDE4_KIO_LIBS})

set_target_properties(kxmlrpcclient PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kxmlrpcclient EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  client.h
  ${CMAKE_CURRENT_BINARY_DIR}/kxmlrpcclient_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kxmlrpcclient COMPONENT Devel
)
