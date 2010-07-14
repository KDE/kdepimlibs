# This file contains all the specific settings that will be used
# when running 'make Experimental'

# Change the maximum warnings that will be displayed
# on the report page (default 50)
set(CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS 1000)

# Warnings that will be ignored
set(CTEST_CUSTOM_WARNING_EXCEPTION
  ${CTEST_CUSTOM_WARNING_EXCEPTION}
  "kio/connection.h"
  "temporary since Dec 2000"
  "/kcal/versit/"
  "invoking macro kDebug argument 1"
  )

# Errors that will be ignored
set(CTEST_CUSTOM_ERROR_EXCEPTION
  ${CTEST_CUSTOM_ERROR_EXCEPTION}
  "ICECC"
  "Segmentation fault"
  "GConf Error: No D-BUS daemon running"
  )

# No coverage for these files
set(CTEST_CUSTOM_COVERAGE_EXCLUDE ".moc$" "moc_" "ui_")
