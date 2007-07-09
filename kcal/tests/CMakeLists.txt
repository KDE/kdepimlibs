
include_directories(
	${CMAKE_SOURCE_DIR}/kcal
	${CMAKE_BINARY_DIR}/kcal/libical/src
	
)



########### next target ###############

set(convertqtopia_SRCS convertqtopia.cpp )


kde4_add_test_executable(convertqtopia ${convertqtopia_SRCS})

target_link_libraries(convertqtopia  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############

set(loadcalendar_SRCS loadcalendar.cpp )



kde4_add_test_executable(loadcalendar ${loadcalendar_SRCS})

target_link_libraries(loadcalendar  ${KDE4_KDECORE_LIBS} kcal )


########### next target ###############

set(testtostring_SRCS testtostring.cpp )



kde4_add_test_executable(testtostring ${testtostring_SRCS})

target_link_libraries(testtostring  ${KDE4_KDECORE_LIBS} kcal )


########### next target ###############
set(incidencestest_SRCS incidencestest.cpp )



kde4_add_test_executable(incidencestest ${incidencestest_SRCS})

target_link_libraries(incidencestest  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############
set(readandwrite_SRCS readandwrite.cpp )



kde4_add_test_executable(readandwrite ${readandwrite_SRCS})

target_link_libraries(readandwrite  ${KDE4_KDECORE_LIBS} kcal )


########### next target ###############
set(testresource_SRCS testresource.cpp )



kde4_add_test_executable(testresource ${testresource_SRCS})

target_link_libraries(testresource  ${KDE4_KDECORE_LIBS} kcal )


########### next target ###############
set(testfields_SRCS testfields.cpp )
set(srcfile "${CMAKE_SOURCE_DIR}/kcal/tests/data/test_pilot.ics")
set_source_files_properties( ${testfields_SRCS} PROPERTIES 
	COMPILE_FLAGS -DINPUT="\\"${srcfile}\\"" )


kde4_add_test_executable(testfields ${testfields_SRCS})

target_link_libraries(testfields  ${KDE4_KDECORE_LIBS} kcal )


########### next target ###############
set(testrecurrence_SRCS testrecurrence.cpp )



kde4_add_test_executable(testrecurrence ${testrecurrence_SRCS})

target_link_libraries(testrecurrence  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############
set(testrecurprevious_SRCS testrecurprevious.cpp )



kde4_add_test_executable(testrecurprevious ${testrecurprevious_SRCS})

target_link_libraries(testrecurprevious  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############
set(testrecurson_SRCS testrecurson.cpp )



kde4_add_test_executable(testrecurson ${testrecurson_SRCS})

target_link_libraries(testrecurson  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############
set(testrecurrencetype_SRCS testrecurrencetype.cpp )



kde4_add_test_executable(testrecurrencetype ${testrecurrencetype_SRCS})

target_link_libraries(testrecurrencetype  ${KDE4_KDECORE_LIBS} kcal )

########### next target ###############

# this test cannot work with msvc because libical should not be altered
# and therefore we can't add KCAL_EXPORT there
# it should work fine with mingw because of the auto-import feature
if(NOT MSVC)
  set(testicaltimezones_SRCS testicaltimezones.cpp
 )


  kde4_add_unit_test(testicaltimezones TESTNAME kcal-testicaltimezones  ${testicaltimezones_SRCS})

  target_link_libraries(testicaltimezones ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})

endif(NOT MSVC)

########### next target ###############
set(testalarm_SRCS testalarm.cpp )


kde4_add_unit_test(testalarm TESTNAME kcal-testalarm  ${testalarm_SRCS})

target_link_libraries(testalarm ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})


########### next target ###############
set(testattachment_SRCS testattachment.cpp )


kde4_add_unit_test(testattachment TESTNAME kcal-testattachment  ${testattachment_SRCS})

target_link_libraries(testattachment ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})



########### next target ###############
set(testattendee_SRCS testattendee.cpp )


kde4_add_unit_test(testattendee TESTNAME kcal-testattendee  ${testattendee_SRCS})

target_link_libraries(testattendee ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})


########### next target ###############
set(testcalendarlocal_SRCS testcalendarlocal.cpp )


kde4_add_unit_test(testcalendarlocal TESTNAME kcal-testcalendarlocal  ${testcalendarlocal_SRCS})

target_link_libraries(testcalendarlocal ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})


########### next target ###############
set(testcalendarnull_SRCS testcalendarnull.cpp )


kde4_add_unit_test(testcalendarnull TESTNAME kcal-testcalendarnull  ${testcalendarnull_SRCS})

target_link_libraries(testcalendarnull ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})


########### next target ###############
set(testcalfilter_SRCS testcalfilter.cpp )


kde4_add_unit_test(testcalfilter TESTNAME kcal-testcalfilter  ${testcalfilter_SRCS})

target_link_libraries(testcalfilter ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})


########### next target ###############
set(testduration_SRCS testduration.cpp )


kde4_add_unit_test(testduration TESTNAME kcal-testduration  ${testduration_SRCS})

target_link_libraries(testduration ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})


########### next target ###############
set(testevent_SRCS testevent.cpp )


kde4_add_unit_test(testevent TESTNAME kcal-testevent  ${testevent_SRCS})

target_link_libraries(testevent ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})


########### next target ###############
set(testperson_SRCS testperson.cpp )


kde4_add_unit_test(testperson TESTNAME kcal-testperson  ${testperson_SRCS})

target_link_libraries(testperson ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})


########### next target ###############
set(testperiod_SRCS testperiod.cpp )


kde4_add_unit_test(testperiod TESTNAME kcal-testperiod  ${testperiod_SRCS})

target_link_libraries(testperiod ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})


########### next target ###############

set(testsortablelist_SRCS testsortablelist.cpp )


kde4_add_unit_test(testsortablelist TESTNAME kcal-testsortablelist  ${testsortablelist_SRCS})

target_link_libraries(testsortablelist  ${KDE4_KDECORE_LIBS} kcal ${QT_QTTEST_LIBRARY})


########### next target ###############
set(testvcalexport_SRCS testvcalexport.cpp )


kde4_add_test_executable(testvcalexport ${testvcalexport_SRCS})

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
