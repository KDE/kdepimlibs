cmake_minimum_required(VERSION 2.8.12)

project(KBlog)

# ECM setup
find_package(ECM 1.3.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)

set(KF5_VERSION "5.4.0")
set(KDEPIMLIBS_VERSION "4.72.0")

ecm_setup_version(${KDEPIMLIBS_VERSION} VARIABLE_PREFIX KBLOG
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kblog_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5BlogConfigVersion.cmake"
                        SOVERSION 5)

########### Find packages ###########
find_package(KF5CoreAddons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(Qt5WebKitWidgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

# FIXME: remove this if/endif when KMime is split.
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  find_package(KF5CalendarCore ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
  find_package(KF5Syndication ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
  find_package(KF5XmlRpcClient ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"libkblog5\")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
########### Targets ###########
add_subdirectory(src)
add_subdirectory(autotests)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Blog")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5BlogConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5BlogConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5BlogConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5BlogConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5BlogTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5BlogTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kblog_version.h
   DESTINATION ${KF5_INCLUDE_INSTALL_DIR}
   COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${KBlog_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
