kde_enable_exceptions()

include(ECMAddTests)
find_package(Qt5Test CONFIG REQUIRED)

########### next target ###############

ecm_add_tests(mboxtest.cpp mboxbenchmark.cpp NAME_PREFIX "mbox-" LINK_LIBRARIES KF5Mbox Qt5::Test)
