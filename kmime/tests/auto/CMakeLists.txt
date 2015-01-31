set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

# to find akonadi-kmime_export.h and akonadi_export.h
include_directories(${CMAKE_BINARY_DIR}/akonadi/kmime ${CMAKE_BINARY_DIR}/akonadi)

macro(add_kmime_test _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME kmime-${_name} ${_test})
  target_link_libraries(${_name} kmime ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} )
  set_target_properties(${_name} PROPERTIES COMPILE_FLAGS -DTEST_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/../data\\"" )
endmacro(add_kmime_test)

add_kmime_test(rfc2047test.cpp)
add_kmime_test(utiltest.cpp)
add_kmime_test(contentindextest.cpp)
add_kmime_test(charfreqtest.cpp)
add_kmime_test(headertest.cpp)
add_kmime_test(contenttest.cpp)
add_kmime_test(messagetest.cpp)
add_kmime_test(codectest.cpp)
#add_kmime_test(headerfactorytest.cpp)
add_kmime_test(rfc2231test.cpp)
add_kmime_test(base64benchmark.cpp)
add_kmime_test(sizetest.cpp)

target_link_libraries(messagetest akonadi-kde akonadi-kmime)
