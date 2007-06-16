
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_KIO_INCLUDES} )

########### next target ###############

set(testclient_SRCS testclient.cpp )

kde4_automoc(${testclient_SRCS})

kde4_add_executable(testclient NOGUI RUN_UNINSTALLED ${testclient_SRCS})

target_link_libraries(testclient  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kxmlrpcclient)

add_test(kxmlrpc-testclient ${EXECUTABLE_OUTPUT_PATH}/testclient)

########## next target ##############

set(testquery_SRCS testquery.cpp )

kde4_automoc(${testquery_SRCS})

kde4_add_executable(testquery NOGUI RUN_UNINSTALLED ${testquery_SRCS})

target_link_libraries(testquery  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kxmlrpcclient)

add_test(kxmlrpc-testquery ${EXECUTABLE_OUTPUT_PATH}/testquery)

endif(KDE4_BUILD_TESTS)

