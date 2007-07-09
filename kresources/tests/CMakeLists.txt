
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/kresources )

########### next target ###############

set(testresources_SRCS testresources.cpp)


kde4_add_unit_test(testresources TESTNAME kresources-testresources  ${testresources_SRCS})

target_link_libraries(testresources ${KDE4_KDECORE_LIBS} kresources)

########### next target ###############

set(idmappertest_SRCS idmappertest.cpp)


kde4_add_unit_test(idmappertest TESTNAME kresources-idmappertest  ${idmappertest_SRCS})

target_link_libraries(idmappertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kresources)

