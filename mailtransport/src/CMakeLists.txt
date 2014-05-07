# Turn exceptions on
kde_enable_exceptions()

add_subdirectory(kconf_update)

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")


set(mailtransport_lib_srcs
  ${mailtransport_lib_srcs}
  transport.cpp
  transportmanager.cpp
  transporttype.cpp

  transportcombobox.cpp
  transportconfigwidget.cpp

  filteractionjob.cpp
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
  sentactionattribute.cpp
  sentbehaviourattribute.cpp
  transportattribute.cpp
  transportconfigdialog.cpp
  sendmailconfigwidget.cpp
  smtpconfigwidget.cpp

  transportlistview.cpp
  transportmanagementwidget.cpp
  addtransportdialog.cpp
)

qt5_wrap_ui(mailtransport_lib_srcs
  ui/sendmailsettings.ui
  ui/addtransportdialog.ui
  ui/transportmanagementwidget.ui
)
if(KDEPIM_MOBILE_UI)
  qt5_wrap_ui(mailtransport_lib_srcs ui/smtpsettings_mobile.ui)
else()
  qt5_wrap_ui(mailtransport_lib_srcs ui/smtpsettings_desktop.ui)
endif()

kconfig_add_kcfg_files(mailtransport_lib_srcs transportbase.kcfgc)

add_library(KF5MailTransport ${mailtransport_lib_srcs})

generate_export_header(KF5MailTransport BASE_NAME mailtransport)

add_library(KF5::MailTransport ALIAS KF5MailTransport)


target_include_directories(KF5MailTransport INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/MailTransport;${KF5_INCLUDE_INSTALL_DIR}/MailTransport/mailtransport>")
target_include_directories(KF5MailTransport PUBLIC "$<BUILD_INTERFACE:${MailTransport_SOURCE_DIR}/src;${MailTransport_BINARY_DIR}/src>")


target_link_libraries(KF5MailTransport
PUBLIC
                      KF5::Wallet
                      KF5::KDELibs4Support
                      KF5::AkonadiCore
                      KF5::Mime
                      KF5::AkonadiMime
PRIVATE
                      KF5::KIOCore
                      KF5::AkonadiWidgets
)

set_target_properties(KF5MailTransport PROPERTIES
    VERSION ${MAILTRANSPORT_VERSION_STRING}
    SOVERSION ${MAILTRANSPORT_SOVERSION}
    EXPORT_NAME MailTransport
)


if(MAILTRANSPORT_INPROCESS_SMTP)
  target_link_libraries(mailtransport ${Sasl2_LIBRARIES} KF5::PimUtils)
endif()


install(TARGETS KF5MailTransport EXPORT KF5MailTransportTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES mailtransport.kcfg DESTINATION ${KCFG_INSTALL_DIR})

add_subdirectory(kcm)


ecm_generate_headers(MailTransport_CamelCase_HEADERS
  HEADER_NAMES
  DispatcherInterface
  DispatchModeAttribute
  ErrorAttribute
  MessageQueueJob
  PrecommandJob
  SendmailJob
  SentBehaviourAttribute
  ServerTest
  SmtpJob
  Socket
  Transport
  TransportAttribute
  #TransportBase
  TransportComboBox
  TransportConfigDialog
  TransportJob
  TransportManagementWidget
  TransportManager
  TransportType
  PREFIX MailTransport
  REQUIRED_HEADERS MailTransport_HEADERS
)

install(FILES
  ${MailTransport_CamelCase_HEADERS}
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/MailTransport/MailTransport/ COMPONENT Devel ) 

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/mailtransport_export.h
  ${MailTransport_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/transportbase.h
  sentactionattribute.h

  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/MailTransport/mailtransport COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME MailTransport LIB_NAME KF5MailTransport DEPS "Wallet KDELibs4Support AkonadiCore Mime AkonadiMime" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/MailTransport/)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

