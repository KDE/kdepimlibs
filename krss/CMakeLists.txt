
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(
  ${CMAKE_BINARY_DIR}/akonadi
  ${CMAKE_BINARY_DIR}/syndication
)

add_subdirectory(tests)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(krss_LIB_SRCS
   enclosure.cpp
   person.cpp
   category.cpp
   item.cpp
   feedpropertiescollectionattribute.cpp
   feedcollection.cpp
   helper.cpp
   feeditemmodel.cpp
   importfromopmljob.cpp
   exporttoopmljob.cpp
   opmlparser.cpp
)

kde4_add_library(krss ${LIBRARY_TYPE} ${krss_LIB_SRCS})

target_link_libraries(krss
                      akonadi-kde
                      syndication
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
)

generate_export_header(krss)

set_target_properties(krss PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS krss EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/krss_export.h
   enclosure.h
   feedcollection.h
   feedpropertiescollectionattribute.h
   person.h
   category.h
   feeditemmodel.h
   exporttoopmljob.h
   importfromopmljob.h
   item.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/krss COMPONENT Devel)
