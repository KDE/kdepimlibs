project(kpimutils)
add_subdirectory( tests )

########### next target ###############

set(kpimutils_LIB_SRCS
   email.cpp linklocator.cpp)

kde4_automoc(${kpimutils_LIB_SRCS})
kde4_add_kcfg_files(kpimutils_LIB_SRCS pimemoticons.kcfgc)

kde4_add_library(kpimutils SHARED ${kpimutils_LIB_SRCS})

target_link_libraries(kpimutils ${KDE4_KDECORE_LIBS})

set_target_properties(kpimutils PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kpimutils  DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install( FILES kpimutils.h email.h linklocator.h DESTINATION ${INCLUDE_INSTALL_DIR}/kpimutils  )

install(FILES pimemoticons.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
