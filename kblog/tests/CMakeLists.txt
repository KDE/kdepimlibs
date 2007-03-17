
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_KIO_INCLUDES} )

########### next target ###############

set(testblogger_SRCS testblogger.cpp )

kde4_automoc(${testblogger_SRCS})

kde4_add_executable(testblogger RUN_UNINSTALLED ${testblogger_SRCS})

target_link_libraries(testblogger ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY}
kblog)

add_test(kblog-testblogger ${EXECUTABLE_OUTPUT_PATH}/testblogger)

########### next target ###############

set(testmetaweblog_SRCS testmetaweblog.cpp )

kde4_automoc(${testblogger_SRCS})

kde4_add_executable(testmetaweblog RUN_UNINSTALLED ${testmetaweblog_SRCS})

target_link_libraries(testmetaweblog ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY}
kblog)

add_test(kblog-testmetaweblog ${EXECUTABLE_OUTPUT_PATH}/testmetaweblog)


endif(KDE4_BUILD_TESTS)

