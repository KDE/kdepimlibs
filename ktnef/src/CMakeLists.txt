remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

set(ktnef_SRCS
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

add_library(KF5Tnef ${ktnef_SRCS})

generate_export_header(KF5Tnef BASE_NAME ktnef)

add_library(KF5::Tnef ALIAS KF5Tnef)

target_include_directories(KF5Tnef INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KTNEF;${Boost_INCLUDE_DIR}>")
target_include_directories(KF5Tnef PUBLIC "$<BUILD_INTERFACE:${KTnef_SOURCE_DIR}/src;${KTnef_BINARY_DIR}/src>")

target_link_libraries(KF5Tnef
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::Abc
  KF5::KDELibs4Support
  KF5::KIOCore
  KF5::PimUtils
)

set_target_properties(KF5Tnef PROPERTIES
    VERSION ${KTNEF_VERSION_STRING}
    SOVERSION ${KTNEF_SOVERSION}
    EXPORT_NAME Tnef
)

install(TARGETS KF5Tnef EXPORT KF5TnefTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(KTnef_CamelCase_HEADERS
  HEADER_NAMES
  Formatter
  KTNEFAttach
  KTNEFDefs
  KTNEFMessage
  KTNEFParser
  KTNEFProperty
  KTNEFPropertySet
  KTNEFWriter

  REQUIRED_HEADERS KTnef_HEADERS
)

install(FILES
    ${KTnef_CamelCase_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/ktnef_export.h
    ${KTnef_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KTNEF
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KTNef LIB_NAME KF5TNef DEPS "CalendarCore CalendarUtils Abc KDELibs4Support KIOCore PimUtils" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KTNEF)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
