

add_subdirectory(tests)

set(smtp_optional_includes)
set(smtp_optional_libs)

if (SASL2_FOUND)
   set(smtp_optional_includes ${smtp_optional_includes} ${SASL2_INCLUDE_DIR})
   set(smtp_optional_libs     ${smtp_optional_libs}     ${SASL2_LIBRARIES})
   set(HAVE_LIBSASL2 1)
else (SASL2_FOUND)
   set(HAVE_LIBSASL2 0)
endif (SASL2_FOUND)

configure_file(smtp-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/smtp-config.h )

include_directories(  ${smtp_optional_includes}  )


########### next target ###############

set(kio_smtp_PART_SRCS
   smtp.cpp
   request.cpp
   response.cpp
   capabilities.cpp
   command.cpp
   transactionstate.cpp )


kde4_add_plugin(kio_smtp ${kio_smtp_PART_SRCS})


target_link_libraries(kio_smtp  ${KDE4_KIO_LIBS} ${smtp_optional_libs})

install(TARGETS kio_smtp  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES smtp.protocol smtps.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )

