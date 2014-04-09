set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMMarkAsTest)
set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

########### next target ###############

set(testemail_SRCS testemail.cpp)

add_executable(testemail ${testemail_SRCS})
add_test(testemail testemail)
ecm_mark_as_test(testemail)
target_link_libraries(testemail KF5PimUtils Qt5::Test)

########### next target ###############

set(testlinklocator_SRCS testlinklocator.cpp)

add_executable(testlinklocator ${testlinklocator_SRCS})
add_test(testlinklocator testlinklocator)
ecm_mark_as_test(testlinklocator)
target_link_libraries(testlinklocator KF5PimUtils Qt5::Test)
