# Turn exceptions on
kde_enable_exceptions()

# FIXME Delete this block when akonadi-kmime is split
# If building the monolithic kdepimlibs, kcfg2dbus.xsl is in akonadi/
if(NOT "${CMAKE_SOURCE_DIR}" STREQUAL "${Akonadi-KMime_SOURCE_DIR}")
  set(KF5Akonadi_DATA_DIR ${Akonadi_SOURCE_DIR})
endif()


# generates a D-Bus interface description from a KConfigXT file
macro(kcfg_generate_dbus_interface _kcfg _name)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_name}.xml
    COMMAND ${LIBXSLT_XSLTPROC_EXECUTABLE} --stringparam interfaceName ${_name}
    ${KF5Akonadi_DATA_DIR}/kcfg2dbus.xsl
    ${_kcfg}
    > ${CMAKE_CURRENT_BINARY_DIR}/${_name}.xml
    ${_kcfg}
    )
endmacro()

macro(add_resource_iface _kcfgFile _ifaceName _className)
  kcfg_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${_ifaceName})
  string(TOLOWER ${_className} _codeFile)
  set_source_files_properties( ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "metatype.h")
  qt5_add_dbus_interface(akonadikmime_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
  )
endmacro()

########### next target ###############

set(akonadikmime_SRCS
    addressattribute.cpp
    attributeregistrar.cpp
    removeduplicatesjob.cpp
    specialmailcollections.cpp
    specialmailcollectionsrequestjob.cpp
    specialmailcollectionsdiscoveryjob.cpp
    messagefolderattribute.cpp
    messagemodel.cpp
    messageparts.cpp
    messageflags.cpp
    messagestatus.cpp
    messagethreadingattribute.cpp
    messagethreaderproxymodel.cpp

    commandbase.cpp
    util.cpp
    emptytrashcommand.cpp
    markascommand.cpp
    movecommand.cpp
    movetotrashcommand.cpp
    standardmailactionmanager.cpp
)

add_resource_iface(imapresource.kcfg
    org.kde.Akonadi.Imap.Settings ImapSettings)

if (KDE4_BUILD_TESTS)
    set(akonadikmime_SRCS
        ${akonadikmime_SRCS}
        specialmailcollectionstesting.cpp
    )
endif()

KCONFIG_ADD_KCFG_FILES(akonadikmime_SRCS
    specialmailcollectionssettings.kcfgc
)

install(FILES
    specialmailcollections.kcfg
    DESTINATION ${KCFG_INSTALL_DIR}
)

add_library(KF5AkonadiKMime ${akonadikmime_SRCS})

generate_export_header(KF5AkonadiKMime BASE_NAME akonadi-kmime)

add_library(KF5::AkonadiKMime ALIAS KF5AkonadiKMime)

target_include_directories(KF5AkonadiKMime INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/Akonadi/KMime;${Boost_INCLUDE_DIR}>")
target_include_directories(KF5AkonadiKMime PUBLIC "$<BUILD_INTERFACE:${Akonadi-KMime_SOURCE_DIR}/src;${Akonadi-KMime_BINARY_DIR}/src;${Boost_INCLUDE_DIR}>")


target_link_libraries(KF5AkonadiKMime
    KF5::KMime
    KF5::AkonadiCore
    KF5::AkonadiWidgets
    KF5::KIOCore
)

set_target_properties(KF5AkonadiKMime PROPERTIES
    VERSION ${AKONADIKMIME_VERSION_STRING}
    SOVERSION ${AKONADIKMIME_SOVERSION}
    EXPORT_NAME AkonadiKMime
)

ecm_generate_pri_file(BASE_NAME AkonadiKMime
    LIB_NAME KF5AkonadiKMime
    DEPS "KMime AkonadiCore AkonadiWidgets KIO" FILENAME_VAR PRI_FILENAME
)

install(TARGETS
    KF5AkonadiKMime
    EXPORT KF5AkonadiKMimeTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
)


ecm_generate_headers(AkonadiKMime_CamelCase_HEADERS
  HEADER_NAMES

  AddressAttribute
  MessageFlags
  MessageFolderAttribute
  MessageModel
  MessageParts
  MessageStatus
  MessageThreaderProxyModel
  MessageThreadingAttribute
  RemoveDuplicatesJob
  SpecialMailCollections
  SpecialMailCollectionsDiscoveryJob
  SpecialMailCollectionsRequestJob
  REQUIRED_HEADERS AkonadiKMime_HEADERS
  PREFIX Akonadi/KMime
)

install(FILES
    ${AkonadiKMime_CamelCase_HEADERS}
    DESTINATION ${INCLUDE_INSTALL_DIR}/Akonadi/KMime
    COMPONENT Devel
)

install(FILES
  ${AkonadiKMime_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/akonadi-kmime_export.h
  standardmailactionmanager.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/akonadi/kmime COMPONENT Devel
)


