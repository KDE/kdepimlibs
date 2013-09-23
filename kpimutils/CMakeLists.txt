project(kpimutils)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5321)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(tests)

include_directories(
  ../kmime
  ${CMAKE_CURRENT_BINARY_DIR}/../kmime
)

set(kpimutils_LIB_SRCS
  email.cpp
  emailvalidator.cpp
  linklocator.cpp
  spellingfilter.cpp
  kfileio.cpp
  processes.cpp
  progressindicatorwidget.cpp
  progressindicatorlabel.cpp
)

# network access helper
if(WINCE)
  set(kpimutils_LIB_SRCS ${kpimutils_LIB_SRCS} networkaccesshelper_wince.cpp)
else()
  set(kpimutils_LIB_SRCS ${kpimutils_LIB_SRCS} networkaccesshelper_fake.cpp)
endif()

kde4_add_library(kpimutils ${LIBRARY_TYPE} ${kpimutils_LIB_SRCS})
generate_export_header(kpimutils)

target_link_libraries(kpimutils ${KDE4_KDEUI_LIBS} ${KDE4_KEMOTICONS_LIBS} kmime)

if(WINCE)
  target_link_libraries(kpimutils ${WCECOMPAT_LIBRARIES} ${KDE4_SOLID_LIBS} toolhelp)
endif()

set_target_properties(kpimutils PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kpimutils EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kpimutils_export.h
  email.h
  emailvalidator.h
  linklocator.h
  spellingfilter.h
  kfileio.h
  supertrait.h
  processes.h
  networkaccesshelper.h
  progressindicatorwidget.h
  progressindicatorlabel.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kpimutils COMPONENT Devel)
