set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${GPGME_INCLUDES} )
include_directories( ${Boost_INCLUDE_DIR} )
add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )

########### next target ###############

set( qgpgme_TESTS
  dataprovidertest
  installtest
  gpgsetexpirytimetest
  gpgsetownertrusttest
  gpgsignkeytest )

foreach( _qgpgme_test ${qgpgme_TESTS} )
  kde4_add_executable( "${_qgpgme_test}" TEST "${_qgpgme_test}.cpp" )
  target_link_libraries( "${_qgpgme_test}" qgpgme ${QT_QTCORE_LIBRARY} )
endforeach( _qgpgme_test )

########### next target ###############

kde4_add_executable(editinteractortest TEST editinteractortest.cpp)
target_link_libraries(editinteractortest  qgpgme ${QT_QTGUI_LIBRARY})
