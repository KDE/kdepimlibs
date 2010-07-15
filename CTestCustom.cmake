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
  "qlist.h.*increases required alignment of target type"
  "qmap.h.*increases required alignment of target type"
  "qhash.h.*increases required alignment of target type"
  )

# Errors that will be ignored
set(CTEST_CUSTOM_ERROR_EXCEPTION
  ${CTEST_CUSTOM_ERROR_EXCEPTION}
  "ICECC"
  "Segmentation fault"
  "GConf Error"
  "Client failed to connect to the D-BUS daemon"
  "Failed to connect to socket"
  )

# No coverage for these files
set(CTEST_CUSTOM_COVERAGE_EXCLUDE ".moc$" "moc_" "ui_")
