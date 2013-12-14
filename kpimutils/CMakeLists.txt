project(kpimutils)

find_package(KF5 NO_MODULE REQUIRED COMPONENTS ItemViews Emoticons)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5321)

add_subdirectory(tests)

include_directories(../kmime)

set(kpimutils_LIB_SRCS
  email.cpp
  emailvalidator.cpp
  linklocator.cpp
  spellingfilter.cpp
  kfileio.cpp
  processes.cpp
  progressindicatorwidget.cpp
)

# network access helper
if(WINCE)
  set(kpimutils_LIB_SRCS ${kpimutils_LIB_SRCS} networkaccesshelper_wince.cpp)
else()
  set(kpimutils_LIB_SRCS ${kpimutils_LIB_SRCS} networkaccesshelper_fake.cpp)
endif()

kde4_add_library(kpimutils ${LIBRARY_TYPE} ${kpimutils_LIB_SRCS})

target_link_libraries(kpimutils KF5::ItemViews KF5::Emoticons kmime)

if(WINCE)
  target_link_libraries(kpimutils ${WCECOMPAT_LIBRARIES} KF5::Solid toolhelp)
endif()

set_target_properties(kpimutils PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

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
  progressindicatorwidget.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kpimutils COMPONENT Devel)
