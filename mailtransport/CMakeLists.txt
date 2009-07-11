set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(
    ${Boost_INCLUDE_DIR}
)

add_subdirectory( kconf_update )
add_subdirectory( tests )

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5324 )
add_definitions( -DUSES_DEPRECATED_MAILTRANSPORT_API ) # for transportmanager

set(mailtransport_lib_srcs
  transport.cpp
  transportmanager.cpp
  transporttype.cpp

  transportcombobox.cpp
  transportlistview.cpp
  transportmanagementwidget.cpp

  addtransportdialog.cpp
  transportconfigdialog.cpp
  transportconfigwidget.cpp
  sendmailconfigwidget.cpp
  smtpconfigwidget.cpp

  transportjob.cpp
  resourcesendjob.cpp
  sendmailjob.cpp
  smtpjob.cpp
  precommandjob.cpp

  legacydecrypt.cpp
  socket.cpp
  servertest.cpp

  dispatcherinterface.cpp
  messagequeuejob.cpp
  outboxactions.cpp

  attributeregistrar.cpp
  dispatchmodeattribute.cpp
  errorattribute.cpp
  sentbehaviourattribute.cpp
  transportattribute.cpp
)


kde4_add_ui_files(mailtransport_lib_srcs
  sendmailsettings.ui
  smtpsettings.ui

  addtransportdialog.ui
  transportmanagementwidget.ui
)
kde4_add_kcfg_files(mailtransport_lib_srcs transportbase.kcfgc)

kde4_add_library(mailtransport SHARED ${mailtransport_lib_srcs})
target_link_libraries(mailtransport ${KDE4_KIO_LIBS} akonadi-kde akonadi-kmime kmime )
set_target_properties(mailtransport PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS mailtransport EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mailtransport.kcfg  DESTINATION ${KCFG_INSTALL_DIR})

set(mailtransport_kcm_srcs configmodule.cpp)
kde4_add_plugin(kcm_mailtransport ${mailtransport_kcm_srcs})
target_link_libraries(kcm_mailtransport  ${KDE4_KDEUI_LIBS} mailtransport)

install(TARGETS kcm_mailtransport  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_mailtransport.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES 
        mailtransport_export.h

        transportjob.h
        resourcesendjob.h
        sendmailjob.h
        smtpjob.h

        ${CMAKE_CURRENT_BINARY_DIR}/transportbase.h
        transport.h
        transportmanager.h
        transporttype.h
        servertest.h

        transportcombobox.h
        transportconfigdialog.h
        transportmanagementwidget.h

        dispatcherinterface.h
        messagequeuejob.h
        outboxactions.h

        dispatchmodeattribute.h
        errorattribute.h
        sentbehaviourattribute.h
        transportattribute.h

        DESTINATION ${INCLUDE_INSTALL_DIR}/mailtransport COMPONENT Devel)

