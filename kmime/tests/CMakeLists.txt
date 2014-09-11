include(CheckIncludeFile)
check_include_file(getopt.h HAVE_GETOPT_H)

if(HAVE_GETOPT_H)

########### next target ###############

  add_executable(test_kmime_header_parsing test_kmime_header_parsing.cpp)
  target_link_libraries(test_kmime_header_parsing KF5Mime)

########### next target ###############

  add_executable(test_mdn test_mdn.cpp)
  add_test(test_mdn test_mdn)
  target_link_libraries(test_mdn KF5Mime)

endif()

########### next target ###############

add_executable(test_charfreq test_charfreq.cpp)
add_test(test_charfreq test_charfreq)
target_link_libraries(test_charfreq KF5Mime)

########### next target ###############

add_executable(test_dates test_dates.cpp)
add_test(test_dates test_dates)
target_link_libraries(test_dates KF5Mime)
