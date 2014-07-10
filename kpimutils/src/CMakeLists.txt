# FIXME: Make it build with this flag
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

set(kpimutils_SRCS
  email.cpp
  emailvalidator.cpp
  linklocator.cpp
  kfileio.cpp
  processes.cpp
  progressindicatorwidget.cpp
  progressindicatorlabel.cpp
)

add_library(KF5PimUtils ${kpimutils_SRCS})

generate_export_header(KF5PimUtils BASE_NAME kpimutils)

add_library(KF5::PimUtils ALIAS KF5PimUtils)

target_include_directories(KF5PimUtils INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KPIMUtils>")
target_include_directories(KF5PimUtils PUBLIC "$<BUILD_INTERFACE:${KPimUtils_SOURCE_DIR}/src;${KPimUtils_BINARY_DIR}/src>")

target_link_libraries(KF5PimUtils
PRIVATE
    KF5::ItemViews
    KF5::Emoticons
    KF5::Mime
)

set_target_properties(KF5PimUtils PROPERTIES
    VERSION ${KPIMUTILS_VERSION_STRING}
    SOVERSION ${KPIMUTILS_SOVERSION}
    EXPORT_NAME PimUtils
)

install(TARGETS KF5PimUtils EXPORT KF5PimUtilsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

########### Generate Headers ###############
ecm_generate_headers(KPimUtils_CamelCase_HEADERS
  HEADER_NAMES
  Email
  KFileIO
  LinkLocator
  ProgressIndicatorLabel
  ProgressIndicatorWidget
  Processes
  PREFIX kpimutils
  REQUIRED_HEADERS KPimUtils_HEADERS
)

install(FILES
    ${KPimUtils_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KPIMUtils/KPIMUtils/
    COMPONENT Devel
)


install(FILES
  ${KPimUtils_BINARY_DIR}/src/kpimutils_export.h
  ${KPimUtils_HEADERS}
  emailvalidator.h
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KPIMUtils/kpimutils
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME PimUtils LIB_NAME KF5PimUtils DEPS "Emoticons ItemViews Mime" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KPIMUtils)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
