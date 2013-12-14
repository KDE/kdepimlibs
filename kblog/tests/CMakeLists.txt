set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
 
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_KIO_INCLUDES} )
 
# ########### next target ###############
 
 set(testblogger1_SRCS testblogger1.cpp )
 
 
 kde4_add_unit_test(testblogger1 TESTNAME kblog-testblogger1 ${testblogger1_SRCS})
 
 target_link_libraries(testblogger1 KF5::CoreAddons Qt5::Test kblog)
 
# ########### next target ###############

set(testgdata_SRCS testgdata.cpp )

# FIXME: disabled as unit test, www.blogger.com apparently changed their API
kde4_add_executable(testgdata ${testgdata_SRCS})

target_link_libraries(testgdata KF5::CoreAddons Qt5::Test kblog)

# ########### next target ###############
 
 set(testmetaweblog_SRCS testmetaweblog.cpp )
 
 
 kde4_add_unit_test(testmetaweblog TESTNAME kblog-testmetaweblog ${testmetaweblog_SRCS})
 
 target_link_libraries(testmetaweblog KF5::CoreAddons Qt5::Test kblog)

# ########### next target ###############
 
 set(testmovabletype_SRCS testmovabletype.cpp )
 
 
 kde4_add_unit_test(testmovabletype TESTNAME kblog-testmovabletype ${testmovabletype_SRCS})
 
 target_link_libraries(testmovabletype KF5::CoreAddons Qt5::Test kblog)

# ########### next target ###############
 
 set(testwordpressbuggy_SRCS testwordpressbuggy.cpp )
 
 
 kde4_add_unit_test(testwordpressbuggy TESTNAME kblog-testwordpressbuggy ${testwordpressbuggy_SRCS})
 
 target_link_libraries(testwordpressbuggy KF5::CoreAddons Qt5::Test kblog)

# ########### next target ###############
 
#  set(testlivejournal_SRCS testlivejournal.cpp )
#  
#  
#  kde4_add_unit_test(testlivejournal TESTNAME kblog-testlivejournal ${testlivejournal_SRCS})
#  
#  target_link_libraries(testlivejournal KF5::CoreAddons Qt5::Test kblog)

# ########### next target ###############
 
 set(testblogpost_SRCS testblogpost.cpp )
 
 
 kde4_add_unit_test(testblogpost TESTNAME kblog-testblogpost ${testblogpost_SRCS})
 
 target_link_libraries(testblogpost KF5::CoreAddons Qt5::Test kblog)

# ########### next target ###############
 
 set(testblogmedia_SRCS testblogmedia.cpp )
 
 
 kde4_add_unit_test(testblogmedia TESTNAME kblog-testblogmedia ${testblogmedia_SRCS})
 
 target_link_libraries(testblogmedia KF5::CoreAddons Qt5::Test kblog)

# ########### next target ###############
 
 set(testblogcomment_SRCS testblogcomment.cpp )
 
 
 kde4_add_unit_test(testblogcomment TESTNAME kblog-testblogcomment ${testblogcomment_SRCS})
 
 target_link_libraries(testblogcomment KF5::CoreAddons Qt5::Test kblog)



