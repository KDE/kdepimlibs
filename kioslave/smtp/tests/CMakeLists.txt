set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(test_responseparser_SRCS test_responseparser.cpp )


kde4_add_unit_test(test_responseparser TESTNAME smtp-responseparser ${test_responseparser_SRCS})

target_link_libraries(test_responseparser  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(test_headergeneration_SRCS test_headergeneration.cpp )


kde4_add_unit_test(test_headergeneration TESTNAME smtp-headergeneration ${test_headergeneration_SRCS})

target_link_libraries(test_headergeneration  ${KDE4_KDECORE_LIBS} )


########### next target ###############
set(test_commands_SRCS test_commands.cpp )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${SASL2_INCLUDE_DIR})
kde4_add_unit_test(test_commands TESTNAME smtp-commands ${test_commands_SRCS})

target_link_libraries(test_commands  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${SASL2_LIBRARIES})


########### next target ###############
set(interactivesmtpserver_SRCS interactivesmtpserver.cpp )


kde4_add_executable(interactivesmtpserver TEST ${interactivesmtpserver_SRCS})

target_link_libraries(interactivesmtpserver  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} )


########### next target ###############
set( test_request_source requesttest.cpp ../request.cpp)
kde4_add_unit_test( requesttest ${test_request_source})
target_link_libraries( requesttest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

