kde_enable_exceptions()

include(ECMMarkAsTest)

find_package(Qt5Test CONFIG REQUIRED)

########### next target ###############

set(mboxtest_SRCS mboxtest.cpp)

add_executable(mboxtest ${mboxtest_SRCS})
add_test(mboxtest mboxtest)
ecm_mark_as_test(mboxtest)
target_link_libraries(mboxtest KF5Mbox Qt5::Test)

########### next target ###############

set(mboxbenchmark_SRCS mboxbenchmark.cpp)

add_executable(mboxbenchmark ${mboxbenchmark_SRCS})
add_test(mboxbenchmark mboxbenchmark)
ecm_mark_as_test(mboxbenchmark)
target_link_libraries(mboxbenchmark KF5Mbox Qt5::Test)
