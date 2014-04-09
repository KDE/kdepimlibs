
# FIXME Delete this block when akonadi-kmime is split
# If building the monolithic kdepimlibs, kcfg2dbus.xsl is in akonadi/
if(NOT "${CMAKE_SOURCE_DIR}" STREQUAL "${Akonadi-KMime_SOURCE_DIR}")
  set(KF5Akonadi_DATA_DIR ${Akonadi_SOURCE_DIR})
endif()

include_directories(${QJSON_INCLUDE_DIR})

#REACTIVATE IT
#add_subdirectory(serializer)

set(akonadi_social_utils_SRCS
  socialnetworkattributes.cpp
  imageprovider.cpp
  socialfeeditem.cpp
)

set(akonadi_social_utils_HDRS
  ${CMAKE_CURRENT_BINARY_DIR}/libakonadisocialutils_export.h
  socialnetworkattributes.h
  imageprovider.h
  socialfeeditem.h
)
add_library(KF5AkonadiSocialUtils ${akonadi_social_utils_SRCS})

generate_export_header(KF5AkonadiSocialUtils BASE_NAME akonadi-socialutils)

add_library(KF5::AkonadiSocialUtils ALIAS KF5AkonadiSocialUtils)

target_include_directories(KF5AkonadiSocialUtils INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/AkonadiSocialUtils;${Boost_INCLUDE_DIR}>")

target_link_libraries(KF5AkonadiSocialUtils
    KF5::AkonadiCore
    KF5::AkonadiWidgets
    KF5::KIOCore
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
    EXPORT KF5AkonadiSocialUtilsTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES ${akonadi_social_utils_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/akonadi/socialutils)

