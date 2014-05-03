set(kmbox_SRCS
    mboxentry.cpp
    mbox_p.cpp
    mbox.cpp
)

add_library(KF5Mbox ${kmbox_SRCS})

generate_export_header(KF5Mbox BASE_NAME kmbox)

add_library(KF5::Mbox ALIAS KF5Mbox)

target_include_directories(KF5Mbox INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KMbox>")
target_include_directories(KF5Mbox PUBLIC "$<BUILD_INTERFACE:${KMbox_SOURCE_DIR}/src;${KMbox_BINARY_DIR}/src>")

target_link_libraries(KF5Mbox
    KF5::CoreAddons
    KF5::Mime
)

set_target_properties(KF5Mbox PROPERTIES
    VERSION ${KMBOX_VERSION_STRING}
    SOVERSION ${KMBOX_SOVERSION}
    EXPORT_NAME Mbox
)

install(TARGETS KF5Mbox EXPORT KF5MboxTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

########### Generate Headers ###############
ecm_generate_headers(KMbox_CamelCase_HEADERS
  HEADER_NAMES
  MBox
  MBoxEntry

  REQUIRED_HEADERS KMbox_HEADERS
)

install(FILES
  ${KMbox_BINARY_DIR}/src/kmbox_export.h
  ${KMbox_HEADERS}
    ${KMbox_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KMbox
    COMPONENT Devel
)


ecm_generate_pri_file(BASE_NAME Mbox LIB_NAME KF5Mbox DEPS "CoreAddons Mime" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KMbox)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
