

add_subdirectory(tests)

set(smtp_optional_includes)
set(smtp_optional_libs)

if (Sasl2_FOUND)
   set(smtp_optional_includes ${smtp_optional_includes} ${Sasl2_INCLUDE_DIR})
   set(smtp_optional_libs     ${smtp_optional_libs}     ${Sasl2_LIBRARIES})
endif()


include_directories(  ${smtp_optional_includes}  )


########### next target ###############

set(kio_smtp_PART_SRCS
   smtp.cpp
   request.cpp
   response.cpp
   capabilities.cpp
   command.cpp
   transactionstate.cpp
   smtpsessioninterface.cpp
   kioslavesession.cpp
   smtp_debug.cpp
)


add_library(kio_smtp MODULE ${kio_smtp_PART_SRCS})


target_link_libraries(kio_smtp  KF5::KIOCore KF5::I18n Qt5::Network ${smtp_optional_libs})

install(TARGETS kio_smtp  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES smtp.protocol smtps.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )

