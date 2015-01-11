set( SHARED_MIME_INFO_MINIMUM_VERSION "0.40" )
find_package( SharedMimeInfo REQUIRED )

if(WIN32)
  set(
    LIB_INSTALL_DIR ${KDE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${KDE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${KDE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${KDE_INSTALL_LIBDIR}
  )
endif()

########### next target ###############

set(akonadi_serializer_socialfeeditem_SRCS
  akonadi_serializer_socialfeeditem.cpp
)

add_library(akonadi_serializer_socialfeeditem MODULE ${akonadi_serializer_socialfeeditem_SRCS})
target_link_libraries(akonadi_serializer_socialfeeditem
  KF5::AkonadiSocialUtils)

install(TARGETS akonadi_serializer_socialfeeditem DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install(FILES
  akonadi_serializer_socialfeeditem.desktop
  DESTINATION ${KDE_INSTALL_DATADIR}/akonadi/plugins/serializer)

install(FILES x-vnd.akonadi.socialfeeditem.xml DESTINATION ${KDE_INSTALL_MIMEDIR})

update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
