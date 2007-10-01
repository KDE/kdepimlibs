set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
 ${CMAKE_SOURCE_DIR}/kcal
 ${CMAKE_BINARY_DIR}/kcal/libical/src
)

MACRO(KCAL_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})
  ENDFOREACH(_testname)
ENDMACRO(KCAL_UNIT_TESTS)

MACRO(KCAL_EXECUTABLE_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_executable(${_testname} TEST ${_testname}.cpp)
    target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})
  ENDFOREACH(_testname)
ENDMACRO(KCAL_EXECUTABLE_TESTS)

KCAL_UNIT_TESTS(
  testalarm
  testattachment
  testattendee
  testcalendarlocal
  testcalendarnull
  testcalendarresources
  testcalfilter
  testduration
  testerrorformat
  testevent
  testfilestorage
  testfreebusy
  testjournal
  testperiod
  testperson
  testsortablelist
  testtodo
)
# this test cannot work with msvc because libical should not be altered
# and therefore we can't add KCAL_EXPORT there
# it should work fine with mingw because of the auto-import feature
if(NOT MSVC)
  KCAL_UNIT_TESTS(testicaltimezones)
endif(NOT MSVC)

KCAL_EXECUTABLE_TESTS(
  convertqtopia
  incidencestest
  loadcalendar
  fbrecurring
  readandwrite
  testrecurprevious
  testrecurrence
  testrecurrencetype
  testrecurson
  testresource
  testtostring
  testvcalexport
)

########### next target ###############
set(testfields_SRCS testfields.cpp)
set(srcfile "${CMAKE_SOURCE_DIR}/kcal/tests/data/test_pilot.ics")
set_source_files_properties( ${testfields_SRCS} PROPERTIES 
	COMPILE_FLAGS -DINPUT="\\"${srcfile}\\"" )

kde4_add_executable(testfields TEST ${testfields_SRCS})

target_link_libraries(testfields  ${KDE4_KDECORE_LIBS} kcal )


########### Tests #######################


FILE( GLOB_RECURSE testFiles       data/RecurrenceRule/*.ics )
FILE( GLOB_RECURSE compatFiles     data/Compat/*.ics )
FILE( GLOB_RECURSE vCalFilesAsIcal data/vCalendar/*.ics )
FILE( GLOB_RECURSE vCalFiles       data/vCalendar/*.vcs )

FOREACH( file ${testFiles} )
  GET_FILENAME_COMPONENT( fn ${file} NAME)
  ADD_TEST( RecurNext-${fn} ${CMAKE_CURRENT_SOURCE_DIR}/runsingletestcase.pl ${EXECUTABLE_OUTPUT_PATH}/testrecurrence "next" ${file} )
ENDFOREACH(file)

FOREACH( file ${testFiles} )
  GET_FILENAME_COMPONENT( fn ${file} NAME)
  ADD_TEST( RecurPrev-${fn} ${CMAKE_CURRENT_SOURCE_DIR}/runsingletestcase.pl ${EXECUTABLE_OUTPUT_PATH}/testrecurprevious "prev" ${file} )
ENDFOREACH(file)

FOREACH( file ${testFiles} )
  GET_FILENAME_COMPONENT( fn ${file} NAME)
  ADD_TEST( RecursOn-${fn} ${CMAKE_CURRENT_SOURCE_DIR}/runsingletestcase.pl ${EXECUTABLE_OUTPUT_PATH}/testrecurson "recurson" ${file} )
ENDFOREACH(file)

FOREACH( file ${compatFiles} )
  GET_FILENAME_COMPONENT( fn ${file} NAME)
  ADD_TEST( Compat-${fn} ${CMAKE_CURRENT_SOURCE_DIR}/runsingletestcase.pl ${EXECUTABLE_OUTPUT_PATH}/readandwrite "ical" ${file} )
ENDFOREACH(file)

FOREACH( file ${vCalFilesAsIcal} )
  GET_FILENAME_COMPONENT( fn ${file} NAME)
  ADD_TEST( VCalOut-${fn} ${CMAKE_CURRENT_SOURCE_DIR}/runsingletestcase.pl ${EXECUTABLE_OUTPUT_PATH}/testvcalexport "vcal" ${file} )
ENDFOREACH(file)

FOREACH( file ${vCalFiles} )
  GET_FILENAME_COMPONENT( fn ${file} NAME)
  ADD_TEST( VCalIn-${fn} ${CMAKE_CURRENT_SOURCE_DIR}/runsingletestcase.pl ${EXECUTABLE_OUTPUT_PATH}/readandwrite "ical" ${file} )
ENDFOREACH(file)

# ADD_TEST( TestRecurrence  runtestcase.pl testrecurrence    "next" ${CMAKE_CURRENT_SOURCE_DIR} data/RecurrenceRule/ "*.ics" )
# ADD_TEST( TestRecPrevious runtestcase.pl testrecurprevious "prev" ${CMAKE_CURRENT_SOURCE_DIR} data/RecurrenceRule/ "*.ics" )
# ADD_TEST( TestRecursOn    runtestcase.pl testrecurson  "recurson" ${CMAKE_CURRENT_SOURCE_DIR} data/RecurrenceRule/ "*.ics" )
# ADD_TEST( TestCompat      runtestcase.pl readandwrite      "ical" ${CMAKE_CURRENT_SOURCE_DIR} data/Compat/         "*.ics" )
# ADD_TEST( TestVCal        runtestcase.pl testvcalexport    "vcal" ${CMAKE_CURRENT_SOURCE_DIR} data/vCalendar/      "*.ics" )
# ADD_TEST( TestFromVCal    runtestcase.pl readandwrite      "ical" ${CMAKE_CURRENT_SOURCE_DIR} data/vCalendar/      "*.vcs" )

