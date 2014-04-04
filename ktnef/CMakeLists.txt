cmake_minimum_required(VERSION 2.8.12)

project(KTnef)

# ECM setup
find_package(ECM 0.0.12 CONFIG REQUIRED)
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
set(KF5_VERSION "4.97.0")

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KTNEF
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ktnef_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KTnefConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KDE4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Abc ${KF5_VERSION} CONFIG REQUIRED)


# FIXME: remove this if/endif when KCalCore is split.
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  find_package(KF5CalendarCore ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5CalendarUtils ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5PimUtils ${KF5_VERSION} CONFIG REQUIRED)
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5975)

add_subdirectory(src)
add_subdirectory(autotests)
