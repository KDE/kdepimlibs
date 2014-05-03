# install the cmake files

file(GLOB cmakeFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")

# Don't install FindQJSON and Xstlproc
list(REMOVE_ITEM cmakeFiles "${CMAKE_CURRENT_SOURCE_DIR}/FindQJSON.cmake")
list(REMOVE_ITEM cmakeFiles "${CMAKE_CURRENT_SOURCE_DIR}/FindXsltproc.cmake")

set(module_install_dir ${KF5_DATA_INSTALL_DIR}/cmake/modules)

install(FILES ${cmakeFiles} DESTINATION ${module_install_dir})

