configure_file(config-ktnef-tests.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktnef-tests.h)
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro(add_ktnef_test _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME ktnef-${_name}  ${_test})
  target_link_libraries(${_name} ktnef ${QT_QTTEST_LIBRARY})
endmacro(add_ktnef_test)

add_ktnef_test(parsertest.cpp)

