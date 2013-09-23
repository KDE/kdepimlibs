set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(
  ${CMAKE_SOURCE_DIR}/kmbox
  ${CMAKE_BINARY_DIR}/kmbox
  ${CMAKE_BINARY_DIR}/kmime
)

########### next target ###############

set(testmbox_SRCS mboxtest.cpp )

kde4_add_unit_test(testmbox TESTNAME mbox-testmbox ${testmbox_SRCS})
kde4_add_unit_test(mboxbenchmark TESTNAME mbox-benchmark mboxbenchmark.cpp)

target_link_libraries(testmbox ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} kmime kmbox)
target_link_libraries(mboxbenchmark ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} kmime kmbox)
