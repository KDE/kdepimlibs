include(ECMMarkAsTest)

find_package(Qt5Test CONFIG REQUIRED)

########### next target ###############

set(testblogger1_SRCS testblogger1.cpp)

add_executable(testblogger1 ${testblogger1_SRCS})
add_test(testblogger1 testblogger1)
ecm_mark_as_test(testblogger1)
target_link_libraries(testblogger1 KF5Blog Qt5::Test)

########### next target ###############

set(testgdata_SRCS testgdata.cpp)

add_executable(testgdata ${testgdata_SRCS})
add_test(testgdata testgdata)
ecm_mark_as_test(testgdata)
target_link_libraries(testgdata KF5Blog Qt5::Test)

########### next target ###############

set(testmetaweblog_SRCS testmetaweblog.cpp)

add_executable(testmetaweblog ${testmetaweblog_SRCS})
add_test(testmetaweblog testmetaweblog)
ecm_mark_as_test(testmetaweblog)
target_link_libraries(testmetaweblog KF5Blog Qt5::Test)

########### next target ###############

set(testmovabletype_SRCS testmovabletype.cpp)

add_executable(testmovabletype ${testmovabletype_SRCS})
add_test(testmovabletype testmovabletype)
ecm_mark_as_test(testmovabletype)
target_link_libraries(testmovabletype KF5Blog Qt5::Test)

########### next target ###############

set(testwordpressbuggy_SRCS testwordpressbuggy.cpp)

add_executable(testwordpressbuggy ${testwordpressbuggy_SRCS})
add_test(testwordpressbuggy testwordpressbuggy)
ecm_mark_as_test(testwordpressbuggy)
target_link_libraries(testwordpressbuggy KF5Blog Qt5::Test)

########### next target ###############

set(testblogpost_SRCS testblogpost.cpp)

add_executable(testblogpost ${testblogpost_SRCS})
add_test(testblogpost testblogpost)
ecm_mark_as_test(testblogpost)
target_link_libraries(testblogpost KF5Blog Qt5::Test)

########### next target ###############

set(testblogmedia_SRCS testblogmedia.cpp)

add_executable(testblogmedia ${testblogmedia_SRCS})
add_test(testblogmedia testblogmedia)
ecm_mark_as_test(testblogmedia)
target_link_libraries(testblogmedia KF5Blog Qt5::Test)

########### next target ###############

set(testblogcomment_SRCS testblogcomment.cpp)

add_executable(testblogcomment ${testblogcomment_SRCS})
add_test(testblogcomment testblogcomment)
ecm_mark_as_test(testblogcomment)
target_link_libraries(testblogcomment KF5Blog Qt5::Test)

# ########### next target ###############

#  set(testlivejournal_SRCS testlivejournal.cpp)
#
# add_executable(testlivejournal ${testlivejournal_SRCS})
# add_test(testlivejournal testlivejournal)
# ecm_mark_as_test(testlivejournal)
# target_link_libraries(testlivejournal KF5Blog Qt5::Test)
