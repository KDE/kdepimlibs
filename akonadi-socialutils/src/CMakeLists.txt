
# FIXME Delete this block when akonadi-kmime is split
# If building the monolithic kdepimlibs, kcfg2dbus.xsl is in akonadi/
if(NOT "${CMAKE_SOURCE_DIR}" STREQUAL "${Akonadi-SocialUtils_SOURCE_DIR}")
  set(KF5Akonadi_DATA_DIR ${Akonadi_SOURCE_DIR})
endif()

#REACTIVATE IT
#add_subdirectory(serializer)

set(akonadi_social_utils_SRCS
  socialnetworkattributes.cpp
  imageprovider.cpp
  socialfeeditem.cpp
)

add_library(KF5AkonadiSocialUtils ${akonadi_social_utils_SRCS})

generate_export_header(KF5AkonadiSocialUtils BASE_NAME akonadi-socialutils)

add_library(KF5::AkonadiSocialUtils ALIAS KF5AkonadiSocialUtils)


target_include_directories(KF5AkonadiSocialUtils INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/Akonadi/socialutils${Boost_INCLUDE_DIR}>")
target_include_directories(KF5AkonadiSocialUtils PUBLIC "$<BUILD_INTERFACE:${Akonadi-SocialUtils_SOURCE_DIR}/src;${Akonadi-SocialUtils_BINARY_DIR}/src;${Boost_INCLUDE_DIR}>")

target_link_libraries(KF5AkonadiSocialUtils
PUBLIC
    KF5::AkonadiCore
    KF5::KIOCore
    KF5::CoreAddons
)

set_target_properties(KF5AkonadiSocialUtils PROPERTIES
    VERSION ${AKONADISOCIALUTILS_VERSION_STRING}
    SOVERSION ${AKONADISOCIALUTILS_SOVERSION}
    EXPORT_NAME AkonadiSocialUtils
)

ecm_generate_pri_file(BASE_NAME AkonadiSocialUtils
    LIB_NAME KF5AkonadiSocialUtils
    DEPS "AkonadiCore AkonadiWidgets KIO" FILENAME_VAR PRI_FILENAME
)

install(TARGETS
    KF5AkonadiSocialUtils
    EXPORT KF5AkonadiSocialUtilsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(AkonadiSocialUtils_CamelCase_HEADERS
  HEADER_NAMES
  ImageProvider
  SocialFeedItem
  SocialNetworkAttributes
  REQUIRED_HEADERS AkonadiSocialUtils_HEADERS
)

install(FILES
   ${AkonadiSocialUtils_CamelCase_HEADERS}
   DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Akonadi/SocialUtils
   COMPONENT Devel)

install(FILES
   ${AkonadiSocialUtils_HEADERS}
   ${CMAKE_CURRENT_BINARY_DIR}/akonadi-socialutils_export.h
   DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/akonadi/socialutils
   COMPONENT Devel)

