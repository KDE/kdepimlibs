project(kcal)

# was used by libical, but not anymore
#if (UNIX)
#  add_definitions(-DICAL_UNIX_NEWLINE=1)
#endif (UNIX)

ADD_DEFINITIONS(-DPACKAGE_DATA_DIR=\\""${DATA_INSTALL_DIR}/libical"\\" ) # added manually

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5800)

include (ConfigureChecks.cmake)
configure_file (libical/config-libical.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/libical/config-libical.h)

if(KDE4_BUILD_TESTS)
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/libical/src/libicalss/
	${CMAKE_CURRENT_BINARY_DIR}/libical/src/libicalss/
	${CMAKE_CURRENT_SOURCE_DIR}/versit/
	${CMAKE_CURRENT_SOURCE_DIR}/libical/src/libical/
	${CMAKE_CURRENT_BINARY_DIR}/libical/src/libical/
	${CMAKE_CURRENT_BINARY_DIR}/libical # for config.h
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/kabc
	${CMAKE_BINARY_DIR}/kabc
        ${KDE4_INCLUDE_DIR}
)

set(ICALSCRIPTS ${CMAKE_SOURCE_DIR}/kcal/libical/scripts/ )

set(PROPERTYDEPS 
	${ICALSCRIPTS}/mkderivedproperties.pl 
	${CMAKE_SOURCE_DIR}/kcal/libical/design-data/properties.csv	
	${CMAKE_SOURCE_DIR}/kcal/libical/design-data/value-types.csv
	${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedproperty.h.in 
	${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedproperty.c.in 
)

add_custom_command(OUTPUT
${CMAKE_BINARY_DIR}/kcal/libical/icalderivedproperty.h
COMMAND ${PERL_EXECUTABLE} ARGS -I ${ICALSCRIPTS}
${ICALSCRIPTS}/mkderivedproperties.pl -i
${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedproperty.h.in
-h  ${CMAKE_SOURCE_DIR}/kcal/libical/design-data/properties.csv ${CMAKE_SOURCE_DIR}/kcal/libical/design-data/value-types.csv > ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedproperty.h   DEPENDS  ${PROPERTYDEPS} )


add_custom_command(OUTPUT
${CMAKE_BINARY_DIR}/kcal/libical/icalderivedproperty.c
COMMAND ${PERL_EXECUTABLE} ARGS -I${ICALSCRIPTS}
${ICALSCRIPTS}/mkderivedproperties.pl -i
${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedproperty.c.in
-c ${CMAKE_SOURCE_DIR}/kcal/libical/design-data/properties.csv
${CMAKE_SOURCE_DIR}/kcal/libical/design-data/value-types.csv > ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedproperty.c   DEPENDS  ${PROPERTYDEPS} ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedproperty.h )


# files generated via add_custom_command are added automatically to the clean files, Alex
#macro_additional_clean_files( 
#                              ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedproperty.c 
#                              ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedproperty.h 
#)


set(PARAMETERDEPS 
	${ICALSCRIPTS}/mkderivedparameters.pl 
	${CMAKE_SOURCE_DIR}/kcal/libical/design-data/parameters.csv 
	${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedparameter.c.in
	${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedparameter.h.in
)

add_custom_command(OUTPUT
${CMAKE_BINARY_DIR}/kcal/libical/icalderivedparameter.h
COMMAND ${PERL_EXECUTABLE} ARGS -I ${ICALSCRIPTS} ${ICALSCRIPTS}/mkderivedparameters.pl -i ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedparameter.h.in -h ${CMAKE_SOURCE_DIR}/kcal/libical/design-data/parameters.csv > ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedparameter.h   DEPENDS  ${PARAMETERDEPS} )

add_custom_command(OUTPUT
${CMAKE_BINARY_DIR}/kcal/libical/icalderivedparameter.c
COMMAND ${PERL_EXECUTABLE} ARGS -I ${ICALSCRIPTS} ${ICALSCRIPTS}/mkderivedparameters.pl -i ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedparameter.c.in -c ${CMAKE_SOURCE_DIR}/kcal/libical/design-data/parameters.csv > ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedparameter.c   DEPENDS  ${PARAMETERDEPS} ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedparameter.h )

#files generated via add_custom_command are added automatically to the clean files, Alex
#macro_additional_clean_files( 
#	${CMAKE_BINARY_DIR}/kcal/libical/icalderivedparameter.c 
#	${CMAKE_BINARY_DIR}/kcal/libical/icalderivedparameter.h 
#)


set(RESTRICTIONDEPS 
	${ICALSCRIPTS}/mkrestrictiontable.pl
	${CMAKE_SOURCE_DIR}/kcal/libical/design-data/restrictions.csv
	${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalrestriction.c.in
)

add_custom_command(OUTPUT
${CMAKE_BINARY_DIR}/kcal/libical/src/libical/icalrestriction.c
COMMAND ${PERL_EXECUTABLE} ARGS -I ${ICALSCRIPTS} ${ICALSCRIPTS}/mkrestrictiontable.pl -i ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalrestriction.c.in ${CMAKE_SOURCE_DIR}/kcal/libical/design-data/restrictions.csv > ${CMAKE_BINARY_DIR}/kcal/libical/src/libical/icalrestriction.c   DEPENDS  ${RESTRICTIONDEPS} )


#files generated via add_custom_command are added automatically to the clean files, Alex
#macro_additional_clean_files( ${CMAKE_BINARY_DIR}/kcal/libical/src/libical/icalrestriction.c)

set(VALUEDEPS
	${CMAKE_SOURCE_DIR}/kcal/libical/scripts/mkderivedvalues.pl
	${CMAKE_SOURCE_DIR}/kcal/libical/design-data/value-types.csv
	${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedvalue.c.in
	${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedvalue.h.in
)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedvalue.h COMMAND ${PERL_EXECUTABLE} ARGS -I${ICALSCRIPTS} ${ICALSCRIPTS}/mkderivedvalues.pl -i ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedvalue.h.in -h ${CMAKE_SOURCE_DIR}/kcal/libical/design-data/value-types.csv > ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedvalue.h   DEPENDS  ${VALUEDEPS} )


add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedvalue.c COMMAND ${PERL_EXECUTABLE} ARGS -I${ICALSCRIPTS} ${ICALSCRIPTS}/mkderivedvalues.pl -i ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalderivedvalue.c.in -c ${CMAKE_SOURCE_DIR}/kcal/libical/design-data/value-types.csv > ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedvalue.c   DEPENDS  ${VALUEDEPS} ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedvalue.h )

#files generated via add_custom_command are added automatically to the clean files, Alex
#macro_additional_clean_files(
#	${CMAKE_BINARY_DIR}/kcal/libical/icalderivedvalue.h
#	${CMAKE_BINARY_DIR}/kcal/libical/icalderivedvalue.c
#)



set(libversit_SRCS 
   ${CMAKE_SOURCE_DIR}/kcal/versit/vcc.c 
   ${CMAKE_SOURCE_DIR}/kcal/versit/vobject.c 
)

# Keep absolute paths here, they are used in tests/CMakeLists.txt too.
set(libical_SRCS
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/caldate.c       
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalarray.c     
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalattach.c        
   ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedparameter.c  
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalrecur.c     
   ${CMAKE_BINARY_DIR}/kcal/libical/src/libical/icalrestriction.c   
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalcomponent.c     
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalenums.c     
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalerror.c     
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalmemory.c        
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalmime.c      
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalparameter.c    
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalparser.c        
   ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedproperty.c   
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalproperty.c      
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icaltime.c      
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalduration.c      
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalperiod.c        
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icaltimezone.c      
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icaltypes.c     
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/icalvalue.c     
   ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedvalue.c  
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/pvl.c           
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/sspm.c          
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libical/vsnprintf.c     
)

set(libicalss_SRCS
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libicalss/icalclassify.c      
   ${CMAKE_SOURCE_DIR}/kcal/libical/src/libicalss/icalclassify.h      
)


macro_additional_clean_files(${CMAKE_BINARY_DIR}/kcal/libical/src/libical/ical.h)

if(WIN32)
  set(TOPS "\"${CMAKE_SOURCE_DIR}\"")
  set(TOPB "\"${CMAKE_BINARY_DIR}\"")
else(WIN32)
  set(TOPS "${CMAKE_SOURCE_DIR}")
  set(TOPB "${CMAKE_BINARY_DIR}")
endif(WIN32)
add_custom_command(OUTPUT
   ${CMAKE_BINARY_DIR}/kcal/libical/src/libical/ical.h 
   COMMAND 
   ${CMAKE_COMMAND} 
   -DTOPS:FILEPATH=${TOPS}
   -DTOPB:FILEPATH=${TOPB}
   -DKDE_FILE_H_FILE:FILEPATH=${CMAKE_BINARY_DIR}/kcal/libical/src/libical/ical.h
   -P ${CMAKE_SOURCE_DIR}/kcal/ical_file.cmake
   DEPENDS
   ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedproperty.h
   ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedparameter.h
   ${CMAKE_BINARY_DIR}/kcal/libical/icalderivedvalue.h
)

macro_additional_clean_files(${CMAKE_BINARY_DIR}/libical/src/libicalss/icalss.h)

add_custom_command(OUTPUT
   ${CMAKE_BINARY_DIR}/kcal/libical/src/libicalss/icalss.h 
   COMMAND 
   ${CMAKE_COMMAND} 
   -DTOP:FILEPATH=${TOPS}
   -DKDE_FILE_H_FILE:FILEPATH=${CMAKE_BINARY_DIR}/kcal/libical/src/libicalss/icalss.h
   -P ${CMAKE_SOURCE_DIR}/kcal/icalss_file.cmake
)


########### next target ###############

set(kcal_LIB_SRCS ${libversit_SRCS} ${libical_SRCS} ${libicalss_SRCS}
   incidencebase.cpp
   incidence.cpp
   journal.cpp
   todo.cpp
   event.cpp
   freebusy.cpp
   attendee.cpp
   attachment.cpp
   recurrencerule.cpp
   recurrence.cpp
   alarm.cpp
   customproperties.cpp
   calendar.cpp
   calendarlocal.cpp
   calformat.cpp
   vcalformat.cpp
   icalformat.cpp
   icalformat_p.cpp
   incidenceformatter.cpp
   vcaldrag.cpp
   icaldrag.cpp
   exceptions.cpp
   scheduler.cpp
   imipscheduler.cpp
   dummyscheduler.cpp
   calfilter.cpp
   person.cpp
   period.cpp
   duration.cpp
   dndfactory.cpp
   calstorage.cpp
   filestorage.cpp
   compat.cpp
   resourcecalendar.cpp
   resourcelocal.cpp
   resourcelocalconfig.cpp
   resourcelocaldir.cpp
   resourcelocaldirconfig.cpp
   resourcecached.cpp
   resourcecachedconfig.cpp
   calendarresources.cpp
   qtopiaformat.cpp
   htmlexport.cpp
   calendarnull.cpp
   freebusyurlstore.cpp
   confirmsavedialog.cpp
   icaltimezones.cpp
   kresult.cpp )

kde4_add_kcfg_files(kcal_LIB_SRCS htmlexportsettings.kcfgc )

kde4_add_library(kcal SHARED ${kcal_LIB_SRCS})

target_link_libraries(kcal ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} kresources kabc kpimutils )

set_target_properties(kcal PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kcal DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

# hack to force ical[ss].h creation
add_custom_target(ical-headers DEPENDS
        ${CMAKE_BINARY_DIR}/kcal/libical/src/libical/ical.h
        ${CMAKE_BINARY_DIR}/kcal/libical/src/libicalss/icalss.h
)
add_dependencies(kcal ical-headers)

########### next target ###############

set(kcal_local_PART_SRCS resourcelocal_plugin.cpp )


kde4_add_plugin(kcal_local ${kcal_local_PART_SRCS})


target_link_libraries(kcal_local  ${KDE4_KDECORE_LIBS} kcal )

install(TARGETS kcal_local  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcal_localdir_PART_SRCS resourcelocaldir_plugin.cpp )


kde4_add_plugin(kcal_localdir ${kcal_localdir_PART_SRCS})


target_link_libraries(kcal_localdir  ${KDE4_KDECORE_LIBS} kcal )

install(TARGETS kcal_localdir  DESTINATION ${PLUGIN_INSTALL_DIR})


add_subdirectory( libical )
add_subdirectory( tests )

########### install files ###############

install( FILES local.desktop localdir.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)
install( FILES
	alarm.h
	attachment.h
	attendee.h
	calendar.h
	calendarlocal.h
	calendarnull.h
	calendarresources.h
	calfilter.h
	calformat.h
	calstorage.h
	confirmsavedialog.h
	customproperties.h
	dndfactory.h
	duration.h
	event.h
	exceptions.h
	filestorage.h
	freebusy.h
	freebusycache.h
	freebusyurlstore.h
	${CMAKE_CURRENT_BINARY_DIR}/htmlexportsettings.h
	htmlexport.h
	icaldrag.h
	icalformat.h
	icaltimezones.h
	imipscheduler.h
	incidencebase.h
	incidence.h
	incidenceformatter.h
	journal.h
	kcal_export.h
	kcalversion.h
	listbase.h
	period.h
	person.h
	qtopiaformat.h
	recurrencerule.h
	recurrence.h
	resourcecached.h
	resourcecachedconfig.h
	resourcecalendar.h
	resourcelocalconfig.h
	resourcelocaldirconfig.h
	resourcelocaldir.h
	resourcelocal.h
	scheduler.h
	sortablelist.h
	todo.h
	vcaldrag.h
	vcalformat.h
        kresult.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kcal)
install( FILES kcal_manager.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources)
