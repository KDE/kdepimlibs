cmake_minimum_required(VERSION 2.8.12)

project(KTnef)

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



# FIXME: Do we want the same versions in kdepimlibs and kf5?
set(KF5_VERSION "5.4.0")
set(KDEPIMLIBS_VERSION "4.75.0")

ecm_setup_version(${KDEPIMLIBS_VERSION} VARIABLE_PREFIX KTNEF
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ktnef_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5TnefConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)


set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Tnef")

# FIXME: remove this if/endif when KCalCore is split.
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  find_package(KF5CalendarCore ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
  find_package(KF5CalendarUtils ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
  find_package(KF5PimUtils ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
  find_package(KF5Contacts ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
endif()


ecm_configure_package_config_file(
  "${KTnef_SOURCE_DIR}/KF5TnefConfig.cmake.in"
  "${KTnef_BINARY_DIR}/KF5TnefConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${KTnef_BINARY_DIR}/KF5TnefConfig.cmake"
  "${KTnef_BINARY_DIR}/KF5TnefConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5TnefTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5TnefTargets.cmake
        NAMESPACE KF5::
)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/ktnef_version.h
   DESTINATION ${KF5_INCLUDE_INSTALL_DIR}
   COMPONENT Devel
)


add_definitions(-DTRANSLATION_DOMAIN=\"libktnef5\")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
add_subdirectory(src)
add_subdirectory(autotests)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
