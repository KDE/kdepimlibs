include(ECMMarkAsTest)
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

set(notestest_SRCS notestest.cpp)
add_executable(notestest ${notestest_SRCS})
add_test(notestest notestest)
ecm_mark_as_test(notestest)
target_link_libraries(notestest KF5AkonadiNotes KF5::Mime Qt5::Test)

set(CMAKE_PREFIX_PATH ../)
