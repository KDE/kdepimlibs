cmake_minimum_required(VERSION 2.8.12)

project(QGpgme)

# ECM setup
find_package(ECM 1.0.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${QGpgme_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

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
set(KF5_VERSION "5.0.0")
set(KDEPIMLIBS_VERSION "4.70.0")

ecm_setup_version(${KDEPIMLIBS_VERSION} VARIABLE_PREFIX QGPGME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/qgpgme_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5QGpgmeConfigVersion.cmake"
                        SOVERSION 5
)

find_package(Gpgme)
set_package_properties(Gpgme PROPERTIES
                       URL "http://www.gnupg.org/related_software/gpgme/"
                       DESCRIPTION "The GnuPG Made Easy (GPGME) library)"
                       TYPE REQUIRED
)


########### Find packages ###########
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${QGpgme_SOURCE_DIR}")
  find_package(KF5Gpgmepp CONFIG REQUIRED)
endif()


add_definitions(-DTRANSLATION_DOMAIN=\"libqgpgme5\")


########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5QGpgme")



ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5QGpgmeConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5QGpgmeConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5QGpgmeConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5QGpgmeConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5QGpgmeTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5QGpgmeTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/qgpgme_version.h
   DESTINATION ${KF5_INCLUDE_INSTALL_DIR}
   COMPONENT Devel
)

########### Targets ###########
add_subdirectory(src)
#qt5 reactivate it
#add_subdirectory(autotests)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${QGpgme_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
