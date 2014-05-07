cmake_minimum_required(VERSION 2.8.12)
project(kdepimlibs)

# where to look first for cmake modules. This line must be the first one or cmake will use the system's FindFoo.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")


############### The kdepimlibs version (used e.g. in KdepimLibsConfig.cmake) ###############
set(KDEPIMLIBS_VERSION_MAJOR 4)
set(KDEPIMLIBS_VERSION_MINOR 13)
set(KDEPIMLIBS_VERSION_PATCH 46)
set(KDEPIMLIBS_VERSION ${KDEPIMLIBS_VERSION_MAJOR}.${KDEPIMLIBS_VERSION_MINOR}.${KDEPIMLIBS_VERSION_PATCH})


############### search packages used by KDE ###############

set (QT_MIN_VERSION "4.8.0")
# kdepim depends on the latest released version of kdelibs, ie. 4.x kdepim depends on 4.x-1 kdelibs
find_package(KDE4 4.12.0 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(KDEPlatformProfile)
include(FeatureSummary)

project(kdepimlibs)

option(KDEPIMLIBS_SUPERBUILD "Build standalone lib" FALSE)

if (KDEPIMLIBS_SUPERBUILD)

# Remove it when splitted

# This CMakeLists.txt is a "superbuild" for the frameworks.
# This means it can be used to build them all standalone one-by-one.
# This is useful to compile all them with one command.

# use mkdir build && cd build && cmake -DKDEPIMLIBS_SUPERBUILD=TRUE ../ && make -j8 sb_all

include(SuperBuild.cmake)

sb_add_project(kmime)
sb_add_project(kabc)
sb_add_project(kcalcore)
sb_add_project(syndication)
sb_add_project(kxmlrpcclient)
sb_add_project(kholidays)
###############################
sb_add_project(kimap)
sb_add_project(kpimtextedit)
sb_add_project(kpimutils)
sb_add_project(kpimidentities)
sb_add_project(mailtransport)
sb_add_project(kcalutils)
sb_add_project(akonadi)
sb_add_project(akonadi-kmime)
sb_add_project(akonadi-calendar)
sb_add_project(akonadi-contact)
sb_add_project(akonadi-notes)
sb_add_project(akonadi-socialutils)
sb_add_project(kmbox)
sb_add_project(kblog)
sb_add_project(microblog)
sb_add_project(kalarmcal)
sb_add_project(gpgme++)
sb_add_project(qgpgme)
sb_add_project(kldap)
sb_add_project(kioslave)
sb_add_project(ktnef)
sb_add_project(kontactinterface)

sb_end()
else(KDEPIMLIBS_SUPERBUILD)

# Uncomment when one of these subdir is building

# These subdirectories don't depend on anything from kdepimlibs.
# They shall be built first
add_subdirectory(kmime)
add_subdirectory(kabc)
add_subdirectory(kcalcore)
add_subdirectory(syndication)
add_subdirectory(kxmlrpcclient)
add_subdirectory(kholidays)
###############################
add_subdirectory(kimap)
add_subdirectory(kpimtextedit)
add_subdirectory(kpimutils)
add_subdirectory(akonadi)
add_subdirectory(akonadi-kmime)
add_subdirectory(akonadi-calendar)
add_subdirectory(akonadi-contact)
add_subdirectory(akonadi-notes)
add_subdirectory(akonadi-socialutils)
add_subdirectory(kpimidentities)
add_subdirectory(kcalutils)
add_subdirectory(kmbox)
add_subdirectory(kblog)
add_subdirectory(microblog)
add_subdirectory(kalarmcal)
add_subdirectory(gpgme++)
add_subdirectory(qgpgme)
add_subdirectory(kioslave)
add_subdirectory(kldap)
add_subdirectory(ktnef)
add_subdirectory(mailtransport)
add_subdirectory(kontactinterface)

feature_summary(WHAT ALL
                INCLUDE_QUIET_PACKAGES
                FATAL_ON_MISSING_REQUIRED_PACKAGES
)

endif()

# TODO Delete the following block when everything is done
if(FALSE)
############### Build Options ###############

option(KDEPIM_ONLY_KLEO "Only build the libraries needed by Kleopatra." FALSE)
cmake_dependent_option(KDEPIM_MOBILE_UI "Build UI for mobile devices instead of for desktops" OFF "_default_mobile_ui" OFF)
option(STATIC_LIBRARY "Build kdepimlibs as static libraries." FALSE)
#option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled." FALSE)
option(KDEPIMLIBS_RUN_ISOLATED_TESTS "Run the isolated tests." FALSE)


#FindGpgme.cmake already handles the log message but we must ensure it is required.
find_package(Gpgme REQUIRED)
# configure macros
if (GPGME_FOUND)
  include (gpgme++/ConfigureChecks.cmake)
endif()

if (NOT KDEPIM_ONLY_KLEO)
  #FindAkonadi.cmake is only there for compatibility reasons, but we don't want to use that.
  set(Akonadi_MIN_VERSION "1.12.44")
  find_package(Akonadi ${Akonadi_MIN_VERSION} QUIET NO_MODULE)
  set_package_properties(Akonadi PROPERTIES DESCRIPTION "Akonadi server libraries" URL "http://pim.kde.org/akonadi" TYPE REQUIRED PURPOSE "Access to PIM storage and services")

  include (ConfigureChecks.cmake)

  find_package(QJSON)
  set_package_properties(QJSON PROPERTIES DESCRIPTION "QJSON" URL "http://qjson.sourceforge.net/" TYPE REQUIRED PURPOSE "Qt library for handling JSON data")

  if (KDE4_BUILD_TESTS)
    find_package(Xsltproc)
    set_package_properties(Xsltproc PROPERTIES DESCRIPTION "XSLT processor from libxslt" TYPE REQUIRED PURPOSE "Required to generate D-Bus interfaces for all Akonadi resources.")

    find_package(LibXml2)
    set_package_properties(LibXml2 PROPERTIES DESCRIPTION "The LibXML2 library" URL "http://xmlsoft.org" TYPE OPTIONAL PURPOSE "Needed for building the knut Akonadi resource.")
  endif()

endif()

#FindGpgme.cmake already handles the log message but we must ensure it is required.
find_package(Gpgme)
set_package_properties(Gpgme PROPERTIES
                       URL "http://www.gnupg.org/related_software/gpgme/"
                       DESCRIPTION "The GnuPG Made Easy (GPGME) library)"
                       TYPE REQUIRED
)

# configure macros
include (gpgme++/ConfigureChecks.cmake)

include (ConfigureChecks.cmake)


############### Here we install some extra stuff  ###############

if (NOT KDEPIM_ONLY_KLEO)
  install(FILES kdepimlibs-mime.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
  update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
endif()

endif()
