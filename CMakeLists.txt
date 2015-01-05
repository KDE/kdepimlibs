project(kdepimlibs)
cmake_minimum_required(VERSION 2.8.12)

option(KDEPIMLIBS_SUPERBUILD "Build standalone lib" FALSE)

if (KDEPIMLIBS_SUPERBUILD)

# Remove it when splitted
# This CMakeLists.txt is a "superbuild" for the frameworks.
# This means it can be used to build them all standalone one-by-one.
# This is useful to compile all them with one command.

# use mkdir build && cd build && cmake -DKDEPIMLIBS_SUPERBUILD=TRUE ../ && make -j8 sb_all

include(SuperBuild.cmake)

sb_add_project(akonadi)
sb_add_project(akonadi-mime)
sb_add_project(akonadi-contact)
sb_add_project(akonadi-notes)
sb_add_project(akonadi-socialutils)
sb_add_project(kioslave)
sb_end()
else(KDEPIMLIBS_SUPERBUILD)
enable_testing()
add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)
# Uncomment when one of these subdir is building

# These subdirectories don't depend on anything from kdepimlibs.
# They shall be built first
add_subdirectory(akonadi)
add_subdirectory(akonadi-mime)
add_subdirectory(akonadi-contact)
add_subdirectory(akonadi-notes)
add_subdirectory(akonadi-socialutils)
add_subdirectory(kioslave)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

endif()
