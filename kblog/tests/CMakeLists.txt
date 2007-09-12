 
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

# ########### next target ###############
 
 set(testmovabletype_SRCS testmovabletype.cpp )
 
 
 kde4_add_unit_test(testmovabletype TESTNAME kabc-testmovabletype ${testmovabletype_SRCS})
 
 target_link_libraries(testmovabletype ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kblog)

# ########### next target ###############
 
 set(testwordpressbuggy_SRCS testwordpressbuggy.cpp )
 
 
 kde4_add_unit_test(testwordpressbuggy TESTNAME kabc-testwordpressbuggy ${testwordpressbuggy_SRCS})
 
 target_link_libraries(testwordpressbuggy ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kblog)

# ########### next target ###############
 
 set(testblogpost_SRCS testblogpost.cpp )
 
 
 kde4_add_unit_test(testblogpost TESTNAME kabc-testblogpost ${testblogpost_SRCS})
 
 target_link_libraries(testblogpost ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kblog)

# ########### next target ###############
 
 set(testblogmedia_SRCS testblogmedia.cpp )
 
 
 kde4_add_unit_test(testblogmedia TESTNAME kabc-testblogmedia ${testblogmedia_SRCS})
 
 target_link_libraries(testblogmedia ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kblog)

# ########### next target ###############
 
 set(testblogcomment_SRCS testblogcomment.cpp )
 
 
 kde4_add_unit_test(testblogcomment TESTNAME kabc-testblogcomment ${testblogcomment_SRCS})
 
 target_link_libraries(testblogcomment ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kblog)



