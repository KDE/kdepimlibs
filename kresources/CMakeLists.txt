project(kresources)

add_subdirectory(tests)

include_directories(${KDE4_KIO_INCLUDES})

add_definitions(${QDBUS_DEFINITIONS} -DKDE_DEFAULT_DEBUG_AREA=5650)
add_definitions(
  -DKRESOURCES_DEPRECATED=
)
# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DKRESOURCES_DEPRECATED_EXPORT=KRESOURCES_EXPORT
)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

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

qt4_add_dbus_adaptor(kresources_LIB_SRCS
  org.kde.KResourcesManager.xml managerimpl.h KRES::ManagerImpl)

add_library(kresources ${LIBRARY_TYPE} ${kresources_LIB_SRCS})
generate_export_header(kresources)

target_link_libraries(kresources LINK_PRIVATE ${KDE4_KIO_LIBS})

set_target_properties(kresources PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kresources EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(kcm_kresources_PART_SRCS kcmkresources.cpp)

kde4_add_plugin(kcm_kresources ${kcm_kresources_PART_SRCS})

target_link_libraries(kcm_kresources ${KDE4_KDEUI_LIBS} kresources)

install(TARGETS kcm_kresources DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kresources.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES
  kresources_manager.desktop
  kresources_plugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
)

install(FILES
  configdialog.h
  configpage.h
  configwidget.h
  factory.h
  idmapper.h
  ${CMAKE_CURRENT_BINARY_DIR}/kresources_export.h
  kcmkresources_export.h
  manager.h
  managerimpl.h
  resource.h
  selectdialog.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kresources COMPONENT Devel
)

install(FILES org.kde.KResourcesManager.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
