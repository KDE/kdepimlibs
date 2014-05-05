
set(kio_ldap_PART_SRCS kio_ldap.cpp)


add_library(kio_ldap MODULE ${kio_ldap_PART_SRCS})

target_link_libraries(kio_ldap KF5::KIOCore KF5::KDELibs4Support KF5::Ldap KF5::Mbox)

install(TARGETS kio_ldap  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES ldap.protocol ldaps.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
