project(kpimutils)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5321 )

add_subdirectory( tests )


include_directories( ../kmime )

set(kpimutils_LIB_SRCS
   email.cpp linklocator.cpp spellingfilter.cpp kfileio.cpp)

kde4_add_library(kpimutils SHARED ${kpimutils_LIB_SRCS})

target_link_libraries(kpimutils ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS} kmime)

set_target_properties(kpimutils PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kpimutils ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES kpimutils_export.h email.h linklocator.h spellingfilter.h kfileio.h 
        DESTINATION ${INCLUDE_INSTALL_DIR}/kpimutils  COMPONENT Devel)
