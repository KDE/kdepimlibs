include(ECMAddTests)

find_package(Qt5Test CONFIG REQUIRED)

########### next target ###############

ecm_add_tests( testblogcomment.cpp testblogger1.cpp testgdata.cpp testmetaweblog.cpp testmovabletype.cpp testwordpressbuggy.cpp testblogpost.cpp testblogmedia.cpp NAME_PREFIX "kblog-" LINK_LIBRARIES KF5Blog Qt5::Test)

# ########### next target ###############

#  set(testlivejournal_SRCS testlivejournal.cpp)
#
# add_executable(testlivejournal ${testlivejournal_SRCS})
# add_test(testlivejournal testlivejournal)
# ecm_mark_as_test(testlivejournal)
# target_link_libraries(testlivejournal KF5Blog Qt5::Test)
