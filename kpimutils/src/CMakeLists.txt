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
  networkaccesshelper_fake.cpp
)

add_library(KF5PimUtils ${kpimutils_SRCS})

generate_export_header(KF5PimUtils BASE_NAME kpimutils)

add_library(KF5::PimUtils ALIAS KF5PimUtils)

target_include_directories(KF5PimUtils INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/kpimutils>")
target_include_directories(KF5PimUtils PUBLIC "$<BUILD_INTERFACE:${KPimUtils_SOURCE_DIR}/src;${KPimUtils_BINARY_DIR}/src>")

target_link_libraries(KF5PimUtils
    KF5::ItemViews
    KF5::Emoticons
    KF5::KMime
)

set_target_properties(KF5PimUtils PROPERTIES
    VERSION ${KPIMUTILS_VERSION_STRING}
    SOVERSION ${KPIMUTILS_SOVERSION}
    EXPORT_NAME PimUtils
)

install(TARGETS KF5PimUtils EXPORT KF5PimUtilsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### Generate Headers ###############
ecm_generate_headers(KPimUtils_CamelCase_HEADERS
  HEADER_NAMES
  Email
  KFileIO
  LinkLocator
  ProgressIndicatorLabel
  ProgressIndicatorWidget
  REQUIRED_HEADERS KPimUtils_HEADERS
  PREFIX KPIMUtils
)

install(FILES
    ${KPimUtils_CamelCase_HEADERS}
    DESTINATION ${INCLUDE_INSTALL_DIR}/KPIMUtils
    COMPONENT Devel
)

# FIXME: The other headers don't have CamelCase headers in KDE4, is that expected?
install(FILES
  ${KPimUtils_BINARY_DIR}/src/kpimutils_export.h
  ${KPimUtils_HEADERS}
  emailvalidator.h
  networkaccesshelper.h
  processes.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kpimutils
  COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME PimUtils LIB_NAME KF5PimUtils DEPS "Emoticons ItemViews KMime" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
