include_directories(${KMime_SOURCE_DIR})
include_directories(${KMime_BINARY_DIR})

remove_definitions(-DQT_USE_QSTRINGBUILDER)

add_subdirectory(manual)
add_subdirectory(auto)
