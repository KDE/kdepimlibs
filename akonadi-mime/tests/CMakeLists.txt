set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED COMPONENTS Test Widgets)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_BINARY_DIR}/akonadi/kmime)

# TODO apparently QTEST_AKONADIMAIN doesn't like NO_CASTs...
remove_definitions( -DQT_NO_CAST_FROM_ASCII )
remove_definitions( -DQT_NO_CAST_TO_ASCII )

set( requester_srcs foldersrequester.cpp )
add_executable( requester "${requester_srcs}" )
target_link_libraries( requester KF5::AkonadiMime )
# for racetest
set(requester_exe_string "QLatin1String( \\\"${CMAKE_CURRENT_BINARY_DIR}/requester\\\" )" )
add_definitions( -DREQUESTER_EXE="${requester_exe_string}" )

# convenience macro to add akonadi demo application
macro(add_akonadimime_demo _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  add_executable(${_name} ${_test})
  target_link_libraries(${_name}  KF5AkonadiMime KF5::Mime)
endmacro()

# demo applications
add_akonadimime_demo(headfetcher.cpp)
