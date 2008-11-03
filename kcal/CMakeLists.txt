project(kcal)

find_package(Libical)
macro_log_feature(LIBICAL_FOUND "libical" "Reference implementation of the iCalendar data type and serialization format" "http://sourceforge.net/projects/freeassociation" TRUE "0.33" "Required for the critical PIM kcal library.")
if(NOT LIBICAL_FOUND)
  macro_display_feature_log()
endif(NOT LIBICAL_FOUND)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5800)

include (ConfigureChecks.cmake)

if(KDE4_BUILD_TESTS)
  add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

include_directories(
  ${LIBICAL_INCLUDE_DIRS}
  ${LIBICAL_INCLUDE_DIRS}/libical
  ${CMAKE_CURRENT_SOURCE_DIR}/versit
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/kabc
  ${CMAKE_BINARY_DIR}/kabc
  ${KDE4_INCLUDE_DIR}
)

set(libversit_SRCS 
   ${CMAKE_SOURCE_DIR}/kcal/versit/vcc.c 
   ${CMAKE_SOURCE_DIR}/kcal/versit/vobject.c 
)

########### next target ###############

set(kcal_LIB_SRCS ${libversit_SRCS}
  incidencebase.cpp
  incidence.cpp
  journal.cpp
  todo.cpp
  event.cpp
  freebusy.cpp
  freebusyperiod.cpp
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

target_link_libraries(kcal ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} kresources kabc kpimutils ${LIBICAL_LIBRARIES})

set_target_properties(kcal PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kcal ${INSTALL_TARGETS_DEFAULT_ARGS} COMPONENT Devel)

########### next target ###############

set(kcal_local_PART_SRCS resourcelocal_plugin.cpp )


kde4_add_plugin(kcal_local ${kcal_local_PART_SRCS})


target_link_libraries(kcal_local  ${KDE4_KDECORE_LIBS} kcal kresources )

install(TARGETS kcal_local  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcal_localdir_PART_SRCS resourcelocaldir_plugin.cpp )


kde4_add_plugin(kcal_localdir ${kcal_localdir_PART_SRCS})


target_link_libraries(kcal_localdir  ${KDE4_KDECORE_LIBS} kcal kresources)

install(TARGETS kcal_localdir  DESTINATION ${PLUGIN_INSTALL_DIR})


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
        freebusyperiod.h
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
        DESTINATION ${INCLUDE_INSTALL_DIR}/kcal COMPONENT Devel)

install( FILES kcal_manager.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources)
