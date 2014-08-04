

set(pop3_optional_includes)
set(pop3_optional_libs)

if(Sasl2_FOUND)
   set(pop3_optional_includes ${pop3_optional_includes} ${Sasl2_INCLUDE_DIR})
   set(pop3_optional_libs     ${pop3_optional_libs}     ${Sasl2_LIBRARIES})
   set(HAVE_LIBSASL2 1)
else()
   set(HAVE_LIBSASL2 0)
endif()


configure_file(pop3-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/pop3-config.h )

include_directories(  ${pop3_optional_includes}  )

########### next target ###############

set(kio_pop3_PART_SRCS pop3.cpp pop3_debug.cpp )


add_library(kio_pop3 MODULE ${kio_pop3_PART_SRCS})


target_link_libraries(kio_pop3  KF5::KIOCore ${pop3_optional_libs} KF5::I18n )

install(TARGETS kio_pop3  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES pop3.protocol pop3s.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )



