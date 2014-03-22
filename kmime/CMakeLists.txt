cmake_minimum_required(VERSION 2.8.12)

project(KMime)

# ECM setup
find_package(ECM 0.0.11 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${KMime_SOURCE_DIR}/cmake)

include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)


set(KF5_VERSION "4.97.0")

# FIXME: KF5KMime ? KF5Mime ?
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KMIME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kmime_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KMimeConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5 NO_MODULE REQUIRED COMPONENTS Codecs ItemViews KDE4Support)

########### Includes ###########
include_directories(${KMime_BINARY_DIR})



########### Targets ###########
add_subdirectory(src)
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

install(EXPORT KF5KMimeTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KMimeTargets.cmake NAMESPACE KF5::)

install(FILES
   ${KMime_BINARY_DIR}/kmime_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
  COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${KMime_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()


include(CheckTimezone)
configure_file (config-kmime.h.cmake ${KMime_BINARY_DIR}/config-kmime.h)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )



########### next target ###############

set(kmime_LIB_SRCS
   kmime_charfreq.cpp
   kmime_util.cpp
   kmime_mdn.cpp
   kmime_codecs.cpp
   kmime_codec_base64.cpp
   kmime_codec_uuencode.cpp
   kmime_codec_qp.cpp
   kmime_codec_identity.cpp
   kmime_parsers.cpp
   kmime_header_parsing.cpp
   kmime_headerfactory.cpp
   kmime_content.cpp
   kmime_contentindex.cpp
   kmime_headers.cpp
   kmime_message.cpp
   kmime_newsarticle.cpp
   kmime_dateformatter.cpp
   boolflags.cpp
   kautodeletehash.cpp )


add_library(kmime ${LIBRARY_TYPE} ${kmime_LIB_SRCS})
generate_export_header(kmime)

target_link_libraries(kmime KF5::ItemViews KF5::Codecs KF5::KDE4Support)

set_target_properties(kmime PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kmime EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
         boolflags.h
         ${KMime_BINARY_DIR}/kmime_export.h
         kmime_charfreq.h
         kmime_codecs.h
         kmime_content.h
         kmime_contentindex.h
         kmime_header_parsing.h
         kmime_headers.h
         kmime_message.h
         kmime_mdn.h
         kmime_newsarticle.h
         kmime_dateformatter.h
         kmime_util.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/kmime COMPONENT Devel)
