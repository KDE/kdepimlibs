add_definitions(
  -DKRESOURCES_DEPRECATED=
)
# This one won't be needed when CMake 2.8.13 is depended on.
add_definitions(
  -DKRESOURCES_DEPRECATED_EXPORT=KRESOURCES_EXPORT
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

set(testresources_SRCS testresources.cpp)

kde4_add_unit_test(testresources TESTNAME kresources-testresources ${testresources_SRCS})

target_link_libraries(testresources ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kresources)

########### next target ###############

set(idmappertest_SRCS idmappertest.cpp)

kde4_add_unit_test(idmappertest TESTNAME kresources-idmappertest ${idmappertest_SRCS})

target_link_libraries(idmappertest
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  kresources
)
