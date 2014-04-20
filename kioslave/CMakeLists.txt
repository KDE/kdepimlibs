cmake_minimum_required(VERSION 2.8.12)

project(kioslave)

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
set(KF5_VERSION "4.98.0")

find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)


add_subdirectory(src)
#reactivate it
#add_subdirectory(docs)

