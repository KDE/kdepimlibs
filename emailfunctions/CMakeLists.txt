project(emailfunctions)
add_subdirectory( tests )

########### next target ###############

set(emailfunctions_LIB_SRCS
   email.cpp linklocator.cpp)

kde4_automoc(${emailfunctions_LIB_SRCS})
kde4_add_kcfg_files(emailfunctions_LIB_SRCS pimemoticons.kcfgc)

kde4_add_library(emailfunctions SHARED ${emailfunctions_LIB_SRCS})

target_link_libraries(emailfunctions ${KDE4_KDECORE_LIBS})

set_target_properties(emailfunctions PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS emailfunctions  DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install( FILES emailfunctions.h email.h linklocator.h DESTINATION ${INCLUDE_INSTALL_DIR}/emailfunctions  )

install(FILES pimemoticons.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
