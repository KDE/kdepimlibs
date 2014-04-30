set(microblog_SRCS
  statusitem.cpp
)

add_library(KF5MicroBlog ${microblog_SRCS})

generate_export_header(KF5MicroBlog BASE_NAME microblog)

add_library(KF5::MicroBlog ALIAS KF5MicroBlog)

target_include_directories(KF5MicroBlog INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/microblog>")
target_include_directories(KF5MicroBlog PUBLIC "$<BUILD_INTERFACE:${MicroBlog_SOURCE_DIR}/src;${MicroBlog_BINARY_DIR}/src>")

target_link_libraries(KF5MicroBlog
  PRIVATE
  KF5::PimUtils
)

set_target_properties(KF5MicroBlog PROPERTIES
    VERSION ${MICROBLOG_VERSION_STRING}
    SOVERSION ${MICROBLOG_SOVERSION}
    EXPORT_NAME MicroBlog
)

install(TARGETS KF5MicroBlog EXPORT KF5MicroBlogTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

########### Generate Headers ###############
ecm_generate_headers(MicroBlog_CamelCase_HEADERS
  HEADER_NAMES
  StatusItem

  REQUIRED_HEADERS MicroBlog_HEADERS
)

########### install files ###############

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/microblog_export.h
    ${MicroBlog_HEADERS}
    ${MicroBlog_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/MicroBlog
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME MicroBlog LIB_NAME KF5MicroBlog DEPS "" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
