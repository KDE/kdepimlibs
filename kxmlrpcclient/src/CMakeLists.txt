# FIXME: Make it build with this flag
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

set(kxmlrpcclient_SRCS
  client.cpp
  query.cpp
)

add_library(KF5XmlRpcClient ${kxmlrpcclient_SRCS})

generate_export_header(KF5XmlRpcClient BASE_NAME kxmlrpcclient)

add_library(KF5::XmlRpcClient ALIAS KF5XmlRpcClient)

target_link_libraries(KF5XmlRpcClient KF5::KIOCore KF5::KDE4Support)

set_target_properties(KF5XmlRpcClient PROPERTIES
    VERSION ${KXMLRPCCLIENT_VERSION_STRING}
    SOVERSION ${KXMLRPCCLIENT_SOVERSION}
    EXPORT_NAME KXmlRpcClient
)

ecm_generate_headers(KXmlRpcClient_CamelCase_HEADERS
  HEADER_NAMES
  Client
  REQUIRED_HEADERS KXmlRpcClient_HEADERS
  PREFIX KXmlRpcClient
)

install(TARGETS KF5XmlRpcClient EXPORT KF5XmlRpcClientTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${KXmlRpcClient_CamelCase_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/KXmlRpcClient
  COMPONENT Devel
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kxmlrpcclient_export.h
  ${KXmlRpcClient_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kxmlrpcclient
  COMPONENT Devel
)
