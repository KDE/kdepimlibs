



########### next target ###############

set(kio_nntp_PART_SRCS nntp.cpp )


add_library(kio_nntp MODULE ${kio_nntp_PART_SRCS})


target_link_libraries(kio_nntp  KF5::KIOCore KF5::I18n)
set_target_properties(kio_nntp PROPERTIES OUTPUT_NAME "nntp")

install(TARGETS kio_nntp  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES nntp.protocol nntps.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )

