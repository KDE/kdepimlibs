# install the cmake files

file(GLOB cmakeFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")

set(module_install_dir ${DATA_INSTALL_DIR}/cmake/modules )

install( FILES  ${cmakeFiles} DESTINATION ${module_install_dir} )

# the files listed here will be removed by remove_obsoleted_cmake_files.cmake, Alex
set(FILES_TO_REMOVE
   FindKDE4.cmake
   )

KDE4_REMOVE_OBSOLETE_CMAKE_FILES( ${FILES_TO_REMOVE})
