set( SHARED_MIME_INFO_MINIMUM_VERSION "0.40" )
find_package( SharedMimeInfo REQUIRED )

if(WIN32)
  set(
    LIB_INSTALL_DIR ${LIB_INSTALL_DIR}
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
  )
endif()

include_directories(
  ${CMAKE_BINARY_DIR}/akonadi/socialutils
)

########### next target ###############

set(akonadi_serializer_socialfeeditem_SRCS
  akonadi_serializer_socialfeeditem.cpp
)

add_library(akonadi_serializer_socialfeeditem MODULE ${akonadi_serializer_socialfeeditem_SRCS})
target_link_libraries(akonadi_serializer_socialfeeditem
  KF5::AkonadiSocialUtils)

install(TARGETS akonadi_serializer_socialfeeditem DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES
  akonadi_serializer_socialfeeditem.desktop
  DESTINATION ${KF5_DATA_INSTALL_DIR}/akonadi/plugins/serializer)

install(FILES x-vnd.akonadi.socialfeeditem.xml DESTINATION ${XDG_MIME_INSTALL_DIR})

update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
