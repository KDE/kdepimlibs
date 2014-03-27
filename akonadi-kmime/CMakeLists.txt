cmake_minimum_required(VERSION 2.8.12)

project(Akonadi-KMime)

# ECM setup
find_package(ECM 0.0.11 CONFIG REQUIRED)
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

set(KF5_VERSION "4.97.0")

# FIXME ? KF5AbcConfig ? KF5AddressBookConfig ? anything else ?
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX AKONADIKMIME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/akonadi-kmime_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiKMimeConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5 ${KF5_VERSION} CONFIG REQUIRED COMPONENTS KIO KDE4Support)
find_package(KF5Akonadi REQUIRED)
find_package(KF5KMime REQUIRED)

find_package(LibXslt)
set_package_properties(LibXslt PROPERTIES
    DESCRIPTION "xsltproc"
    URL "http://xmlsoft.org/XSLT/"
    TYPE REQUIRED
    PURPOSE "Needed to generate D-Bus interface specifications"
)

########### Targets ###########

add_subdirectory(src)
# TODO: Port tests
#add_subdirectory(tests)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5AkonadiKMime")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5AkonadiKMimeConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiKMimeConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiKMimeConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiKMimeConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5AkonadiKMimeTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5AkonadiKMimeTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/akonadi-kmime_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${KAbc_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
