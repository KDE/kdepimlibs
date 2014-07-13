include(ECMMarkAsTest)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

# Turn exceptions on
kde_enable_exceptions()

macro(ADD_KMIME_TEST)
  foreach(_testName ${ARGN})
    add_executable(${_testName} ${_testName}.cpp)
    add_test(kmime-${_testName} ${_testName})
    target_link_libraries(${_testName}
      KF5Mime
      Qt5::Test
      KF5::Codecs
    )
    set_target_properties(${_testName} PROPERTIES COMPILE_FLAGS -DTEST_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\"" )
    ecm_mark_as_test(${_testName})
  endforeach()
endmacro()

add_kmime_test(
  rfc2047test
  utiltest
  contentindextest
  charfreqtest
  headertest
  contenttest
  messagetest
  codectest
  #headerfactorytest
  rfc2231test
  base64benchmark
  sizetest
)
