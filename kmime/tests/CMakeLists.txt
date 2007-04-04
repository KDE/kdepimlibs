
if(KDE4_BUILD_TESTS)

include_directories(${CMAKE_SOURCE_DIR}/kmime)

# convenience macro to add libkmime qtestlib unit-tests
macro(add_kmime_test _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_automoc(${_test})
  kde4_add_executable(${_name} NOGUI RUN_UNINSTALLED ${_test})
  target_link_libraries(${_name} kmime ${QT_QTTEST_LIBRARY})
  add_test(kmime-${_name} ${EXECUTABLE_OUTPUT_PATH}/${_name})
endmacro(add_kmime_test)


########### next target ###############

if(HAVE_GETOPT_H)
    set(test_kmime_header_parsing_SRCS test_kmime_header_parsing.cpp )

    kde4_automoc(${test_kmime_header_parsing_SRCS})

    kde4_add_executable(test_kmime_header_parsing ${test_kmime_header_parsing_SRCS})

    target_link_libraries(test_kmime_header_parsing  kmime ${KDE4_KDECORE_LIBS} )
endif(HAVE_GETOPT_H)

########### next target ###############

set(test_charfreq_SRCS test_charfreq.cpp )

kde4_automoc(${test_charfreq_SRCS})

kde4_add_executable(test_charfreq ${test_charfreq_SRCS})

target_link_libraries(test_charfreq  kmime ${KDE4_KDECORE_LIBS} )

########### next target ###############

if(HAVE_GETOPT_H)
    set(test_mdn_SRCS test_mdn.cpp )

    kde4_automoc(${test_mdn_SRCS})

    kde4_add_executable(test_mdn ${test_mdn_SRCS})

    target_link_libraries(test_mdn  kmime ${KDE4_KDECORE_LIBS} )
endif(HAVE_GETOPT_H)

########### next target ###############

set(test_dates_SRCS test_dates.cpp )

kde4_automoc(${test_dates_SRCS})

kde4_add_executable(test_dates ${test_dates_SRCS})

target_link_libraries(test_dates  kmime ${KDE4_KDECORE_LIBS} )

########### next target ###############
if(HAVE_GETOPT_H)
    set(test_kmime_codec_SRCS test_kmime_codec.cpp )

    kde4_automoc(${test_kmime_codec_SRCS})
        
    kde4_add_executable(test_kmime_codec ${test_kmime_codec_SRCS})
        
    target_link_libraries(test_kmime_codec  kmime ${KDE4_KDECORE_LIBS} )
endif(HAVE_GETOPT_H)

# qtestlib unit tests
add_kmime_test(rfc2047test.cpp)
add_kmime_test(kmime_util_test.cpp)
add_kmime_test(contentindextest.cpp)
add_kmime_test(kmime_content_test.cpp)
add_kmime_test(headertest.cpp)
add_kmime_test(kmime_message_test.cpp)

endif(KDE4_BUILD_TESTS)
