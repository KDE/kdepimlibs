
include_directories( ${CMAKE_SOURCE_DIR}/ktnef  )

########### next target ###############

set(ktnef_LIB_SRCS 
   ktnefparser.cpp 
   ktnefproperty.cpp 
   ktnefattach.cpp 
   mapi.cpp 
   ktnefpropertyset.cpp 
   ktnefmessage.cpp 
   ktnefwriter.cpp 
   lzfu.cpp )

kde4_automoc(${ktnef_LIB_SRCS})

kde4_add_library(ktnef SHARED ${ktnef_LIB_SRCS})

target_link_libraries(ktnef  ${KDE4_KIO_LIBS} )

set_target_properties(ktnef PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} ktnef )


########### install files ###############




