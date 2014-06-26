set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include(ECMAddTests)
set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)


configure_file(config-ktnef-tests.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktnef-tests.h)

ecm_add_tests( parsertest.cpp NAME_PREFIX "ktnetf-" LINK_LIBRARIES KF5Tnef Qt5::Test)
