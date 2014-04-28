kde_enable_exceptions()

include(ECMMarkAsTest)
set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)


configure_file(config-ktnef-tests.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktnef-tests.h)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${CMAKE_SOURCE_DIR}/ktnef
  ${CMAKE_BINARY_DIR}/ktnef
)

macro(add_ktnef_test _source)
  get_filename_component(_name ${_source} NAME_WE)
  add_executable(${_name} ${_name}.cpp)
  add_test(${_name} ${_name})
  ecm_mark_as_test(${_name})
  target_link_libraries(${_name}
    KF5Tnef
    Qt5::Test
  )
endmacro()

add_ktnef_test(parsertest.cpp)

