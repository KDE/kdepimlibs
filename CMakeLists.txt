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

sb_add_project(kmime)
sb_add_project(kcontacts)
sb_add_project(kcalcore)
sb_add_project(syndication)
sb_add_project(kxmlrpcclient)
sb_add_project(kholidays)
###############################
sb_add_project(kimap)
sb_add_project(kpimtextedit)
sb_add_project(kpimutils)
sb_add_project(kidentitymanagement)
sb_add_project(mailtransport)
sb_add_project(kcalutils)
sb_add_project(akonadi)
sb_add_project(akonadi-mime)
sb_add_project(akonadi-calendar)
sb_add_project(akonadi-contact)
sb_add_project(akonadi-notes)
sb_add_project(akonadi-socialutils)
sb_add_project(akonadi-abc)
sb_add_project(kmbox)
sb_add_project(kblog)
sb_add_project(kalarmcal)
sb_add_project(gpgme++)
sb_add_project(qgpgme)
sb_add_project(kldap)
sb_add_project(kioslave)
sb_add_project(ktnef)
sb_add_project(kontactinterface)

sb_end()
else(KDEPIMLIBS_SUPERBUILD)
enable_testing()
add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)
# Uncomment when one of these subdir is building

# These subdirectories don't depend on anything from kdepimlibs.
# They shall be built first
add_subdirectory(kmime)
add_subdirectory(kcontacts)
add_subdirectory(kcalcore)
add_subdirectory(syndication)
add_subdirectory(kxmlrpcclient)
add_subdirectory(kholidays)
add_subdirectory(kimap)
add_subdirectory(kpimtextedit)
add_subdirectory(kpimutils)
add_subdirectory(akonadi)
add_subdirectory(akonadi-mime)
add_subdirectory(akonadi-calendar)
add_subdirectory(akonadi-contact)
add_subdirectory(akonadi-notes)
add_subdirectory(akonadi-socialutils)
add_subdirectory(akonadi-abc)
add_subdirectory(kidentitymanagement)
add_subdirectory(kcalutils)
add_subdirectory(kmbox)
add_subdirectory(kblog)
add_subdirectory(kalarmcal)
add_subdirectory(gpgme++)
add_subdirectory(qgpgme)
add_subdirectory(kioslave)
add_subdirectory(kldap)
add_subdirectory(ktnef)
add_subdirectory(mailtransport)
add_subdirectory(kontactinterface)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

endif()
