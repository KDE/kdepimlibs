cmake_minimum_required(VERSION 2.8.12)

project(KMime)

# ECM setup
find_package(ECM 0.0.12 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${KMime_SOURCE_DIR}/cmake)

include(GenerateExportHeader)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)


set(KF5_VERSION "4.97.0")
set(QT_REQUIRED_VERSION "5.2.0")

# FIXME: KF5KMime ? KF5Mime ?
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KMIME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kmime_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KMimeConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5ItemViews ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)

########### Targets ###########
add_subdirectory(src)
add_subdirectory(includes)
add_subdirectory(autotests)
add_subdirectory(tests)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5KMime")

ecm_configure_package_config_file(
  "${KMime_SOURCE_DIR}/KF5KMimeConfig.cmake.in"
  "${KMime_BINARY_DIR}/KF5KMimeConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${KMime_BINARY_DIR}/KF5KMimeConfig.cmake"
  "${KMime_BINARY_DIR}/KF5KMimeConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5KMimeTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5KMimeTargets.cmake
        NAMESPACE KF5::
)

install(FILES
   ${KMime_BINARY_DIR}/kmime_version.h
   DESTINATION ${INCLUDE_INSTALL_DIR}
   COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${KMime_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
