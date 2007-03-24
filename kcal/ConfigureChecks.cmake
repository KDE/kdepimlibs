include(CheckIncludeFiles)
include(CheckSymbolExists)

check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(wctype.h HAVE_WCTYPE_H)
check_include_files(getopt.h HAVE_GETOPT_H)
check_symbol_exists(iswspace "wctype.h" HAVE_ISWSPACE)  # icalparser.c
