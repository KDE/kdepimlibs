
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_KIO_INCLUDES} )

########### next target ###############

set(testkxmlrpc_SRCS testkxmlrpc.cpp )

kde4_automoc(${testkxmlrpc_SRCS})

kde4_add_executable(testkxmlrpc RUN_UNINSTALLED ${testkxmlrpc_SRCS})

target_link_libraries(testkxmlrpc  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kxmlrpcclient)

add_test(kxmlrpc-testkxmlrpc ${EXECUTABLE_OUTPUT_PATH}/testkxmlrpc)


endif(KDE4_BUILD_TESTS)

