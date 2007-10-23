
project(kresources)

add_subdirectory( tests )

include_directories( ${KDE4_KIO_INCLUDES} )

add_definitions(${QDBUS_DEFINITIONS})

########### next target ###############

set(kresources_LIB_SRCS
   resource.cpp
   factory.cpp
   managerimpl.cpp
   configwidget.cpp
   configdialog.cpp
   selectdialog.cpp
   configpage.cpp
   idmapper.cpp
   )

qt4_add_dbus_adaptor( kresources_LIB_SRCS org.kde.KResourcesManager.xml managerimpl.h KRES::ManagerImpl )


kde4_add_library(kresources SHARED ${kresources_LIB_SRCS})

target_link_libraries(kresources  ${KDE4_KIO_LIBS} )

set_target_properties(kresources PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kresources  
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

########### next target ###############

set(kcm_kresources_PART_SRCS kcmkresources.cpp )


kde4_add_plugin(kcm_kresources ${kcm_kresources_PART_SRCS})

target_link_libraries(kcm_kresources  ${KDE4_KDECORE_LIBS} kresources )

install(TARGETS kcm_kresources  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kresources.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kresources_plugin.desktop kresources_manager.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES
  kresources_export.h
  resource.h
  configwidget.h
  factory.h
  managerimpl.h
  manager.h
  selectdialog.h
  configdialog.h
  configpage.h
  idmapper.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kresources)
install( FILES org.kde.KResourcesManager.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )


