include(ECMMarkAsTest)

find_package(Qt5Test CONFIG REQUIRED)



add_executable(image-provider-tests image-provider-tests image-provider-tests.cpp)
add_test(image-provider-tests image-provider-tests)
ecm_mark_as_test(image-provider-tests)
target_link_libraries(image-provider-tests KF5::AkonadiSocialUtils Qt5::Test)
