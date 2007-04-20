
# Test apps should be in the bin dir so that add_test can locate them
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
	${CMAKE_SOURCE_DIR}/kcal
	${CMAKE_BINARY_DIR}/kcal/libical/src
	
)

########### next target ###############

if(KDE4_BUILD_TESTS)
set(loadcalendar_SRCS loadcalendar.cpp )

kde4_automoc(${loadcalendar_SRCS})


kde4_add_executable(loadcalendar ${loadcalendar_SRCS})

target_link_libraries(loadcalendar  ${KDE4_KDECORE_LIBS} kcal )


########### next target ###############

set(testtostring_SRCS testtostring.cpp )

kde4_automoc(${testtostring_SRCS})


kde4_add_executable(testtostring ${testtostring_SRCS})

target_link_libraries(testtostring  ${KDE4_KDECORE_LIBS} kcal )


########### next target ###############
set(incidencestest_SRCS incidencestest.cpp )

kde4_automoc(${incidencestest_SRCS})


kde4_add_executable(incidencestest ${incidencestest_SRCS})

target_link_libraries(incidencestest  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############
set(readandwrite_SRCS readandwrite.cpp )

kde4_automoc(${readandwrite_SRCS})


kde4_add_executable(readandwrite ${readandwrite_SRCS})

target_link_libraries(readandwrite  ${KDE4_KDECORE_LIBS} kcal )


########### next target ###############
set(testresource_SRCS testresource.cpp )

kde4_automoc(${testresource_SRCS})


kde4_add_executable(testresource ${testresource_SRCS})

target_link_libraries(testresource  ${KDE4_KDECORE_LIBS} kcal )


########### next target ###############
set(testfields_SRCS testfields.cpp )
set(srcfile "${CMAKE_SOURCE_DIR}/kcal/tests/data/test_pilot.ics")
set_source_files_properties( ${testfields_SRCS} PROPERTIES 
	COMPILE_FLAGS -DINPUT="\\"${srcfile}\\"" )
kde4_automoc(${testfields_SRCS})


kde4_add_executable(testfields ${testfields_SRCS})

target_link_libraries(testfields  ${KDE4_KDECORE_LIBS} kcal )


########### next target ###############
set(testrecurrence_SRCS testrecurrence.cpp )

kde4_automoc(${testrecurrence_SRCS})


kde4_add_executable(testrecurrence RUN_UNINSTALLED ${testrecurrence_SRCS})

target_link_libraries(testrecurrence  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############
set(testrecurprevious_SRCS testrecurprevious.cpp )

kde4_automoc(${testrecurprevious_SRCS})


kde4_add_executable(testrecurprevious RUN_UNINSTALLED ${testrecurprevious_SRCS})

target_link_libraries(testrecurprevious  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############
set(testrecurson_SRCS testrecurson.cpp )

kde4_automoc(${testrecurson_SRCS})


kde4_add_executable(testrecurson RUN_UNINSTALLED ${testrecurson_SRCS})

target_link_libraries(testrecurson  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############
set(testrecurrencetype_SRCS testrecurrencetype.cpp )

kde4_automoc(${testrecurrencetype_SRCS})


kde4_add_executable(testrecurrencetype RUN_UNINSTALLED ${testrecurrencetype_SRCS})

target_link_libraries(testrecurrencetype  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############

# this test cannot work with msvc because libical should not be altered
# and therefore we can't add KCAL_EXPORT there
# it should work fine with mingw because of the auto-import feature
if(NOT MSVC)
  set(testicaltimezones_SRCS testicaltimezones.cpp
 )

  kde4_automoc(${testicaltimezones_SRCS})

  kde4_add_executable(testicaltimezones NOGUI RUN_UNINSTALLED ${testicaltimezones_SRCS})

  target_link_libraries(testicaltimezones ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

  add_test(kcal-testicaltimezones ${EXECUTABLE_OUTPUT_PATH}/testicaltimezones)
endif(NOT MSVC)

########### next target ###############
set(testalarm_SRCS testalarm.cpp )

kde4_automoc(${testalarm_SRCS})

kde4_add_executable(testalarm NOGUI RUN_UNINSTALLED ${testalarm_SRCS})

target_link_libraries(testalarm ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testalarm ${EXECUTABLE_OUTPUT_PATH}/testalarm)

########### next target ###############
set(testattachment_SRCS testattachment.cpp )

kde4_automoc(${testattachment_SRCS})

kde4_add_executable(testattachment NOGUI RUN_UNINSTALLED ${testattachment_SRCS})

target_link_libraries(testattachment ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testattachment ${EXECUTABLE_OUTPUT_PATH}/testattachment)


########### next target ###############
set(testattendee_SRCS testattendee.cpp )

kde4_automoc(${testattendee_SRCS})

kde4_add_executable(testattendee NOGUI RUN_UNINSTALLED ${testattendee_SRCS})

target_link_libraries(testattendee ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testattendee ${EXECUTABLE_OUTPUT_PATH}/testattendee)

########### next target ###############
set(testcalendarlocal_SRCS testcalendarlocal.cpp )

kde4_automoc(${testcalendarlocal_SRCS})

kde4_add_executable(testcalendarlocal NOGUI RUN_UNINSTALLED ${testcalendarlocal_SRCS})

target_link_libraries(testcalendarlocal ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testcalendarlocal ${EXECUTABLE_OUTPUT_PATH}/testcalendarlocal)

########### next target ###############
set(testcalendarnull_SRCS testcalendarnull.cpp )

kde4_automoc(${testcalendarnull_SRCS})

kde4_add_executable(testcalendarnull NOGUI RUN_UNINSTALLED ${testcalendarnull_SRCS})

target_link_libraries(testcalendarnull ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testcalendarnull ${EXECUTABLE_OUTPUT_PATH}/testcalendarnull)

########### next target ###############
set(testcalfilter_SRCS testcalfilter.cpp )

kde4_automoc(${testcalfilter_SRCS})

kde4_add_executable(testcalfilter NOGUI RUN_UNINSTALLED ${testcalfilter_SRCS})

target_link_libraries(testcalfilter ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testcalfilter ${EXECUTABLE_OUTPUT_PATH}/testcalfilter)

########### next target ###############
set(testduration_SRCS testduration.cpp )

kde4_automoc(${testduration_SRCS})

kde4_add_executable(testduration NOGUI RUN_UNINSTALLED ${testduration_SRCS})

target_link_libraries(testduration ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testduration ${EXECUTABLE_OUTPUT_PATH}/testduration)

########### next target ###############
set(testevent_SRCS testevent.cpp )

kde4_automoc(${testevent_SRCS})

kde4_add_executable(testevent NOGUI RUN_UNINSTALLED ${testevent_SRCS})

target_link_libraries(testevent ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testevent ${EXECUTABLE_OUTPUT_PATH}/testevent)

########### next target ###############
set(testperson_SRCS testperson.cpp )

kde4_automoc(${testperson_SRCS})

kde4_add_executable(testperson NOGUI RUN_UNINSTALLED ${testperson_SRCS})

target_link_libraries(testperson ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testperson ${EXECUTABLE_OUTPUT_PATH}/testperson)

########### next target ###############
set(testperiod_SRCS testperiod.cpp )

kde4_automoc(${testperiod_SRCS})

kde4_add_executable(testperiod NOGUI RUN_UNINSTALLED ${testperiod_SRCS})

target_link_libraries(testperiod ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testperiod ${EXECUTABLE_OUTPUT_PATH}/testperiod)

########### next target ###############

set(testsortablelist_SRCS testsortablelist.cpp )

kde4_automoc(${testsortablelist_SRCS})

kde4_add_executable(testsortablelist NOGUI RUN_UNINSTALLED ${testsortablelist_SRCS})

target_link_libraries(testsortablelist  ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testsortablelist ${EXECUTABLE_OUTPUT_PATH}/testsortablelist)

########### next target ###############
set(testtodo_SRCS testtodo.cpp )

kde4_automoc(${testtodo_SRCS})

kde4_add_executable(testtodo NOGUI RUN_UNINSTALLED ${testtodo_SRCS})

target_link_libraries(testtodo ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

add_test(kcal-testtodo ${EXECUTABLE_OUTPUT_PATH}/testtodo)

########### next target ###############
set(testvcalexport_SRCS testvcalexport.cpp )

kde4_automoc(${testvcalexport_SRCS})

kde4_add_executable(testvcalexport RUN_UNINSTALLED ${testvcalexport_SRCS})

target_link_libraries(testvcalexport ${KDE4_KDECORE_LIBS} kcal )

########### install files ###############

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

endif(KDE4_BUILD_TESTS)





#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/libkcal -I$(top_builddir)/libkcal/libical/src \
#	-Iqttestlib $(all_includes)
#
#
#check_PROGRAMS = loadcalendar \
#                 testtostring \
#                 testincidence \
#                 testcalendar \
#                 readandwrite \
#                 testresource \
#                 testfields \
#                 testrecurrence \
#                 testrecurprevious \
#                 testrecurson \
#                 testrecurrencetype \
#		 testicaltimezones \
#                 testvcalexport
#
#
#METASOURCES = AUTO
#
#loadcalendar_SOURCES = loadcalendar.cpp
#loadcalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#loadcalendar_LDADD = ../libkcal.la
#
#testincidence_SOURCES = testincidence.cpp
#testincidence_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testincidence_LDADD = ../libkcal.la
#
#testcalendar_SOURCES = testcalendar.cpp
#testcalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testcalendar_LDADD = ../libkcal.la
#
#testtostring_SOURCES = testtostring.cpp
#testtostring_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testtostring_LDADD = ../libkcal.la
#
#readandwrite_SOURCES = readandwrite.cpp
#readandwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#readandwrite_LDADD = ../libkcal.la
#
#testresource_SOURCES = testresource.cpp
#testresource_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testresource_LDADD = ../libkcal.la
#
#testrecurrence_SOURCES = testrecurrence.cpp
#testrecurrence_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testrecurrence_LDADD = ../libkcal.la
#
#testrecurprevious_SOURCES = testrecurprevious.cpp
#testrecurprevious_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testrecurprevious_LDADD = ../libkcal.la
#
#testrecurson_SOURCES = testrecurson.cpp
#testrecurson_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testrecurson_LDADD = ../libkcal.la
#
#testrecurrencetype_SOURCES = testrecurrencetype.cpp
#testrecurrencetype_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testrecurrencetype_LDADD = ../libkcal.la
#
#testicaltimezones_SOURCES = testicaltimezones.cpp
#testicaltimezones_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testicaltimezones_LDADD = ../libkcal.la -lQtTest_debug
#
#testvcalexport_SOURCES = testvcalexport.cpp
#testvcalexport_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testvcalexport_LDADD = ../libkcal.la
#
#testfields_SOURCES = testfields.cpp
#testfields_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testfields_LDADD   = ../libkcal.la
#testfields_CXXFLAGS= -DINPUT='"$(srcdir)/data/test_pilot.ics"'
#
#TESTFILES = test1.ics test2.ics test3.ics test4.ics test5.ics test_Mozilla.ics
#
#check-local: readandwrite testrecurrence testrecurprevious testrecurson testvcalexport testicaltimezones
#	rm -f FAILED; \
#	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
#	  perl $(srcdir)/runtestcase.pl testrecurrence "next" $$i; \
#	done; \
#	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
#	  perl $(srcdir)/runtestcase.pl testrecurprevious "prev" $$i; \
#	done; \
#	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
#	  perl $(srcdir)/runtestcase.pl testrecurson "recurson" $$i; \
#	done; \
#	for i in `find $(srcdir)/data/Compat/ -name "*.ics"`; do \
#	  perl $(srcdir)/runtestcase.pl readandwrite "ical" $$i; \
#	done; \
#	for i in `find $(srcdir)/data/vCalendar/ -name "*.ics"`; do \
#		perl $(srcdir)/runtestcase.pl testvcalexport "vcal" $$i; \
#	done;
#	for i in `find $(srcdir)/data/vCalendar/ -name "*.vcs"`; do \
#		perl $(srcdir)/runtestcase.pl readandwrite "ical" $$i; \
#	done;
#	[ ! -e FAILED ]
#
