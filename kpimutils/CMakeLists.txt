project(kpimutils)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=5321 )

add_subdirectory( tests )


include_directories( ../kmime )

set(kpimutils_LIB_SRCS
  email.cpp
  emailvalidator.cpp
  linklocator.cpp
  spellingfilter.cpp
  kfileio.cpp
  processes.cpp
)

# network access helper
if(WINCE)
  set(kpimutils_LIB_SRCS ${kpimutils_LIB_SRCS} networkaccesshelper_wince.cpp)
else(WINCE)
  set(kpimutils_LIB_SRCS ${kpimutils_LIB_SRCS} networkaccesshelper_fake.cpp)
endif(WINCE)

kde4_add_library(kpimutils ${LIBRARY_TYPE} ${kpimutils_LIB_SRCS})

target_link_libraries(kpimutils ${KDE4_KDEUI_LIBS} ${KDE4_KEMOTICONS_LIBS} kmime)

if(WINCE)
  target_link_libraries(kpimutils ${WCECOMPAT_LIBRARIES} ${KDE4_SOLID_LIBS} toolhelp)
endif(WINCE)

set_target_properties(kpimutils PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kpimutils EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  kpimutils_export.h
  email.h
  emailvalidator.h
  linklocator.h
  spellingfilter.h
  kfileio.h
  supertrait.h
  processes.h
  networkaccesshelper.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kpimutils  COMPONENT Devel)
