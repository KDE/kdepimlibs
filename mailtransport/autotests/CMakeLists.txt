
include(ECMMarkAsTest)

find_package(Qt5Test CONFIG REQUIRED)

macro(add_akonadi_isolated_test _source _path)
  get_filename_component(_targetName ${_source} NAME_WE)
  set(_srcList ${_source} )

  kde4_add_executable(${_targetName} TEST ${_srcList})
  target_link_libraries(${_targetName}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    
    akonadi-kmime
    mailtransport
    kmime
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDE4SUPPORT_LIBS}
  )

  # based on kde4_add_unit_test
  if (WIN32)
    get_target_property( _loc ${_targetName} LOCATION )
    set(_executable ${_loc}.bat)
  else()
    set(_executable ${EXECUTABLE_OUTPUT_PATH}/${_targetName})
  endif()
  if (UNIX)
    set(_executable ${_executable}.shell)
  endif()

  find_program(_testrunner akonaditest)

  if (KDEPIMLIBS_RUN_ISOLATED_TESTS)
    add_test( mailtransport-${_targetName} ${_testrunner} -c ${CMAKE_CURRENT_SOURCE_DIR}/${_path}/config.xml ${_executable} )
  endif()
endmacro(add_akonadi_isolated_test)



# Independent executables:

set(tm_srcs transportmgr.cpp)
add_executable(transportmgr ${tm_srcs})
add_test(transportmgr transportmgr)
ecm_mark_as_test(transportmgr)
target_link_libraries(transportmgr KF5MailTransport)

set(queuer_srcs queuer.cpp)
add_executable(queuer ${queuer_srcs})
add_test(queuer queuer)
ecm_mark_as_test(queuer)
target_link_libraries(queuer KF5MailTransport)

set( sendqueued_srcs sendqueued.cpp )
add_executable( sendqueued ${sendqueued_srcs} )
add_test(sendqueued sendqueued)
ecm_mark_as_test(sendqueued)
target_link_libraries( sendqueued KF5MailTransport  KF5::AkonadiMime)

set( clearerror_srcs clearerror.cpp )
add_executable( clearerror ${clearerror_srcs} )
add_test(clearerror clearerror)
ecm_mark_as_test(clearerror)
target_link_libraries( clearerror KF5MailTransport  KF5::AkonadiMime)

set( abort_srcs abort.cpp )
add_executable( abort ${abort_srcs} )
add_test(abort abort)
ecm_mark_as_test(abort)
target_link_libraries( abort KF5MailTransport KF5::AkonadiCore)


# Akonadi testrunner-based tests:

#add_akonadi_isolated_test( attributetest.cpp unittestenv )
#add_akonadi_isolated_test( messagequeuejobtest.cpp unittestenv )
if (KDEPIMLIBS_RUN_KDEPIMRUNTIME_ISOLATED_TESTS)
#add_akonadi_isolated_test( filteractiontest.cpp unittestenv_akonadi )
endif()
