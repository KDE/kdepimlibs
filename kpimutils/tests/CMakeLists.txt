
if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/..   )

########### next target ###############

set(testemail_SRCS testemail.cpp )

kde4_automoc(${testemail_SRCS})

kde4_add_executable(testemail NOGUI RUN_UNINSTALLED ${testemail_SRCS})

target_link_libraries(testemail ${KDE4_KDECORE_LIBS} kpimutils ${QT_QTTEST_LIBRARY} )

add_test(kpimutils-testemail ${EXECUTABLE_OUTPUT_PATH}/testemail)

########### next target ###############

set(testlinklocator_SRCS testlinklocator.cpp )

kde4_automoc(${testlinklocator_SRCS})

kde4_add_executable(testlinklocator NOGUI RUN_UNINSTALLED ${testlinklocator_SRCS})

target_link_libraries(testlinklocator ${KDE4_KDECORE_LIBS} kpimutils ${QT_QTTEST_LIBRARY})

add_test(kpimutils-testlinklocator ${EXECUTABLE_OUTPUT_PATH}/testlinklocator)

endif(KDE4_BUILD_TESTS)
