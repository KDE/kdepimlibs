project(kxmlrpcclient)

find_package(KF5 NO_MODULE REQUIRED COMPONENTS KIO KDE4Support)

add_subdirectory(tests)

########### next target ###############

set(kxmlrpcclient_LIB_SRCS
  client.cpp
  query.cpp
)

kde4_add_library(kxmlrpcclient ${LIBRARY_TYPE} ${kxmlrpcclient_LIB_SRCS})

target_link_libraries(kxmlrpcclient KF5::KIOCore KF5::KDE4Support)

set_target_properties(kxmlrpcclient PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kxmlrpcclient EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  client.h
  kxmlrpcclient_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kxmlrpcclient COMPONENT Devel
)
