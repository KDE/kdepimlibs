add_subdirectory(tests)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(krss_LIB_SRCS
   item.cpp
   enclosure.cpp
   person.cpp
   category.cpp
   rssitem.cpp
   feedpropertiescollectionattribute.cpp
   feedcollection.cpp
   helper.cpp
   feeditemmodel.cpp
   importfromopmljob.cpp
   exporttoopmljob.cpp
   opmlparser.cpp
)

set(krss_LIB_HDRS
   krss_export.h
   item.h
   enclosure.h
   person.h
   category.h
   feeditemmodel.h
   rssitem.h
)


kde4_add_library(krss ${LIBRARY_TYPE} ${krss_LIB_SRCS})

target_link_libraries(krss
                      akonadi-kde
                      syndication
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
)

set_target_properties(krss PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS krss EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES
   krss_export.h
   item.h
   enclosure.h
   feedcollection.h
   feedpropertiescollectionattribute.h
   person.h
   category.h
   feeditemmodel.h
   exporttoopmljob.h
   importfromopmljob.h
   rssitem.h  #TODO review if rssitem.h should be private
   DESTINATION ${INCLUDE_INSTALL_DIR}/krss COMPONENT Devel)
