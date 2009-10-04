if ( WIN32 AND GPGME_VANILLA_FOUND )
   set( _qgpgme_libgpgme gpgmepp )
endif ( WIN32 AND GPGME_VANILLA_FOUND )

if ( NOT WIN32 AND GPGME_PTHREAD_FOUND )
   set( _qgpgme_libgpgme gpgmepp-pthread )
endif ( NOT WIN32 AND GPGME_PTHREAD_FOUND )

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${GPGME_INCLUDES} )
include_directories( ${Boost_INCLUDE_DIR} )
link_directories( ${_gpgme_library_dirs} )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############

set( qgpgme_TESTS
  dataprovidertest
  installtest
  # PORT ME AWAY FROM EVENT LOOPS
  #gpgsetexpirytimetest
  #gpgsetownertrusttest
  #gpgsignkeytest
  #gpgadduseridtest
  )

foreach( _qgpgme_test ${qgpgme_TESTS} )
  kde4_add_executable( "${_qgpgme_test}" TEST "${_qgpgme_test}.cpp" )
  if (WIN32) # what's up?
    target_link_libraries( "${_qgpgme_test}" qgpgme ${_qgpgme_libgpgme} ${_gpg_error_library} ${QT_QTCORE_LIBRARY} )
  else(WIN32)
    target_link_libraries( "${_qgpgme_test}" qgpgme ${_qgpgme_libgpgme} gpgme gpg-error ${QT_QTCORE_LIBRARY} )
  endif(WIN32)
endforeach( _qgpgme_test )

########### next target ###############

# PORT ME AWAY FROM EVENT LOOPS
#kde4_add_executable(editinteractortest TEST editinteractortest.cpp)
#target_link_libraries(editinteractortest  qgpgme ${QT_QTGUI_LIBRARY})
