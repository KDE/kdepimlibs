
include_directories( ${SASL2_INCLUDE_DIR} )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(kio_imap4_PART_SRCS 
   imapcommand.cpp 
   imaplist.cpp
   mailaddress.cpp
   mimeheader.cpp
   imap4.cpp
   imapinfo.cpp
   imapparser.cpp 
   mailheader.cpp
   mimehdrline.cpp
   mimeio.cpp )


kde4_add_plugin(kio_imap4 ${kio_imap4_PART_SRCS})

target_link_libraries(kio_imap4 ${KDE4_KDE3SUPPORT_LIBS} ${SASL2_LIBRARIES} kmime kimap)

install(TARGETS kio_imap4  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES imap4.protocol imaps.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
