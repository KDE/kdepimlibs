# Turn exceptions on
kde_enable_exceptions()

set(akonadinotes_LIB_SRC
  noteutils.cpp
)

add_library(KF5AkonadiNotes ${akonadinotes_LIB_SRC})

generate_export_header(KF5AkonadiNotes BASE_NAME akonadi-notes)

add_library(KF5::KF5AkonadiNotes ALIAS KF5AkonadiNotes)

target_include_directories(KF5AkonadiNotes INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/AkonadiNotes;${Boost_INCLUDE_DIR}>")
target_include_directories(KF5AkonadiNotes PUBLIC "$<BUILD_INTERFACE:${Akonadi-Notes_SOURCE_DIR}/src;${Akonadi-Notes_BINARY_DIR}/src;${Boost_INCLUDE_DIR}>")

target_link_libraries(KF5AkonadiNotes
    KF5::KMime
    KF5::AkonadiCore
    KF5::KIOCore
)

set_target_properties(KF5AkonadiNotes PROPERTIES
    VERSION ${AKONADINOTES_VERSION_STRING}
    SOVERSION ${AKONADINOTES_SOVERSION}
    EXPORT_NAME AkonadiNotes
)

install(TARGETS
    KF5AkonadiNotes
    EXPORT KF5AkonadiNotesTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(AkonadiNotes_CamelCase_HEADERS
  HEADER_NAMES

  NoteUtils
  REQUIRED_HEADERS AkonadiNotes_HEADERS
  PREFIX Akonadi/Notes
)

install(FILES
    ${AkonadiNotes_CamelCase_HEADERS}
    DESTINATION ${INCLUDE_INSTALL_DIR}/Akonadi/Notes
    COMPONENT Devel
)

install( FILES
  noteutils.h
  ${AkonadiNotes_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/akonadi-notes_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/akonadi/notes COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME AkonadiNotes LIB_NAME KF5AkonadiNotes DEPS "KMime AkonadiCore KIOCore" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

