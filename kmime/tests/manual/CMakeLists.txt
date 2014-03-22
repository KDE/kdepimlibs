if(HAVE_GETOPT_H)
    set(test_kmime_header_parsing_SRCS test_kmime_header_parsing.cpp )

    add_executable(test_kmime_header_parsing TEST ${test_kmime_header_parsing_SRCS})

    target_link_libraries(test_kmime_header_parsing  kmime ${KDE4_KDECORE_LIBS} )
endif()

########### next target ###############

set(test_charfreq_SRCS test_charfreq.cpp )

add_executable(test_charfreq TEST ${test_charfreq_SRCS})

target_link_libraries(test_charfreq  kmime ${KDE4_KDECORE_LIBS} )

########### next target ###############

if(HAVE_GETOPT_H)
    set(test_mdn_SRCS test_mdn.cpp )

    add_executable(test_mdn TEST ${test_mdn_SRCS})

    target_link_libraries(test_mdn  kmime ${KDE4_KDECORE_LIBS} )
endif()

########### next target ###############

set(test_dates_SRCS test_dates.cpp )

add_executable(test_dates TEST ${test_dates_SRCS})

target_link_libraries(test_dates  kmime ${KDE4_KDECORE_LIBS} )
