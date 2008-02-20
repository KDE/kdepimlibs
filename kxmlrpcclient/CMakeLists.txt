project(kxmlrpcclient)
add_subdirectory( tests )

########### next target ###############

set(kxmlrpcclient_LIB_SRCS
   client.cpp
   query.cpp)


kde4_add_library(kxmlrpcclient SHARED ${kxmlrpcclient_LIB_SRCS})

target_link_libraries(kxmlrpcclient ${KDE4_KIO_LIBS})

set_target_properties(kxmlrpcclient PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kxmlrpcclient ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES kxmlrpcclient_export.h client.h 
         DESTINATION ${INCLUDE_INSTALL_DIR}/kxmlrpcclient COMPONENT Devel)
