
include(ECMMarkAsTest)

find_package(Qt5Test CONFIG REQUIRED)

macro(add_akonadi_isolated_test _source _path)
  get_filename_component(_targetName ${_source} NAME_WE)
  set(_srcList ${_source} )

  add_executable(${_targetName} TEST ${_srcList})
  target_link_libraries(${_targetName}
    Qt5::Test
    akonadi-kmime
    mailtransport
    kmime
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



# Akonadi testrunner-based tests:

#add_akonadi_isolated_test( attributetest.cpp unittestenv )
#add_akonadi_isolated_test( messagequeuejobtest.cpp unittestenv )
if (KDEPIMLIBS_RUN_KDEPIMRUNTIME_ISOLATED_TESTS)
#add_akonadi_isolated_test( filteractiontest.cpp unittestenv_akonadi )
endif()
