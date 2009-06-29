add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5324 )

include_directories(
    ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_subdirectory( tests )

set( outboxinterface_lib_srcs
     dispatcherinterface.cpp
     messagequeuejob.cpp
     outboxactions.cpp

     dispatchmodeattribute.cpp
     errorattribute.cpp
     sentbehaviourattribute.cpp
     transportattribute.cpp
     attributeregistrar.cpp
)

kde4_add_library( outboxinterface SHARED ${outboxinterface_lib_srcs} )
target_link_libraries( outboxinterface ${KDE4_KIO_LIBS} akonadi-kde akonadi-kmime kmime mailtransport )
set_target_properties( outboxinterface PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install( TARGETS outboxinterface ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES 
         outboxinterface_export.h

         dispatcherinterface.h
         messagequeuejob.h
         outboxactions.h

         dispatchmodeattribute.h
         errorattribute.h
         sentbehaviourattribute.h
         transportattribute.h

         DESTINATION ${INCLUDE_INSTALL_DIR}/outboxinterface COMPONENT Devel )

