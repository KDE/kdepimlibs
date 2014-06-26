
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

#REACTIVATE
#if (KDE4_BUILD_TESTS)
#    set(akonadikmime_SRCS
#        ${akonadikmime_SRCS}
#        specialmailcollectionstesting.cpp
#    )
#endif()

KCONFIG_ADD_KCFG_FILES(akonadikmime_SRCS
    specialmailcollectionssettings.kcfgc
)

install(FILES
    specialmailcollections.kcfg
    DESTINATION ${KCFG_INSTALL_DIR}
)

add_library(KF5AkonadiMime ${akonadikmime_SRCS})

generate_export_header(KF5AkonadiMime BASE_NAME akonadi-kmime)

add_library( KF5::AkonadiMime ALIAS KF5AkonadiMime)

target_include_directories(KF5AkonadiMime INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/Akonadi/KMime;${Boost_INCLUDE_DIR}>")
target_include_directories(KF5AkonadiMime PUBLIC "$<BUILD_INTERFACE:${Akonadi-KMime_SOURCE_DIR}/src;${Akonadi-KMime_BINARY_DIR}/src;${Boost_INCLUDE_DIR}>")
# TODO: Remove when kdepimlibs is split (and fix the broken includes in eg. mailtransport)
target_include_directories(KF5AkonadiMime INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/akonadi/kmime>")

target_link_libraries(KF5AkonadiMime
PUBLIC
    KF5::AkonadiCore
PRIVATE
    KF5::Mime
    KF5::AkonadiWidgets
    KF5::KIOCore
)

set_target_properties(KF5AkonadiMime PROPERTIES
    VERSION ${AKONADIKMIME_VERSION_STRING}
    SOVERSION ${AKONADIKMIME_SOVERSION}
    EXPORT_NAME AkonadiMime
)

ecm_generate_pri_file(BASE_NAME AkonadiMime
    LIB_NAME KF5AkonadiMime
    DEPS "AkonadiCore" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/Akonadi/KMime
)

install(TARGETS
    KF5AkonadiMime
    EXPORT KF5AkonadiMimeTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)


ecm_generate_headers(AkonadiMime_CamelCase_HEADERS
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
  StandardMailActionManager
  REQUIRED_HEADERS AkonadiMime_HEADERS
  PREFIX Akonadi/KMime
)

install(FILES
    ${AkonadiMime_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Akonadi/KMime
    COMPONENT Devel
)

install(FILES
    ${AkonadiMime_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/akonadi-kmime_export.h
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/akonadi/kmime
    COMPONENT Devel
)

