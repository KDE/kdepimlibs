
set(kpimutils_SRCS
  email.cpp
  emailvalidator.cpp
  linklocator.cpp
  spellingfilter.cpp
  kfileio.cpp
  processes.cpp
  progressindicatorwidget.cpp
  progressindicatorlabel.cpp
  networkaccesshelper_fake.cpp
)

add_library(KF5PimUtils ${kpimutils_SRCS})

generate_export_header(KF5PimUtils BASE_NAME kpimutils)

add_library(KF5::PimUtils ALIAS KF5PimUtils)

target_include_directories

target_link_libraries(KF5PimUtils
    KF5::ItemViews
    KF5::Emoticons
    KF5::KMime
)

# FIXME ?
#
#  target_link_libraries(kpimutils  KF5::Solid toolhelp)

set_target_properties(KF5PimUtils PROPERTIES
    VERSION ${KPIMUTILS_VERSION_STRING}
    SOVERSION ${KPIMUTILS_SOVERSION}
    EXPORT_NAME PimUtils
)

install(TARGETS KF5PimUtils EXPORT KF5PimUtilsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${KPimUtils_BINARY_DIR}/src/kpimutils_export.h
  email.h
  emailvalidator.h
  linklocator.h
  spellingfilter.h
  kfileio.h
  supertrait.h
  processes.h
  networkaccesshelper.h
  progressindicatorwidget.h
  progressindicatorlabel.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kpimutils
  COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME PimUtils LIB_NAME KF5PimUtils DEPS "Emoticons ItemViews KMime" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
