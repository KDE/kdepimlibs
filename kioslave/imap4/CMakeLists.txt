
set(imap4_optional_includes)
set(imap4_optional_libs)

if (SASL2_FOUND)
   set(imap4_optional_includes ${imap4_optional_includes} ${SASL2_INCLUDE_DIR})
   set(imap4_optional_libs     ${imap4_optional_libs}     ${SASL2_LIBRARIES})
   set(HAVE_LIBSASL2 1)
else (SASL2_FOUND)
   set(HAVE_LIBSASL2 0)
endif (SASL2_FOUND)

configure_file(imap4-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/imap4-config.h)

include_directories(${imap4_optional_includes})

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
   mimeio.cpp)

kde4_add_plugin(kio_imap4 ${kio_imap4_PART_SRCS})

target_link_libraries(kio_imap4 kmime kimap ${KDE4_KIO_LIBS})
if (SASL2_FOUND)
	target_link_libraries(kio_imap4 ${SASL2_LIBRARIES} )
endif(SASL2_FOUND)

install(TARGETS kio_imap4  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES imap4.protocol imaps.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
