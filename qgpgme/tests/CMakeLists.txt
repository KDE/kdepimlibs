set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${GPGME_INCLUDES} )


########### next target ###############

set(dataprovidertest_SRCS dataprovidertest.cpp )
kde4_add_executable(dataprovidertest TEST ${dataprovidertest_SRCS})
target_link_libraries(dataprovidertest  qgpgme ${QT_QTCORE_LIBRARY} )

########### next target ###############

set(installtest_SRCS installtest.cpp)
kde4_add_executable(installtest TEST ${installtest_SRCS})
target_link_libraries(installtest  qgpgme)
