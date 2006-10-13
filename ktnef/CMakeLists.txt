project(ktnef)
########### next target ###############

include_directories(
   ${CMAKE_SOURCE_DIR}/kabc
   ${CMAKE_BINARY_DIR}/kabc
   ${CMAKE_SOURCE_DIR}/kcal
)

set(ktnef_LIB_SRCS 
   ktnefparser.cpp 
   ktnefproperty.cpp 
   ktnefattach.cpp 
   mapi.cpp 
   ktnefpropertyset.cpp 
   ktnefmessage.cpp 
   ktnefwriter.cpp 
   lzfu.cpp
   formatter.cpp
)

kde4_automoc(ktnef ${ktnef_LIB_SRCS})

kde4_add_library(ktnef SHARED ${ktnef_LIB_SRCS})

target_link_libraries(ktnef  ${KDE4_KIO_LIBS} kabc kcal)

set_target_properties(ktnef PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS ktnef DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES ktnef.h ktnefparser.h ktnefattach.h ktnefproperty.h
         ktnefpropertyset.h ktnefmessage.h ktnefwriter.h ktnefdefs.h formatter.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/ktnef)


