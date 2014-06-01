cmake_minimum_required(VERSION 2.8.12)

project(MailTransport)

# ECM setup
find_package(ECM 0.0.13 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${MailTransport_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)

add_definitions(-DTRANSLATION_DOMAIN=\"libmailtransport5\")
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5324)

# FIXME: Do we want the same versions in kdepimlibs and kf5?
set(KF5_VERSION "4.98.0")
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5MailTransport")

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX MAILTRANSPORT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mailtransport_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MailTransportConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5ConfigWidgets ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)

# FIXME: remove this if/endif when KMime is split.
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  set(AkonadiServer_MIN_VERSION "1.72.43")
  find_package(Akonadi ${AkonadiServer_MIN_VERSION} CONFIG REQUIRED)

  find_package(KF5Mime ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5Akonadi CONFIG REQUIRED)
  find_package(KF5AkonadiMime CONFIG REQUIRED)
endif()



find_package(Sasl2)
set_package_properties(Sasl2 PROPERTIES
                             DESCRIPTION "The Cyrus-sasl library"
                             URL "http://www.cyrussasl.org"
                             TYPE OPTIONAL
)

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5MailTransportConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MailTransportConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MailTransportConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5MailTransportConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/mailtransport_version.h
   DESTINATION ${KF5_INCLUDE_INSTALL_DIR}
   COMPONENT Devel
)


install(EXPORT KF5MailTransportTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5MailTransportTargets.cmake NAMESPACE KF5::)

########### Targets ###########
add_subdirectory(cmake)
add_subdirectory(src)
add_subdirectory(autotests)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${MailTransport_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

