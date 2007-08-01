include_directories( ${GPGME_INCLUDES} )


########### next target ###############
set(dataprovidertest_SRCS dataprovidertest.cpp )


kde4_add_test_executable(dataprovidertest ${dataprovidertest_SRCS})

target_link_libraries(dataprovidertest  qgpgme ${KDE4_KDECORE_LIBS} )

########### next target ############### 
set(installtest_SRCS installtest.cpp)
kde4_add_test_executable(installtest ${installtest_SRCS})  

target_link_libraries(installtest  qgpgme ${KDE4_KDECORE_LIBS} ) 
