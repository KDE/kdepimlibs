
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/..   )

########### next target ###############

set(testemail_SRCS testemail.cpp )

kde4_automoc(${testemail_SRCS})

kde4_add_unit_test(testemail TESTNAME kpimutils-testemail  ${testemail_SRCS})

target_link_libraries(testemail ${KDE4_KDECORE_LIBS} kpimutils ${QT_QTTEST_LIBRARY} )

########### next target ###############

set(testlinklocator_SRCS testlinklocator.cpp )

kde4_automoc(${testlinklocator_SRCS})

kde4_add_unit_test(testlinklocator TESTNAME kpimutils-testlinklocator  ${testlinklocator_SRCS})

target_link_libraries(testlinklocator ${KDE4_KDECORE_LIBS} kpimutils ${QT_QTTEST_LIBRARY})
