set(akonadiabc_LIB_SRC
  contactparts.cpp
)

add_library(KF5AkonadiAbc ${akonadiabc_LIB_SRC})

generate_export_header(KF5AkonadiAbc BASE_NAME akonadi-abc)

add_library(KF5::AkonadiAbc ALIAS KF5AkonadiAbc)

# NOTE: include subdirs are 'akonadi/kabc' to remain as SC as possible.
target_include_directories(KF5AkonadiAbc INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/Akonadi/KAbc>")
target_include_directories(KF5AkonadiAbc PUBLIC "$<BUILD_INTERFACE:${Akonadi-Abc_SOURCE_DIR}/src;${Akonadi-Abc_BINARY_DIR}/src>")

target_link_libraries(KF5AkonadiAbc
PUBLIC
    Qt5::Core
)

set_target_properties(KF5AkonadiAbc PROPERTIES
    VERSION ${AKONADIABC_VERSION_STRING}
    SOVERSION ${AKONADIABC_SOVERSION}
    EXPORT_NAME AkonadiAbc
)

install(TARGETS
    KF5AkonadiAbc
    EXPORT KF5AkonadiAbcTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(AkonadiAbc_CamelCase_HEADERS
  HEADER_NAMES

  ContactParts
  REQUIRED_HEADERS AkonadiAbc_HEADERS
  PREFIX Akonadi/KAbc
)

install(FILES
    ${AkonadiAbc_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/Akonadi/KAbc
    COMPONENT Devel
)

install(FILES
    ${AkonadiAbc_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/akonadi-abc_export.h
    ${AkonadiAbc_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/akonadi/kabc
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME AkonadiAbc LIB_NAME KF5AkonadiAbc DEPS "" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/Akonadi/kabc)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

