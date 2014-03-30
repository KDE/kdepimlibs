kde_enable_exceptions()

# TODO: Add a cmake option for this and enable it on jenkins
set( PLEASE_TEST_INVITATIONS FALSE)
if ( PLEASE_TEST_INVITATIONS )
  add_definitions( -DPLEASE_TEST_INVITATIONS )
endif()

#add_subdirectory( tests )


remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)

set(akonadicalendar_LIB_SRC
  blockalarmsattribute.cpp
  calendarbase.cpp
  calendarclipboard.cpp
  calendarmodel.cpp
  calfilterproxymodel_p.cpp
  etmcalendar.cpp
  history.cpp
  history_p.cpp
  icalimporter.cpp
  incidencefetchjob_p.cpp
  incidencechanger.cpp
  incidencechanger_p.cpp
  itiphandler.cpp
  itiphandler_p.cpp
  itiphandlerhelper_p.cpp
  kcolumnfilterproxymodel.cpp
  fetchjobcalendar.cpp
  freebusydownloadjob_p.cpp
  freebusymanager.cpp
  freebusyproviderbase.cpp
  mailclient_p.cpp
  mailscheduler_p.cpp
  publishdialog.cpp
  publishdialog_p.cpp
  scheduler_p.cpp
  standardcalendaractionmanager.cpp
  todopurger.cpp
  utils_p.cpp
)

kde4_add_kcfg_files(akonadicalendar_LIB_SRC calendarsettings.kcfgc)
kde4_add_ui_files(akonadicalendar_LIB_SRC publishdialog_base.ui)

#qt5_add_dbus_adaptor(akonadicalendar_LIB_SRC ${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.Resource.FreeBusyProvider.xml freebusyproviderbase_p.h Akonadi::FreeBusyProviderBasePrivate freebusyprovideradaptor  Akonadi__FreeBusyProviderAdaptor)

add_library(KF5AkonadiCalendar ${akonadicalendar_LIB_SRC})
generate_export_header(KF5AkonadiCalendar BASE_NAME akonadi-calendar)
add_library(KF5::KF5AkonadiCalendar ALIAS KF5AkonadiCalendar)
target_include_directories(KF5AkonadiCalendar INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/AkonadiCalendar;${Boost_INCLUDE_DIR}>")

target_link_libraries(KF5AkonadiCalendar
    KF5::KMime
    KF5::AkonadiCore
    KF5::AkonadiWidgets
    KF5::KIOCore
    KF5::ItemModels
    KF5::CalendarCore
    KF5::CalendarUtils
    KF5::PimUtils
    KF5::PimIdentities
)

set_target_properties(KF5AkonadiCalendar PROPERTIES
    VERSION ${AKONADICALENDAR_VERSION_STRING}
    SOVERSION ${AKONADICALENDAR_SOVERSION}
    EXPORT_NAME AkonadiCalendar
)

ecm_generate_pri_file(BASE_NAME AkonadiCalendar LIB_NAME KF5AkonadiCalendar
DEPS "KMime AkonadiCore AkonadiWidgets KIOCore CalendarCore CalendarUtils PimUtils PimIdentities ItemModels"
FILENAME_VAR PRI_FILENAME)

install(TARGETS
    KF5AkonadiCalendar
    EXPORT KF5AkonadiCalendarTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install( FILES
  ${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_export.h
  blockalarmsattribute.h
  calendarbase.h
  calendarclipboard.h
  etmcalendar.h
  history.h
  icalimporter.h
  incidencechanger.h
  itiphandler.h
  fetchjobcalendar.h
  freebusymanager.h
  freebusyproviderbase.h
  publishdialog.h
  standardcalendaractionmanager.h
  todopurger.h
  ${CMAKE_CURRENT_BINARY_DIR}/calendarsettings.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/akonadi/calendar COMPONENT Devel
)
