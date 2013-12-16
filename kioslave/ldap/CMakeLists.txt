set(kio_ldap_PART_SRCS kio_ldap.cpp)


include_directories(
  ${CMAKE_BINARY_DIR}/kldap
)

kde4_add_plugin(kio_ldap ${kio_ldap_PART_SRCS})

target_link_libraries(kio_ldap KF5::KIOCore KF5::KDE4Support kldap)

install(TARGETS kio_ldap  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES ldap.protocol ldaps.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
