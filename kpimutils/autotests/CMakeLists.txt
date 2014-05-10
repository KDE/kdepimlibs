set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMAddTests)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

########### next target ###############

ecm_add_tests( testemail.cpp testlinklocator.cpp NAME_PREFIX "kpimutils-" LINK_LIBRARIES KF5PimUtils Qt5::Test)
