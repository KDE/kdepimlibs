
set( _any_gpgme_found false )

if ( WIN32 AND GPGME_QT_FOUND )

   # on Windows, we require gpgme-qt

   set( _any_gpgme_found true )
   set( _qgpgme_libgpgme gpgmepp-qt )
   set( _qgpgme_extra_src eventloopinteractor_win.cpp )

endif ( WIN32 AND GPGME_QT_FOUND )

if ( NOT WIN32 AND GPGME_PTHREAD_FOUND )

   # on Unix, we requrie gpgme-pthread (for convenience, since vanilla
   # gpgme isn't thread-aware, and we don't want to restrict clients
   # of qgpgme to non-threaded useage of gpgme)

   set( _any_gpgme_found true )
   set( _qgpgme_libgpgme gpgmepp-pthread )
   set( _qgpgme_extra_src eventloopinteractor_unix.cpp )

endif ( NOT WIN32 AND GPGME_PTHREAD_FOUND )

# build / install only if any useable gpgme was found:

if ( _any_gpgme_found )

   include_directories( ${GPGME_INCLUDES} ${CMAKE_SOURCE_DIR}/gpgme++ ${Boost_INCLUDE_DIR} )

   add_subdirectory( tests )

   set( qgpgme_LIB_SRCS
        dataprovider.cpp
        eventloopinteractor.cpp
        ${_qgpgme_extra_src}
   )

   kde4_add_library( qgpgme SHARED ${qgpgme_LIB_SRCS} )

   target_link_libraries( qgpgme ${QT_QTCORE_LIBRARY} ${_qgpgme_libgpgme} )

   set_target_properties( qgpgme PROPERTIES
     VERSION   1.0.0
     SOVERSION 1.0.0
   )

   install( TARGETS qgpgme  
     RUNTIME DESTINATION ${BIN_INSTALL_DIR}
     LIBRARY DESTINATION ${LIB_INSTALL_DIR}
     ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
   )

   install( FILES qgpgme_export.h eventloopinteractor.h dataprovider.h DESTINATION ${INCLUDE_INSTALL_DIR}/qgpgme )

endif ( _any_gpgme_found )
