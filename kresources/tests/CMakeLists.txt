
# Test apps should be in the bin dir so that add_test can locate them
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/kresources )

if(KDE4_BUILD_TESTS)

########### next target ###############

set(testresources_SRCS testresources.cpp)

kde4_automoc(${testresources_SRCS})

kde4_add_executable(testresources NOGUI RUN_UNINSTALLED ${testresources_SRCS})

target_link_libraries(testresources ${KDE4_KDECORE_LIBS} kresources)

add_test(kabc-testresources ${EXECUTABLE_OUTPUT_PATH}/testresources)

########### next target ###############

set(idmappertest_SRCS idmappertest.cpp)

kde4_automoc(${idmappertest_SRCS})

kde4_add_executable(idmappertest NOGUI RUN_UNINSTALLED ${idmappertest_SRCS})

target_link_libraries(idmappertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kresources)

add_test(kresources-idmappertest ${EXECUTABLE_OUTPUT_PATH}/idmappertest)

endif(KDE4_BUILD_TESTS)

