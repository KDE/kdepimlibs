add_definitions( -DMAIL_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/../data/mails\\"" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

macro(add_kmime_test _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME kmime-${_name} ${_test})
  target_link_libraries(${_name} kmime ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} )
endmacro(add_kmime_test)

add_kmime_test(rfc2047test.cpp)
add_kmime_test(kmime_util_test.cpp)
add_kmime_test(contentindextest.cpp)
add_kmime_test(kmime_charfreq_test.cpp)
add_kmime_test(headertest.cpp)
add_kmime_test(kmime_content_test.cpp)
add_kmime_test(kmime_message_test.cpp)
#add_kmime_test(kmime_headerfactorytest.cpp)

