 
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_KIO_INCLUDES} )
 
# ########### next target ###############
 
 set(testblogger1_SRCS testblogger1.cpp )
 
 
 kde4_add_unit_test(testblogger1 TESTNAME kabc-testblogger1 ${testblogger1_SRCS})
 
 target_link_libraries(testblogger1 ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kblog)
 
# ########### next target ###############

set(testgdata_SRCS testgdata.cpp )


kde4_add_unit_test(testgdata TESTNAME kabc-testgdata ${testgdata_SRCS})

target_link_libraries(testgdata ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kblog)

# ########### next target ###############
 
 set(testmetaweblog_SRCS testmetaweblog.cpp )
 
 
 kde4_add_unit_test(testmetaweblog TESTNAME kabc-testmetaweblog ${testmetaweblog_SRCS})
 
 target_link_libraries(testmetaweblog ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kblog)

