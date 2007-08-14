include_directories( ${GPGME_INCLUDES} ${CMAKE_SOURCE_DIR}/gpgme++/
	)

add_subdirectory( tests ) 




########### next target ###############

set(qgpgme_LIB_SRCS dataprovider.cpp eventloopinteractor.cpp )

#if(WIN32)
#  set(qgpgme_LIB_SRCS ${qgpgme_LIB_SRCS} eventloopnotify_win.cpp)
#else(WIN32)
  set(qgpgme_LIB_SRCS ${qgpgme_LIB_SRCS} eventloopnotify.cpp)
#endif(WIN32)

kde4_add_library(qgpgme SHARED ${qgpgme_LIB_SRCS})

target_link_libraries(qgpgme  ${QT_QTCORE_LIBRARY}  gpgmepp )

set_target_properties(qgpgme PROPERTIES
   VERSION   1.0.0
   SOVERSION 1.0.0
 )
install(TARGETS qgpgme  DESTINATION ${LIB_INSTALL_DIR})

install(FILES qgpgme_export.h eventloopinteractor.h dataprovider.h DESTINATION ${INCLUDE_INSTALL_DIR}/qgpgme)
