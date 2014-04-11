set(mailtransport_kcm_srcs configmodule.cpp)
kde4_add_plugin(kcm_mailtransport ${mailtransport_kcm_srcs})
  target_link_libraries(kcm_mailtransport
                        KF5::KCMUtils
                        KF5::ConfigWidgets
                        KF5::MailTransport
  )

install(TARGETS kcm_mailtransport DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_mailtransport.desktop DESTINATION ${SERVICES_INSTALL_DIR})

