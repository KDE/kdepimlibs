set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${GPGME_INCLUDES} )

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )

########### next target ###############

set(dataprovidertest_SRCS dataprovidertest.cpp )
kde4_add_executable(dataprovidertest TEST ${dataprovidertest_SRCS})
target_link_libraries(dataprovidertest  qgpgme ${QT_QTCORE_LIBRARY} )

########### next target ###############

set(installtest_SRCS installtest.cpp)
kde4_add_executable(installtest TEST ${installtest_SRCS})
target_link_libraries(installtest  qgpgme)

########### next target ###############

kde4_add_executable(gpgsetexpirytimetest TEST gpgsetexpirytimetest.cpp)
target_link_libraries(gpgsetexpirytimetest  qgpgme ${QT_QTCORE_LIBRARY})

########### next target ###############

kde4_add_executable(gpgsetownertrusttest TEST gpgsetownertrusttest.cpp)
target_link_libraries(gpgsetownertrusttest  qgpgme ${QT_QTCORE_LIBRARY})

########### next target ###############

kde4_add_executable(editinteractortest TEST editinteractortest.cpp)
target_link_libraries(editinteractortest  qgpgme ${QT_QTGUI_LIBRARY})
