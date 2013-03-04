set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
 ${CMAKE_SOURCE_DIR}/kcal
 ${LIBICAL_INCLUDE_DIRS}
)


MACRO(KCAL_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp)
    target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${LIBICAL_LIBRARIES})
  ENDFOREACH(_testname)
ENDMACRO(KCAL_UNIT_TESTS)

MACRO(KCAL_EXECUTABLE_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
    target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kcal ${QT_QTTEST_LIBRARY})
  ENDFOREACH(_testname)
ENDMACRO(KCAL_EXECUTABLE_TESTS)

KCAL_UNIT_TESTS(
  testalarm
  testassignmentvisitor
  testattachment
  testattendee
  testcalendarlocal
  testcalendarnull
#  testcalendarresources  disable for now because it cause akonadi to start
  testcalfilter
  testcomparisonvisitor
  testcustomproperties
  testduration
  testerrorformat
  testevent
  testfilestorage
  testfreebusy
  testincidencerelation
  testjournal
  testkresult
  testperiod
  testperson
  testsortablelist
  testtodo
  testtimesininterval
)

set_target_properties(testcalendarlocal PROPERTIES COMPILE_FLAGS -DICALTESTDATADIR="\\"${CMAKE_SOURCE_DIR}/kcal/tests/data/\\"" )
# this test cannot work with msvc because libical should not be altered
# and therefore we can't add KCAL_EXPORT_DEPRECATED there
# it should work fine with mingw because of the auto-import feature
if(NOT MSVC)
  KCAL_UNIT_TESTS(testicaltimezones)
endif()

KCAL_EXECUTABLE_TESTS(
  convertqtopia
  incidencestest
  loadcalendar
  fbrecurring
  readandwrite
  testfb
  testrecurprevious
  testrecurrence
  testrecurrencetype
  testrecurson
  testtostring
  testvcalexport
)
if (NOT KDEPIM_NO_KRESOURCES)
  KCAL_EXECUTABLE_TESTS(
    testresource
  )
endif()

########### next target ###############
set(testfields_SRCS testfields.cpp)
set(srcfile "${CMAKE_SOURCE_DIR}/kcal/tests/data/test_pilot.ics")
add_definitions( -D_TESTINPUT="\\"${srcfile}\\"" )

kde4_add_executable(testfields TEST ${testfields_SRCS})

target_link_libraries(testfields  ${KDE4_KDECORE_LIBS} kcal )


########### Tests #######################


FILE( GLOB_RECURSE testFiles       data/RecurrenceRule/*.ics )
FILE( GLOB_RECURSE compatFiles     data/Compat/*.ics )
FILE( GLOB_RECURSE vCalFilesAsIcal data/vCalendar/*.ics )
FILE( GLOB_RECURSE vCalFiles       data/vCalendar/*.vcs )

if (WIN32)
  find_program(PERL_EXECUTABLE perl)
endif()

macro(kcal_run_single_test _prefix _file _test _testarg)
  get_filename_component( _fn ${_file} NAME)

  # Write the output file in the build directory.
  string(REGEX REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" _outFile ${_file})
  get_filename_component( _outputPath ${_outFile} PATH)
  file(MAKE_DIRECTORY ${_outputPath})

  set(_runner ${CMAKE_CURRENT_SOURCE_DIR}/runsingletestcase.pl)
  if (UNIX)
    set(_test_path ${EXECUTABLE_OUTPUT_PATH}/${_test}.shell)
  endif()
  if (WIN32)
    set(_test_path ${EXECUTABLE_OUTPUT_PATH}/${_test}.exe.bat)
    set(_runner ${PERL_EXECUTABLE} ${_runner})
  endif()

  add_test( ${_prefix}-${_fn} ${_runner} ${_test_path} ${_testarg} ${_file} ${_outFile})
endmacro(kcal_run_single_test)

FILE(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/FAILED.log)

FOREACH( file ${testFiles} )
  kcal_run_single_test( RecurNext ${file} testrecurrence "next" )
ENDFOREACH(file)

FOREACH( file ${testFiles} )
  kcal_run_single_test( RecurPrev ${file} testrecurprevious "prev" )
ENDFOREACH(file)

FOREACH( file ${testFiles} )
  kcal_run_single_test( RecursOn ${file} testrecurson "recurson" )
ENDFOREACH(file)

FOREACH( file ${compatFiles} )
  kcal_run_single_test( Compat ${file} readandwrite "ical" )
ENDFOREACH(file)

FOREACH( file ${vCalFilesAsIcal} )
  kcal_run_single_test( VCalOut ${file} testvcalexport "vcal" )
ENDFOREACH(file)

FOREACH( file ${vCalFiles} )
  kcal_run_single_test( VCalIn ${file} readandwrite "ical" )
ENDFOREACH(file)

