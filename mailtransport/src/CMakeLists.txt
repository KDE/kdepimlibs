# Turn exceptions on
kde_enable_exceptions()

option(MAILTRANSPORT_INPROCESS_SMTP "false" "Use in-process SMTP instead of KIO slaves (note that this changes the mailtransport license from LGPL to GPL!)")

add_subdirectory(kconf_update)

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
add_definitions(-DUSES_DEPRECATED_MAILTRANSPORT_API) # for transportmanager
add_definitions(
  -DMAILTRANSPORT_DEPRECATED=
)
# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DMAILTRANSPORT_DEPRECATED_EXPORT=MAILTRANSPORT_EXPORT
)


if(MAILTRANSPORT_INPROCESS_SMTP)
  add_definitions("-DMAILTRANSPORT_INPROCESS_SMTP")
  include_directories(${Sasl2_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/kioslave)
  set(mailtransport_lib_srcs
    ../kioslave/smtp/command.cpp
    ../kioslave/smtp/capabilities.cpp
    ../kioslave/smtp/request.cpp
    ../kioslave/smtp/response.cpp
    ../kioslave/smtp/transactionstate.cpp
    ../kioslave/smtp/smtpsessioninterface.cpp
    smtp/smtpsession.cpp
  )
endif()

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


target_include_directories(KF5MailTransport INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/MailTransport>")
target_include_directories(KF5MailTransport PUBLIC "$<BUILD_INTERFACE:${MailTransport_SOURCE_DIR}/src;${MailTransport_BINARY_DIR}/src>")


target_link_libraries(KF5MailTransport
                      KF5::KIOCore
                      KF5::ItemViews
                      KF5::KDELibs4Support
                      KF5::AkonadiCore
                      KF5::AkonadiWidgets
                      KF5::AkonadiMime
                      KF5::Mime
                      KF5::Wallet
)

set_target_properties(KF5MailTransport PROPERTIES
    VERSION ${MAILTRANSPORT_VERSION_STRING}
    SOVERSION ${MAILTRANSPORT_SOVERSION}
    EXPORT_NAME MailTransport
)


if(MAILTRANSPORT_INPROCESS_SMTP)
  target_link_libraries(mailtransport ${Sasl2_LIBRARIES} KF5::PimUtils)
endif()


install(TARGETS KF5MailTransport EXPORT KF5MailTransportTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES mailtransport.kcfg DESTINATION ${KCFG_INSTALL_DIR})

#Reactivate it
#add_subdirectory(kcm)


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
  REQUIRED_HEADERS MailTransport_HEADERS
)

install(FILES
  ${MailTransport_CamelCase_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/mailtransport_export.h
  ${MailTransport_HEADERS}
  transportjob.h
  sendmailjob.h
  smtpjob.h
  precommandjob.h
  socket.h

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

  dispatchmodeattribute.h
  errorattribute.h
  sentactionattribute.h
  sentbehaviourattribute.h
  transportattribute.h

  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/MailTransport COMPONENT Devel
)

