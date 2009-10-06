set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_KIO_INCLUDES} )

########### next target ###############

set(testclient_SRCS testclient.cpp )


kde4_add_unit_test(testclient TESTNAME kxmlrpc-testclient  ${testclient_SRCS})

target_link_libraries(testclient  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} kxmlrpcclient)

########## next target ##############

set(testquery_SRCS testquery.cpp )


kde4_add_unit_test(testquery TESTNAME kxmlrpc-testquery  ${testquery_SRCS})

target_link_libraries(testquery  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} kxmlrpcclient)

