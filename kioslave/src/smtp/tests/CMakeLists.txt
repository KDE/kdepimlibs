include(ECMMarkAsTest)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(test_responseparser_SRCS test_responseparser.cpp )

add_executable( test_responseparser ${test_responseparser_SRCS} )
add_test( test_responseparser test_responseparser )
ecm_mark_as_test(smtp-responseparser)
target_link_libraries(test_responseparser   Qt5::Test KF5::I18n KF5::KIOCore)

########### next target ###############

set(test_headergeneration_SRCS test_headergeneration.cpp ../smtp_debug.cpp )

add_executable( test_headergeneration  ${test_headergeneration_SRCS} )
add_test( test_headergeneration test_headergeneration )
ecm_mark_as_test(smtp-headergeneration)

target_link_libraries(test_headergeneration  Qt5::Test )


########### next target ###############
set(test_commands_SRCS test_commands.cpp ../smtp_debug.cpp )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${Sasl2_INCLUDE_DIR}  ${kioslave_SOURCE_DIR}/src/)

add_executable( test_commands ${test_commands_SRCS} )
add_test( test_commands test_commands )
ecm_mark_as_test(smtp-commands)
target_link_libraries(test_commands   KF5::KIOCore ${Sasl2_LIBRARIES} Qt5::Test KF5::I18n)


########### next target ###############
set(interactivesmtpserver_SRCS interactivesmtpserver.cpp )

add_executable( interactivesmtpserver ${interactivesmtpserver_SRCS} )
ecm_mark_as_test(smtp-interactivesmtpserver)
target_link_libraries(interactivesmtpserver    Qt5::Test Qt5::Widgets Qt5::Network)


########### next target ###############
set(test_capabilities_SRCS test_capabilities.cpp ../capabilities.cpp )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${kioslave_SOURCE_DIR}/src/smtp)

add_executable( test_capabilities ${test_capabilities_SRCS} )
ecm_mark_as_test(test-capabilities)
target_link_libraries(test_capabilities   KF5::KIOCore)


