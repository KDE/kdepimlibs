cmake_minimum_required(VERSION 2.8.12)

project(kioslave)

# ECM setup
find_package(ECM 1.1.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(CMAKE_MODULE_PATH ${kioslave_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

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
set(KF5_VERSION "5.1.0")
set(KDEPIMLIBS_VERSION "4.71.0")

find_package(Sasl2)
set_package_properties(Sasl2 PROPERTIES
                             DESCRIPTION "The Cyrus-sasl library"
                             URL "http://www.cyrussasl.org"
                             TYPE OPTIONAL
)

find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5DocTools ${KF5_VERSION} CONFIG REQUIRED)
if(${CMAKE_SOURCE_DIR} STREQUAL "${kioslave_SOURCE_DIR}")
  find_package(KF5Ldap ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
  find_package(KF5Mbox ${KDEPIMLIBS_VERSION} CONFIG REQUIRED)
endif()
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")


add_subdirectory(src)
add_subdirectory(docs)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${kioslave_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

