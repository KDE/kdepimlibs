
set(kio_ldap_PART_SRCS kio_ldap.cpp kldap_debug.cpp)


add_library(kio_ldap MODULE ${kio_ldap_PART_SRCS})

target_link_libraries(kio_ldap KF5::KIOCore KF5::I18n KF5::Ldap KF5::Mbox)
set_target_properties(kio_ldap PROPERTIES OUTPUT_NAME "ldap")
install(TARGETS kio_ldap  DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kio)


########### install files ###############

install( FILES ldap.protocol ldaps.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
