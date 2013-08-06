project(kcal)


add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5800)
add_definitions(-DWANT_DEPRECATED_KCAL_API)
add_definitions(-DWANT_DEPRECATED_KABC_API)
add_definitions(-DWANT_DEPRECATED_KRESOURCE_API)

# Workaround a SIC change in libical 0.46
if(USE_ICAL_0_46)
  add_definitions(-DUSE_ICAL_0_46)
endif()

include (ConfigureChecks.cmake)

#if(KDE4_BUILD_TESTS)
# add_definitions(-DCOMPILING_TESTS)
#endif()

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
  calstorage.cpp
  filestorage.cpp
  compat.cpp
  qtopiaformat.cpp
  htmlexport.cpp
  calendarnull.cpp
  freebusyurlstore.cpp
  icaltimezones.cpp
  kresult.cpp
  assignmentvisitor.cpp
  comparisonvisitor.cpp
)

if (NOT WINCE)
  set(kcal_LIB_SRCS ${kcal_LIB_SRCS}
    dndfactory.cpp
    #disabled due to no QTreeWidget
    confirmsavedialog.cpp
  )
endif()


if (NOT KDEPIM_NO_KRESOURCES)
  set(kcal_LIB_SRCS ${kcal_LIB_SRCS}
    calhelper.cpp
    resourcecalendar.cpp
    resourcelocal.cpp
    resourcelocalconfig.cpp
    resourcelocaldir.cpp
    resourcelocaldirconfig.cpp
    resourcecached.cpp
    resourcecachedconfig.cpp
    calendarresources.cpp
  )
endif()

kde4_add_kcfg_files(kcal_LIB_SRCS htmlexportsettings.kcfgc )

kde4_add_library(kcal ${LIBRARY_TYPE} ${kcal_LIB_SRCS})

target_link_libraries(kcal ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} kabc kpimutils ${LIBICAL_LIBRARIES})
target_link_libraries(kcal LINK_INTERFACE_LIBRARIES kabc)
if (NOT KDEPIM_NO_KRESOURCES)
  target_link_libraries(kcal kresources)
  target_link_libraries(kcal LINK_INTERFACE_LIBRARIES kresources)
endif()

set_target_properties(kcal PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kcal EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

if (NOT KDEPIM_NO_KRESOURCES)
set(kcal_local_PART_SRCS resourcelocal_plugin.cpp )


kde4_add_plugin(kcal_local ${kcal_local_PART_SRCS})


target_link_libraries(kcal_local ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} kcal kresources )

install(TARGETS kcal_local  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcal_localdir_PART_SRCS resourcelocaldir_plugin.cpp )


kde4_add_plugin(kcal_localdir ${kcal_localdir_PART_SRCS})


target_link_libraries(kcal_localdir ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} kcal kresources)

install(TARGETS kcal_localdir  DESTINATION ${PLUGIN_INSTALL_DIR})
endif()

add_subdirectory( tests )

########### install files ###############

install( FILES
        alarm.h
        assignmentvisitor.h
        attachment.h
        attendee.h
        calendar.h
        calendarlocal.h
        calendarnull.h
        calfilter.h
        calformat.h
        calstorage.h
        comparisonvisitor.h
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
        scheduler.h
        sortablelist.h
        todo.h
        vcaldrag.h
        vcalformat.h
        kresult.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/kcal COMPONENT Devel)

if (NOT KDEPIM_NO_KRESOURCES)
  install( FILES localdir.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kcal)
  install( FILES
        calendarresources.h
        calhelper.h
        resourcecached.h
        resourcecachedconfig.h
        resourcecalendar.h
        resourcelocalconfig.h
        resourcelocaldirconfig.h
        resourcelocaldir.h
        resourcelocal.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/kcal COMPONENT Devel)
  install( FILES kcal_manager.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kresources)
endif()
